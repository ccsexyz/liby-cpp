#include "TimerQueue.h"
#include "Channel.h"
#include "FileDescriptor.h"
#include "Logger.h"
#include "Poller.h"
#include "TimerQueue.h"
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#ifdef __linux__
#include <sys/timerfd.h>
#endif

using namespace Liby;

std::atomic<uint64_t> Timer::timerIds_(
    1); // tiemrId will never be zero, so zero timerId is invalid

TimerQueue::TimerQueue(Poller *poller) : poller_(poller) {
#ifdef __linux__
    fatalif(poller_ == nullptr, "Poller cannot be nullptr");
    timerfd_ = ::timerfd_create(CLOCK_REALTIME, TFD_CLOEXEC | TFD_NONBLOCK);
    fatalif(timerfd_ < 0, "timerfd_create: %s", ::strerror(errno));
    timerfp_ = std::make_shared<FileDescriptor>(timerfd_);
    timerChan_ = std::make_unique<Channel>(poller_, timerfd_);
#endif
}

TimerQueue::~TimerQueue() {
#ifdef __linux__
    //　使timerChan_析构时不去调用Poller虚基类的虚函数
    if (timerChan_) {
        //        timerChan_->setPoller(nullptr);
    }
#endif
}

void TimerQueue::updateTimerfd(const Timestamp &timeout) {
#ifdef __linux
    struct itimerspec new_timer;
    new_timer.it_value.tv_sec = timeout.sec();
    new_timer.it_value.tv_nsec = timeout.usec() * 1000;
    new_timer.it_interval = {0, 0};
    int ret = ::timerfd_settime(timerfd_, TFD_TIMER_ABSTIME, &new_timer, NULL);
    errorif(ret < 0, "timer_settime: %s", ::strerror(errno));
#else
    ClearUnuseVariableWarning(timeout);
#endif
}

void TimerQueue::start() {
#ifdef __linux__
    timerChan_->enableRead();
    timerChan_->onErro([this] { destroy(); });
    timerChan_->onRead([this] {
        if (timerfd_ < 0) {
            error("timerfd < 0");
        }

        uint64_t n;
        while (1) {
            int ret = ::read(timerfd_, &n, sizeof(n));
            if (ret <= 0)
                break;
        }

        handleTimeoutEvents();
    });
    timerChan_->addChannel();
#endif
}

void TimerQueue::destroy() {
#ifdef __linux__
    timerfd_ = -1;
    timerChan_.reset();
#endif
    queue_.clear();
}

void TimerQueue::handleTimeoutEvents() {
    assert(poller_);
    if (queue_.empty()) {
        return;
    }

    auto now = Timestamp::now();
    while (!queue_.empty()) {
        WeakTimerHolder &weak_holder = queue_.find_min();
        Timer &minTimer = weak_holder.getTimer();
        if (now < minTimer.timeout()) {
            break;
        }

        if (!weak_holder.getWeakTimerPtr().expired()) {
            std::shared_ptr<Timer> TimerPtr =
                weak_holder.getWeakTimerPtr().lock();
            TimerPtr->runHandler();
        }
        verbose("delete timer id = %lu\n", minTimer.id());
        queue_.delete_min();
    }
    if (!queue_.empty()) {
        updateTimerfd(queue_.find_min().getTimer().timeout());
    }
}

TimerHolder TimerQueue::insert(const Timer &timer) {
    verbose("add timer id = %lu", timer.id());
    if (timer.id() == 0) // ignore all timer which id is zero
        throw "timer id will not be zero";

    if (queue_.empty()) {
        updateTimerfd(timer.timeout());
    }

    TimerHolder holder(timer);
    WeakTimerHolder weak_holder(holder);
    queue_.insert(weak_holder);
    return holder;
}

void TimerQueue::setPoller(Poller *poller) {
    poller_ = poller;
#ifdef __linux__
    timerChan_->setPoller(poller_);
#endif
}

TimerHolder TimerQueue::insert(const Timestamp &timeout,
                               const BasicHandler &handler) {
    TimerHolder holder(timeout, handler);
    WeakTimerHolder weak_holder(holder);

    if (queue_.empty()) {
        updateTimerfd(timeout);
    }

    queue_.insert(weak_holder);
    return holder;
}

void TimerQueue::insert(const std::shared_ptr<Timer> &timer) {
    WeakTimerHolder weakTimerHolder(timer);
    if (queue_.empty()) {
        updateTimerfd(timer->timeout());
    }
    queue_.insert(weakTimerHolder);
}

const Timestamp *TimerQueue::getMinTimestamp() {
    if (!queue_.empty()) {
        const WeakTimerHolder &minHolder = queue_.find_min();
        return &(minHolder.getTimer().timeout());
    }
    return nullptr;
}
