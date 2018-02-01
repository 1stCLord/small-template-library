#pragma once
#include <memory>
#include <mutex>

namespace small_tl::threading
{
	class worker
	{
		friend class worker_thread_pool;
		friend class worker_thread;
		worker(const worker &other) = delete;
		worker(worker &&other) = delete;
		worker &operator=(const worker &other) = delete;
		worker &operator=(worker &&other) = delete;

	protected:
		worker(const std::string &name);
		void schedule_work();
		const worker_thread &get_worker_thread() const;
		virtual void setup() {};
	private:
		void set_worker_thread(worker_thread *in_worker_thread);

		virtual void run() = 0;

		bool has_work();
		void work_completed();

		std::mutex my_data_mutex;
		uint8_t my_has_work;
		worker_thread *my_worker_thread;

		const std::string my_name;
	};
}