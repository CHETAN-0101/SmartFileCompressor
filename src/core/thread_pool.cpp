#include "sfc/thread_pool.h"
#include "sfc/scheduler.h"

namespace sfc {

ThreadPool::ThreadPool(size_t num_threads, PriorityLevel priority)
    : priority_(priority)
{
    num_threads = num_threads > 0 ? num_threads : 1;
    for (size_t i = 0; i < num_threads; ++i) {
        workers_.emplace_back(&ThreadPool::worker_loop, this, i);
    }
}

ThreadPool::~ThreadPool() {
    stop_ = true;
    cv_task_.notify_all();
    cv_pause_.notify_all();
    for (std::thread& worker : workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}

void ThreadPool::set_priority(PriorityLevel priority) {
    priority_ = priority;
}

void ThreadPool::pause() {
    paused_ = true;
}

void ThreadPool::resume() {
    paused_ = false;
    cv_pause_.notify_all();
}

size_t ThreadPool::queue_size() {
    std::unique_lock<std::mutex> lock(queue_mutex_);
    return tasks_.size();
}

void ThreadPool::worker_loop(size_t thread_id) {
    // Apply OS thread priority
    os::set_current_thread_priority(priority_);

    while (true) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            cv_task_.wait(lock, [this]() {
                return stop_ || (!tasks_.empty() && !paused_);
            });

            if (stop_ && tasks_.empty()) {
                return;
            }

            if (paused_) {
                cv_pause_.wait(lock, [this]() { return stop_ || !paused_; });
                if (stop_ && tasks_.empty()) return;
            }

            if (tasks_.empty()) continue;

            task = std::move(tasks_.front());
            tasks_.pop();
        }

        active_workers_++;
        task();
        active_workers_--;
    }
}

} // namespace sfc
