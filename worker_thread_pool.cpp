#include "worker_thread_pool.h"
#include "worker.h"
#include <string>

namespace threadpool
{
	worker_thread_pool::worker_thread_pool(const std::string & name, const uint8_t worker_thread_count) :
		my_name(name)
	{
		for (uint8_t i = 0; i < worker_thread_count; ++i)
			my_worker_threads.emplace_back(new worker_thread(my_name + ' ' + std::to_string(i)));
		my_add_thread_index = my_worker_threads.begin();
	}

	void worker_thread_pool::add_worker(shared_worker &worker)
	{

		worker_thread *worker_thread = nullptr;
		{
			std::lock_guard<std::mutex> add_lock(my_add_mutex);
			worker_thread = (*my_add_thread_index++).get();
			if (my_add_thread_index == my_worker_threads.end())
				my_add_thread_index = my_worker_threads.begin();
		}
		worker->set_worker_thread(worker_thread);
		worker_thread->add_worker(worker);

	}
}