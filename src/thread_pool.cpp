#include "thread_pool.hpp"

#include <algorithm>

namespace ttp {

ThreadPool::ThreadPool(size_t thread_count) : stopping_(false), current_work_count_(0) {
    auto hardware_threads = std::thread::hardware_concurrency();
    thread_count = std::min(thread_count, static_cast<size_t>(hardware_threads));

    for (int i = 0; i < thread_count; i++) {
        threads_.emplace_back([this]() {
            while (true) {
                std::shared_ptr<TaskBase> workload;
                std::shared_ptr<Task> callback;

                {
                    std::unique_lock<std::mutex> lock(mtx_);
                    cond_var_.wait(lock, [=]() { return stopping_ || !work_queue_.Empty(); });

                    if (stopping_) {
                        break;
                    }

                    current_work_count_++;
                    const auto& work = work_queue_.Peek();

                    workload = work->workload;
                    callback = work->callback;
                    work_queue_.Pop();
                }

                workload->Execute();

                if (callback) {
                    callbacks_.Push(callback);
                }

                {
                    std::unique_lock<std::mutex> lock(mtx_);
                    current_work_count_--;
                    current_work_count_var_.notify_one();
                }
            }
        });
    }
}

ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lck(mtx_);
        stopping_ = true;
    }

    cond_var_.notify_all();

    for (auto& t : threads_) {
        t.join();
    }

    ExecuteCallbacks();
}

void ThreadPool::ExecuteCallbacks() {
    while (!callbacks_.Empty()) {
        callbacks_.Peek()->Execute();
        callbacks_.Pop();
    }
}

void ThreadPool::Wait() {
    std::unique_lock<std::mutex> lck(mtx_);
    current_work_count_var_.wait(lck, [=]() { return work_queue_.Empty() && current_work_count_ == 0; });
}

void ThreadPool::Enqueue(std::function<void(void)> work, std::function<void(void)> callback) {
    work_queue_.Push(std::make_unique<Work>(Work{std::make_shared<Task>(work), std::make_shared<Task>(callback)}));
    cond_var_.notify_one();
}

ThreadPool::Task::Task(std::function<void(void)> work) : work_(work) {}

void ThreadPool::Task::Execute() {
    if (work_) {
        work_();
    }
}

}  // namespace ttp