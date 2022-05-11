#ifndef TTP_BLOCKING_QUEUE_HPP_
#define TTP_BLOCKING_QUEUE_HPP_

#include <mutex>
#include <queue>

namespace ttp {

template <typename T>
class BlockingQueue {
  public:
    void Push(T&& obj) {
        std::lock_guard<std::mutex> lock(mtx_);
        queue_.push(std::forward<T>(obj));
    }

    void Push(const T& obj) {
        std::lock_guard<std::mutex> lock(mtx_);
        queue_.push(obj);
    }

    bool Empty() {
        std::lock_guard<std::mutex> lock(mtx_);
        return queue_.empty();
    }

    T& Peek() {
        std::lock_guard<std::mutex> lock(mtx_);

        if (queue_.size() == 0) {
            throw std::out_of_range("Out of range");
        }

        return queue_.front();
    }

    void Pop() {
        std::lock_guard<std::mutex> lock(mtx_);

        if (queue_.size() == 0) {
            throw std::out_of_range("Out of range");
        }

        return queue_.pop();
    }

    size_t Size() {
        std::lock_guard<std::mutex> lock(mtx_);
        return queue_.size();
    }

  private:
    std::mutex mtx_;
    std::queue<T> queue_;
};

}  // namespace ttp

#endif /* TTP_BLOCKING_QUEUE_HPP_ */
