#include "worker_thread.h"
#include "worker.h"
#include <cassert>
#include <memory>
#include <algorithm>
#include <thread>

namespace small_tl::threading
{
	void set_thread_name(const std::thread::native_handle_type thread_id, char const * const name)
	{

	}

	worker_thread::worker_thread(const std::string &name) :my_name(name), my_worker_count(0), my_have_work(false)
	{
		stop = new std::atomic_bool();
		stop->store(false);
		my_worker_thread = std::thread(&worker_thread::run, this);

		schedule_work();
	}

	worker_thread::~worker_thread()
	{
		//tell the thread to stop, wake it up and then wait on it finishing
		//if we're currently on the thread we won't wait, we'll let it run, when we return to it's main loop it will immediately end without touching members
		stop->store(true);
		schedule_work();

		if (is_current_thread())
			my_worker_thread.detach();
		else if (my_worker_thread.joinable())
			my_worker_thread.join();
	}

	void worker_thread::schedule_work()
	{
		std::unique_lock<std::mutex> have_work_lock(my_have_work_mutex);
		my_have_work = true;
		my_have_work_event.notify_one();
	}

	bool worker_thread::is_current_thread() const
	{
		return std::this_thread::get_id() == my_worker_thread.get_id();
	}

	void worker_thread::assert_on_thread() const
	{
		assert(is_current_thread());
	}

	std::thread::id worker_thread::thread_id() const
	{
		return my_worker_thread.get_id();
	}

	void worker_thread::add_worker(weak_worker worker)
	{
		std::lock_guard<std::mutex> worker_changes_lock(my_worker_changes_mutex);
		my_workers_to_add.push_back(worker);
	}

	size_t worker_thread::worker_count() const
	{
		std::lock_guard<std::mutex> worker_changes_lock(my_worker_changes_mutex);
		return my_worker_count;
	}

	void worker_thread::update_thread_local_workers(weak_workers &workers)
	{
		assert_on_thread();
		std::lock_guard<std::mutex> worker_changes_lock(my_worker_changes_mutex);
		my_worker_count = workers.size();
		//workers erase all expired workers
		uint16_t dead_workers = 0;
		for (weak_worker &weak_worker : my_workers_to_add)
		{
			std::shared_ptr<worker> worker = weak_worker.lock();
			if (worker)
			{
				worker->setup();
				workers.push_back(weak_worker);
			}
			else
			{
				++dead_workers;
			}
		}

		my_workers_to_add.clear();
		//pulled these numbers out the air but we only need to do the expensive vector erase if there's a relatively large amount of dead workers
		if (dead_workers > 20 || dead_workers > my_worker_count / 3)
			workers.erase(std::remove_if(workers.begin(), workers.end(), [](weak_worker &worker) -> bool {return worker.expired(); }), workers.end());
	}

	void worker_thread::run()
	{
		//thead scope owns the stop flag, other threads can flip it through the non owning raw member reference
		std::unique_ptr<std::atomic_bool> local_stop = std::unique_ptr<std::atomic_bool>(stop);

		//wait until the worker_thread has been assigned
		await_work();
		assert_on_thread();
		set_thread_name(my_worker_thread.native_handle(), my_name.c_str());
		weak_workers workers;
		while (!local_stop->load())
		{
			await_work();
			//if we've been woken up by the destructor don't bother doing any work
			if (local_stop->load())
				return;

			update_thread_local_workers(workers);
			for (weak_worker &weak_worker : workers)
			{
				std::shared_ptr<worker> shared_worker = weak_worker.lock();
				if (shared_worker && shared_worker->has_work())
				{
					shared_worker->run();
					shared_worker->work_completed();
					//if the thread was destroyed by the worker abort now
					if (local_stop->load())
						return;
				}
			}
		}
		return;
	}
	void worker_thread::await_work()
	{
		std::unique_lock<std::mutex> have_work_lock(my_have_work_mutex);
		my_have_work_event.wait(have_work_lock, [this]()->bool {return my_have_work; });
		my_have_work = false;
	}
}