#pragma once
#include "worker_types.h"
#include "worker.h"
#include "worker_thread.h"
#include "thread_local_member.h"
#include <set>
#include <deque>
#include <cassert>
#include "message.h"

namespace threadpool
{
	template<class listener>
	class messenger : public worker
	{
	public:
		typedef message<listener> message_t;
		typedef std::unique_ptr<const message_t> message_ptr_t;

		messenger(const std::string &name) : worker(name), my_remove_all_flag(false) {}

		virtual ~messenger()
		{
			//the run loop will never run again, we need to unblock all waiting threads
			my_remove_all_flag = false;
			my_change_list.clear();
			my_change_event.notify_all();
		}

		//
		//Add/Remove Listeners
		//
		void add_listener(listener *listener)
		{
			std::lock_guard<std::mutex> change_lock(my_change_mutex);
			my_change_list[listener] = add_change;
		}

		void remove_all_listeners()
		{
			std::unique_lock<std::mutex> change_lock(my_change_mutex);
			my_remove_all_flag = true;

			bool should_continue = get_worker_thread().is_current_thread();
			while (!should_continue)
			{
				schedule_work();
				my_change_event.wait(change_lock);
				should_continue = !my_remove_all_flag;
			}
		}

		//if you remove a registered listener in a callback, we will not touch it again, 
		//if you remove and destroy it on another thread without a wait it could cause a bad access
		void remove_listener(listener *listener, bool wait = true)
		{
			std::unique_lock<std::mutex> change_lock(my_change_mutex);
			my_change_list[listener] = remove_change;

			if (wait)
			{
				wait_on_remove(listener, std::move(change_lock));
			}
		}

		void wait_on_remove(listener *listener, std::unique_lock<std::mutex> change_lock = std::unique_lock<std::mutex>(my_change_mutex))
		{
			//listener is not in the remove list
			if (my_change_list.find(listener) == my_change_list.cend())return;

			bool should_continue = get_worker_thread().is_current_thread();
			while (!should_continue)
			{
				schedule_work();
				my_change_event.wait(change_lock);
				//listener is no longer in the remove list
				typename std::map<listener*, change_t>::const_iterator change_iterator = my_change_list.find(listener);
				should_continue = change_iterator == my_change_list.cend() || change_iterator->second != remove_change;
			}
		}

		//Send a new Message to all listeners
		//ownership of EV_Message is passed to EV_Messenger, EV_Messenger will delete it
		void message_listeners(message_ptr_t message)
		{
			std::lock_guard<std::mutex> message_queue_lock(my_message_queue_mutex);
			my_pending_messages.push_back(std::move(message));
			schedule_work();
		}

	private:

		enum change_t
		{
			add_change,
			remove_change
		};

		thread_local_member<std::set<listener *>> thread_local_listeners;

		mutable std::mutex my_change_mutex;
		bool my_remove_all_flag;
		std::map<listener *, change_t> my_change_list;
		mutable std::condition_variable my_change_event;

		std::mutex my_message_queue_mutex;
		std::deque<message_ptr_t> my_pending_messages;

		virtual void setup()
		{
			thread_local_listeners.set_thread_id(get_worker_thread().thread_id());
		}

		// Inherited via Worker
		virtual void run() override
		{
			assert(thread_local_listeners.get());
			std::set<listener *> &message_thread_listeners = *thread_local_listeners.get();

			//thread local copy of messages
			std::deque<message_ptr_t> thread_local_pending_messages;
			{
				std::lock_guard<std::mutex> message_queue_lock(my_message_queue_mutex);
				thread_local_pending_messages = std::move(my_pending_messages);
				my_pending_messages.clear();
			}

			update_listeners(message_thread_listeners);
			while (!thread_local_pending_messages.empty())
			{
				message_ptr_t message = std::move(message_thread_pending_messages.front());
				message_thread_pending_messages.pop_front();
				std::set<listener *> listeners_messaged;
				while (!all_listeners_have_been_messaged(message_thread_listeners, listeners_messaged))
				{
					//traverse the set to find a listener not yet messaged
					for (listener *listener : message_thread_listeners)
					{
						if (listeners_messaged.find(listener) == listeners_messaged.end())
						{
							//signal it
							message->call(listener);
							listeners_messaged.insert(listener);
							//we want callbacks to be able to remove listeners without fear of them being called, so every call we process the removals and discard our iterator
							update_listeners(message_thread_listeners);
							//discard the iterator, it may have become invalid
							break;
						}
					}
				}
			}
		}

		void update_listeners(std::set<listener *> &listeners)
		{
			std::lock_guard<std::mutex> change_lock(my_change_mutex);
			if (my_remove_all_flag)
				listeners.clear();
			else
			{
				for (std::pair<listener*, change_t> pair : my_change_list)
				{
					if (pair.second == remove_change)
						listeners.erase(pair.first);
					else
						listeners.insert(pair.first);
				}
			}

			my_remove_all_flag = false;
			my_change_list.clear();
			my_change_event.notify_all();
		}

		static bool all_listeners_have_been_messaged(const std::set<listener*> &all_listeners, const std::set<listener*>messaged_listeners)
		{
			//99% of the time this check is enough
			if (messaged_listeners.size() < all_listeners.size())
				return false;
			for (typename std::set<listener *>::iterator all_it = all_listeners.begin(); all_it != all_listeners.end(); ++all_it)
				if (messaged_listeners.find(*all_it) == messaged_listeners.end())
					return false;
			return true;
		}
	};
}
