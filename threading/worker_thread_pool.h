#pragma once
#include <atomic>
#include <list>
#include <thread>
#include <algorithm>
#include "worker_thread.h"
#include "worker_types.h"

//oversubscribe potentially quiet threads
uint8_t default_thread_pool_size = std::min<uint8_t>(8, std::thread::hardware_concurrency() * 2);

namespace small_tl::threading
{
	class worker;
	class worker_thread_pool
	{
	public:
		worker_thread_pool(const std::string &name, uint8_t thread_count = default_thread_pool_size);
		template<class worker_type>
		std::shared_ptr<worker_type> add_worker(const std::string &name)
		{
			std::shared_ptr<worker_type> worker(new worker_type(name));

			add_worker(std::static_pointer_cast<worker, worker_type>(worker));
			return worker;
		}
	private:
		typedef std::vector<std::unique_ptr<worker_thread>> worker_threads;

		void add_worker(shared_worker &worker);

		const std::string my_name;
		std::mutex my_add_mutex;
		worker_threads::iterator my_add_thread_index;
		worker_threads my_worker_threads;
	};
}