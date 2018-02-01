#pragma once
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <vector>
#include <string>
#include <list>
#include <thread>
#include "worker_types.h"

namespace small_tl::threading
{
	class worker_thread
	{
		worker_thread(const worker_thread &) = delete;
		worker_thread(worker_thread &&) = delete;
		worker_thread& operator=(const worker_thread &) = delete;
		worker_thread& operator=(worker_thread &&) = delete;

		friend class worker_thread_pool;
		friend class worker;

	public:
		worker_thread(const std::string &name);
		~worker_thread();

		bool is_current_thread() const;
		void assert_on_thread() const;
		std::thread::id thread_id() const;

	private:
		//wake up the thread to let it perform scheduled work
		void schedule_work();

		//add a worker to the pool at the next synchronisation point
		void add_worker(weak_worker worker);

		size_t worker_count() const;

		//pulls a list of workers into the thread local pool
		void update_thread_local_workers(weak_workers &workers);

		//Executes scheduled work on a thread local pool of workers
		void run();

		void await_work();

		mutable std::mutex my_worker_changes_mutex;
		size_t my_worker_count;
		weak_workers my_workers_to_add;
		std::atomic_bool *stop;
		std::thread my_worker_thread;
		const std::string my_name;

		std::mutex my_have_work_mutex;
		bool my_have_work;
		std::condition_variable my_have_work_event;

	};
}