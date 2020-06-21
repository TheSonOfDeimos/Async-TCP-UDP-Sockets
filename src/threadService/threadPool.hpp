#ifndef THREAD_POOL
#define THREAD_POOL

#include <thread>
#include <functional>
#include <vector>

class ThreadPool
{
public:

	ThreadPool() = default;
	~ThreadPool() = default;

	template <class Function, class ... Args>
	void newThread(Function&& t_func, Args&& ... t_args);

	void joinAll();

private:
	std::vector<std::thread> m_thread_pool;

};

template <class Function, class... Args>
void ThreadPool::newThread(Function &&t_func, Args &&... t_args)
{
    m_thread_pool.emplace_back(t_func, t_args...);
}

#endif