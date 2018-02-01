#include "worker.h"
#include "worker_thread.h"
#include <cassert>

namespace small_tl::threading
{
	worker::worker(const std::string & name) : my_name(name), my_worker_thread(nullptr), my_has_work(0) {}

	void worker::schedule_work()
	{
		std::lock_guard<std::mutex> data_lock(my_data_mutex);
		my_has_work++;
		my_worker_thread->schedule_work();
	}

	const worker_thread &worker::get_worker_thread() const
	{
		return *my_worker_thread;
	}

	void worker::set_worker_thread(worker_thread *in_worker_thread)
	{
		my_worker_thread = in_worker_thread;
	}

	bool worker::has_work()
	{
		std::lock_guard<std::mutex> data_lock(my_data_mutex);
		return my_has_work != 0;
	}

	void worker::work_completed()
	{
		std::lock_guard<std::mutex> data_lock(my_data_mutex);
		assert(my_has_work > 0);
		my_has_work--;
	}
}
