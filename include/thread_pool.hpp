#ifndef TTP_THREAD_POOL_HPP_
#define TTP_THREAD_POOL_HPP_

#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <thread>

#include "blocking_queue.hpp"

namespace ttp {

class ThreadPool {
  public:
    explicit ThreadPool(size_t thread_count) : stopping_(false), current_work_count_(0) {

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

    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(mtx_);
            stopping_ = true;
        }

        cond_var_.notify_all();

        for (auto& t : threads_) {
            t.join();
        }

        ExecuteCallbacks();
    }

    /**
     * @brief Enqueue work accompanied by a callback function.
     *
     */
    void Enqueue(std::function<void(void)> work, std::function<void(void)> callback) {
        work_queue_.Push(std::make_unique<Work>(Work{std::make_shared<Task>(work), std::make_shared<Task>(callback)}));
        cond_var_.notify_one();
    }

    /**
     * @brief Enqueue work that could return a value (future).
     *
     */
    template <typename F, typename R = std::result_of_t<F()>>
    std::future<R> Enqueue(F work) {
        auto packaged_task = std::make_shared<PackagedTask<R>>(work);
        auto future = packaged_task->work_.get_future();

        auto work_package = std::make_unique<Work>(Work{std::move(packaged_task), nullptr});

        work_queue_.Push(std::move(work_package));
        cond_var_.notify_one();
        return future;
    }

    /**
     * @brief Executes any callbacks on current thread.
     *
     */
    void ExecuteCallbacks() {
        while (!callbacks_.Empty()) {
            callbacks_.Peek()->Execute();
            callbacks_.Pop();
        }
    }

    /**
     * @brief Blocks untill the work queue is empty.
     *
     */
    void Wait() {
        std::unique_lock<std::mutex> lck(mtx_);
        current_work_count_var_.wait(lck, [=]() { return work_queue_.Empty() && current_work_count_ == 0; });
    }

  private:
    struct TaskBase {
        virtual void Execute() = 0;
    };

    struct Task : public TaskBase {
        Task(std::function<void(void)> work) : work_(work) {}
        void Execute() override {
            if (work_) {
                work_();
            }
        }

        std::function<void(void)> work_;
    };

    template <typename T>
    struct PackagedTask : public TaskBase {
        PackagedTask(std::function<T(void)> work) : work_(std::move(work)) {}
        void Execute() override { work_(); }

        std::packaged_task<T(void)> work_;
    };

    struct Work {
        std::shared_ptr<TaskBase> workload;
        std::shared_ptr<Task> callback;
    };

    BlockingQueue<std::unique_ptr<Work>> work_queue_;
    BlockingQueue<std::shared_ptr<Task>> callbacks_;

    std::vector<std::thread> threads_;

    bool stopping_;

    size_t current_work_count_;

    std::mutex mtx_;

    std::condition_variable cond_var_;
    std::condition_variable current_work_count_var_;
};

}  // namespace ttp

#endif /* TTP_THREAD_POOL_HPP_ */
