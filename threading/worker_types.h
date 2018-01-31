#pragma once
#include <memory>
#include <set>
#include <map>
#include <stack>
#include <cstdint>

namespace threadpool
{
	class worker;
	class worker_thread;

	typedef std::weak_ptr<worker> weak_worker;
	typedef std::shared_ptr<worker> shared_worker;
	typedef std::vector<weak_worker> weak_workers;
}
