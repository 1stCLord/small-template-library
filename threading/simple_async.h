#pragma once

#include <atomic>
#include <mutex>
#include <vector>
#include <queue>
#include <set>
#include <condition_variable>
#include <atomic>
#include <thread>
#include <mutex>

namespace threadpool
{
	class simple_async
	{
	private:
		simple_async(const simple_async &) = delete;
		simple_async(simple_async &&) = delete;
		simple_async& operator=(const simple_async &) = delete;
		simple_async& operator=(simple_async &&) = delete;

	public:
		typedef std::chrono::milliseconds duration;
		typedef void (void_function)(void);
		typedef std::function<void_function> task;
		typedef std::chrono::system_clock clock;

		simple_async();
		~simple_async();

		void run();
		void schedule(const task &async_task, duration wait = duration(0));
		void cancel(const task &async_task);

	private:
		struct scheduled_task;

		std::mutex my_async_task_mutex;
		std::set<scheduled_task> my_scheduled_async_tasks;
		std::thread my_task_thread;
		std::condition_variable my_wake;
		std::atomic_bool my_stop;

	};
}
