#include "threadPool.hpp"

void ThreadPool::joinAll()
{
    for (auto &thread : m_thread_pool)
    {
        thread.join();
    }
}