#pragma once

namespace small_tl::threading
{
	template<class thread_local_member_t>
	class thread_local_member
	{
	public:
		thread_local_member() = default;
		thread_local_member(std::thread::id thread_id) :my_thread_id(thread_id) {}

		void set_thread_id(std::thread::id thread_id) { my_thread_id = thread_id; }

		thread_local_member_t *get()
		{
			if (std::this_thread::get_id() == my_thread_id)
				return &member;
			else return nullptr;
		}
	private:
		std::thread::id my_thread_id;
		thread_local_member_t member;
	};
}