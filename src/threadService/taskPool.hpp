#ifndef TASK_POOL
#define TASK_POOL

#include <mutex>
#include <vector>
#include <thread>
#include <condition_variable>
#include <future>

class TaskPool
{
public:
    typedef std::function<void()> complete_function_t;

    TaskPool();
    ~TaskPool();
    TaskPool(const TaskPool &t_other) = delete;
    TaskPool(TaskPool &&t_other) = delete;
    TaskPool operator=(const TaskPool &t_other) = delete;

    template <class Function, class... Args>
    void push(Function &&t_func, Args &&... t_args);

    complete_function_t pop();
    std::size_t size();
    void run();
    void stop();

private:
    std::atomic<bool> m_is_running{true};
    std::mutex m_mtx;
    std::condition_variable m_new_task_notify;
    std::vector<complete_function_t> m_task_pool;

    std::vector<std::shared_ptr<std::promise<void>>> m_promice_vec;
    std::vector<std::shared_ptr<std::future<void>>> m_future_vec;

    const std::thread::id m_master_thread_id;
};

template <class Function, class... Args>
void TaskPool::push(Function &&t_func, Args &&... t_args)
{
    std::lock_guard<std::mutex> lock(m_mtx);
    m_task_pool.emplace_back(t_func, t_args...);
    m_new_task_notify.notify_one();
}

#endif