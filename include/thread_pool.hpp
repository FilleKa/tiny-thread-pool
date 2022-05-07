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
    explicit ThreadPool(size_t thread_count);
    ~ThreadPool();

    void Enqueue(std::function<void(void)> work, std::function<void(void)> callback);

    template <typename F, typename R = std::result_of_t<F()>>
    std::future<R> Enqueue(F work);

    void ExecuteCallbacks();
    void Wait();

  private:
    struct TaskBase {
        virtual void Execute() = 0;
    };

    struct Task : public TaskBase {
        Task(std::function<void(void)> work);
        void Execute() override;

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

template <typename F, typename R>
std::future<R> ThreadPool::Enqueue(F work) {
    auto packaged_task = std::make_shared<PackagedTask<R>>(work);
    auto future = packaged_task->work_.get_future();

    auto work_package = std::make_unique<Work>(Work{std::move(packaged_task), nullptr});

    work_queue_.Push(std::move(work_package));
    cond_var_.notify_one();
    return future;
}

}  // namespace ttp

#endif /* TTP_THREAD_POOL_HPP_ */
