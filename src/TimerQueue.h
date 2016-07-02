#ifndef LIBY_CPP_TIMERQUEUE_H
#define LIBY_CPP_TIMERQUEUE_H

#include "BinaryHeap.h"
#include "util.h"
#include <atomic>
#include <set>

namespace Liby {

class Timer : public std::enable_shared_from_this<Timer> {
public:
    Timer() : id_(0), timeout_(), handler_() {
    }

    Timer(const Timestamp &timeout, const BasicHandler &handler)
        : id_(timerIds_++), timeout_(timeout), handler_(handler) {
    }

    Timer(TimerId id, const Timestamp &timeout, const BasicHandler &handler)
        : id_(id), timeout_(timeout), handler_(handler) {
    }

    Timer(const Timer &that)
        : id_(that.id_), timeout_(that.timeout_), handler_(that.handler_) {
    }

    Timer &operator=(const Timer &that) {
        id_ = that.id_;
        timeout_ = that.timeout_;
        handler_ = that.handler_;
        return *this;
    }

    static TimerId getOneValidId() {
        return timerIds_++;
    }

    //        ~Timer() {
    //            info("Timer deconstruct!");
    //        }
    uint64_t id() const {
        return id_;
    }

    const Timestamp &timeout() const {
        return timeout_;
    }

    void runHandler() const {
        handler_();
    }

    void setTimeout(const Timestamp &timestamp) {
        timeout_ = timestamp;
    }

private:
    static std::atomic<uint64_t> timerIds_;
    uint64_t id_;
    Timestamp timeout_;
    BasicHandler handler_;
};

inline bool operator<(const Timer &lhs, const Timer &rhs) {
    return lhs.timeout() < rhs.timeout() && lhs.id() < rhs.id();
}

class TimerHolder {
public:
    TimerHolder() = default;
    TimerHolder(const TimerHolder &that) {
        timerPtr_ = that.timerPtr_;
    }
    TimerHolder &operator=(const TimerHolder &that) {
        timerPtr_ = that.timerPtr_;
        return *this;
    }
    TimerHolder(const Timer &timer)
        : timerPtr_(std::make_shared<Timer>(timer)) {
    }
    TimerHolder(const Timestamp &timeout, const BasicHandler &handler)
        : timerPtr_(std::make_shared<Timer>(timeout, handler)) {
    }

    Timer &getTimer() const {
        assert(timerPtr_);
        return *timerPtr_;
    }

    std::shared_ptr<Timer> getTimerPtr() const {
        return timerPtr_;
    }

private:
    std::shared_ptr<Timer> timerPtr_;
};

class WeakTimerHolder {
public:
    WeakTimerHolder() = default;
    WeakTimerHolder(const TimerHolder &holder)
        : timer_(holder.getTimer().id(), holder.getTimer().timeout(), nullptr),
          weakTimerPtr_(holder.getTimerPtr()) {
    }
    WeakTimerHolder(const std::shared_ptr<Timer> &timer)
        : timer_(timer->id(), timer->timeout(), nullptr), weakTimerPtr_(timer) {
    }

    friend inline bool operator<(const WeakTimerHolder &lhs,
                                 const WeakTimerHolder &rhs) {
        return lhs.timer_.timeout() < rhs.timer_.timeout() &&
               lhs.timer_.id() < rhs.timer_.id();
    }

    Timer &getTimer() {
        return timer_;
    }

    const Timer &getTimer() const {
        return timer_;
    }

    std::weak_ptr<Timer> getWeakTimerPtr() const {
        return weakTimerPtr_;
    }

private:
    Timer timer_;
    std::weak_ptr<Timer> weakTimerPtr_;
};

class TimerQueue : clean_ {
public:
    TimerQueue(Poller *poller);

    ~TimerQueue();

    void start();

    Poller *getPoller() const {
        return poller_;
    }

    void destroy();

    TimerHolder insert(const Timer &timer);

    TimerHolder insert(const Timestamp &timeout, const BasicHandler &handler);

    void insert(const std::shared_ptr<Timer> &timer);

    void setPoller(Poller *poller);

    void handleTimeoutEvents();

    const Timestamp *getMinTimestamp();

private:
    void updateTimerfd(const Timestamp &timeout);

private:
#ifdef __linux__
    int timerfd_;
    fdPtr timerfp_;
    std::unique_ptr<Channel> timerChan_;
#endif
    Poller *poller_;
    BinaryHeap<WeakTimerHolder> queue_;
};
}
#endif // LIBY_CPP_TIMERQUEUE_H
