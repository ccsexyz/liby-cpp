#ifndef LIBY_CPP_BLOCKINGQUEUE_H
#define LIBY_CPP_BLOCKINGQUEUE_H

#include <condition_variable>
#include <initializer_list>
#include <list>
#include <mutex>

namespace Liby {
template <typename T> class BlockingQueue {
public:
    using value_type = T;
    using reference_type = T &;
    using const_reference_type = const T &;

    BlockingQueue(std::initializer_list<T> elements = {}) {
        for (auto &e : elements) {
            queue_.emplace_back(e);
        }
    }

    void push_notify(const_reference_type e) {
        std::lock_guard<std::mutex> G_(mutex_);
        queue_.emplace_back(e);
        cond_.notify_one();
    }

    void push_broadcast(const_reference_type e) {
        std::lock_guard<std::mutex> G_(mutex_);
        queue_.emplace_back(e);
        cond_.notify_all();
    }

    void push_back(const_reference_type e) {
        std::lock_guard<std::mutex> G_(mutex_);
        queue_.emplace_back(e);
    }

    T pop_front() {
        std::lock_guard<std::mutex> G_(mutex_);
        return pop_front_imp();
    }

    void pop() {
        std::lock_guard<std::mutex> G_(mutex_);
        queue_.pop_front();
    }

    T pop_wait() {
        std::unique_lock<std::mutex> lck(mutex_);
        cond_.wait(lck, [this] { return !queue_.empty(); });
        return pop_front_imp();
    }

    T &front() { return queue_.front(); }

    bool empty() { return queue_.empty(); }

private:
    T pop_front_imp() {
        auto t = queue_.front();
        queue_.pop_front();
        return t;
    }

private:
    std::mutex mutex_;
    std::list<T> queue_;
    std::condition_variable cond_;
};
}

#endif // LIBY_CPP_BLOCKINGQUEUE_H
