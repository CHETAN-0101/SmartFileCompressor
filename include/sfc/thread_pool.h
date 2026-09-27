#pragma once

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <atomic>

namespace sfc {

enum class PriorityLevel {
    Normal,
    Low,
    Idle
};

class ThreadPool {
public:
    explicit ThreadPool(size_t num_threads = std::thread::hardware_concurrency(), PriorityLevel priority = PriorityLevel::Low);
    ~ThreadPool();

    // Enqueue a task returning a std::future
    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args) 
        -> std::future<typename std::invoke_result<F, Args...>::type>;

    void set_priority(PriorityLevel priority);
    void pause();
    void resume();
    bool is_paused() const { return paused_; }
    size_t active_workers() const { return active_workers_; }
    size_t queue_size();

private:
    void worker_loop(size_t thread_id);

    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    std::mutex queue_mutex_;
    std::condition_variable cv_task_;
    std::condition_variable cv_pause_;
    std::atomic<bool> stop_{false};
    std::atomic<bool> paused_{false};
    std::atomic<size_t> active_workers_{0};
    PriorityLevel priority_;
};

template<class F, class... Args>
auto ThreadPool::enqueue(F&& f, Args&&... args) 
    -> std::future<typename std::invoke_result<F, Args...>::type> 
{
    using return_type = typename std::invoke_result<F, Args...>::type;

    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );
    
    std::future<return_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        if (stop_) {
            throw std::runtime_error("ThreadPool: enqueue called on stopped ThreadPool");
        }
        tasks_.emplace([task]() { (*task)(); });
    }
    cv_task_.notify_one();
    return res;
}

} // namespace sfc
