#include "taskPool.hpp"

TaskPool::TaskPool()
    : m_master_thread_id(std::this_thread::get_id())
{
}

TaskPool::~TaskPool()
{
    this->stop();
    for (auto future : m_future_vec)
    {
        future->wait();
    }
}

TaskPool::complete_function_t TaskPool::pop()
{
    std::lock_guard<std::mutex> lock(m_mtx);
    complete_function_t task = m_task_pool.front();
    m_task_pool.erase(m_task_pool.begin());
    return task;
}

std::size_t TaskPool::size()
{
    std::lock_guard<std::mutex> lock(m_mtx);
    return m_task_pool.size();
}

void TaskPool::run()
{
    if (std::this_thread::get_id() == m_master_thread_id)
    {
        throw std::logic_error("Task pool can't be runnable in master thread!");
    }

    m_mtx.lock();
    m_promice_vec.emplace_back(new std::promise<void>());
    m_future_vec.emplace_back(new std::future<void>(m_promice_vec.back()->get_future()));
    m_promice_vec.back()->set_value_at_thread_exit();
    m_mtx.unlock();

    while (m_is_running)
    {
        if (this->size() == 0)
        {
            std::unique_lock<std::mutex> lock(m_mtx);
            m_new_task_notify.wait(lock);
        }

        if (this->size() != 0)
        {
            this->pop().operator()();
        }
    }
}

void TaskPool::stop()
{
    std::lock_guard<std::mutex> lock(m_mtx);
    m_is_running = false;
    m_new_task_notify.notify_all();
}