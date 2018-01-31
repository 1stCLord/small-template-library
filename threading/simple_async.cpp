#include "simple_async.h"
#include <cassert>
#include <algorithm>

namespace threadpool
{

  struct simple_async::scheduled_task
  {
    task my_task;
    clock::time_point my_time_point;

    scheduled_task(task task, clock::time_point time_point = clock::time_point()) { my_task = task; my_time_point = time_point; }

    bool operator==(scheduled_task &other)
    {
      assert(my_task.target<void_function>() != nullptr && other.my_task.target<void_function>() != nullptr);
      return my_task.target<void_function>() == other.my_task.target<void_function>();
    }

    bool operator!=(scheduled_task &other)
    {
      assert(my_task.target<void_function>() != nullptr && other.my_task.target<void_function>() != nullptr);
      return my_task.target<void_function>() != other.my_task.target<void_function>();
    }

    bool operator< (const scheduled_task &other) const { return my_time_point <  other.my_time_point; }
    bool operator> (const scheduled_task &other) const { return my_time_point >  other.my_time_point; }
    bool operator<=(const scheduled_task &other) const { return my_time_point <= other.my_time_point; }
    bool operator>=(const scheduled_task &other) const { return my_time_point >= other.my_time_point; }
  };

  simple_async::simple_async()
  {
	  my_stop.store(false);
	  my_task_thread = std::thread(&simple_async::run, this);
  }

  simple_async::~simple_async()
  {
	  my_stop.store(true);
	  my_wake.notify_one();
    if (my_task_thread.joinable())
		my_task_thread.join();
  }

  void simple_async::run()
  {
    while (!my_stop.load())
    {
      std::unique_lock<std::mutex> lock(my_async_task_mutex);
      if (!my_scheduled_async_tasks.empty() && my_scheduled_async_tasks.cbegin()->my_time_point <= clock::now())
      {
        const scheduled_task task = *my_scheduled_async_tasks.cbegin();
		my_scheduled_async_tasks.erase(task);
        lock.unlock();
        task.my_task();
      }
      else if (!my_scheduled_async_tasks.empty())
      {
		  my_wake.wait_until(lock, my_scheduled_async_tasks.cbegin()->my_time_point);
      }
      else
      {
		  my_wake.wait(lock);
      }
    }
  }

  void simple_async::schedule(const task & async_task, duration wait)
  {
    clock::time_point my_time_point = clock::now() + wait;
    std::lock_guard<std::mutex> lock(my_async_task_mutex);
    scheduled_task scheduled_task(async_task, my_time_point);
	my_scheduled_async_tasks.erase(scheduled_task);
	my_scheduled_async_tasks.insert(scheduled_task);
	my_wake.notify_one();
  }

  void simple_async::cancel(const task & async_task)
  {
    std::lock_guard<std::mutex> lock(my_async_task_mutex);
	my_scheduled_async_tasks.erase(scheduled_task( async_task ));
  }
}