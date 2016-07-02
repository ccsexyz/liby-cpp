#include "EventLoop.h"
#include "EventQueue.h"
#include "PollerEpoll.h"
#include "PollerKevent.h"
#include "PollerPoll.h"
#include "PollerSelect.h"
#include "TimerQueue.h"

using namespace Liby;

__thread EventLoop *th_loop = nullptr;

EventLoop::EventLoop(const std::string &chooser) {
    if (chooser == "POLL") {
        poller_ = std::make_unique<PollerPoll>();
    } else if (chooser == "SELECT") {
        poller_ = std::make_unique<PollerSelect>();
    } else {
#ifdef __linux__
        poller_ = std::make_unique<PollerEpoll>();
#elif defined(__APPLE__)
        poller_ = std::make_unique<PollerKevent>();
#endif
    }

    equeue_ = std::make_unique<EventQueue>(poller_.get());
    tqueue_ = std::make_unique<TimerQueue>(poller_.get());
}

void EventLoop::bind_to_thread() {
    if (th_loop) {
        throw "th_loop cannot be bound more than one time";
    } else {
        th_loop = this;
    }

    poller_->bind_to_thread();
}

EventLoop *EventLoop::curr_thread_loop() {
    assert(th_loop);
    return th_loop;
}

void EventLoop::run(BoolFunctor bf) {
    equeue_->start();
    tqueue_->start();

    while (bf()) {
#ifdef __APPLE__
        tqueue_->handleTimeoutEvents();

        Timestamp min;
        const Timestamp *pmin = tqueue_->getMinTimestamp();
        Timestamp *p = nullptr;
        if (pmin != nullptr) {
            min = *pmin;
            p = &min;
        }

        poller_->loop_once(p);

        tqueue_->handleTimeoutEvents();
#elif defined(__linux__)

        poller_->loop_once(p);

#endif
    }
}

void EventLoop::wakeup() {
    equeue_->wakeup();
}

void EventLoop::runEventHandler(const BasicHandler &handler) {
    equeue_->pushHandler(handler);
    equeue_->wakeup();
}

TimerHolder EventLoop::runAt(const Timestamp &timestamp,
                             const BasicHandler &handler) {
    return tqueue_->insert(timestamp, handler);
}

TimerHolder EventLoop::runAfter(const Timestamp &timestamp,
                                const BasicHandler &handler) {
    return tqueue_->insert(timestamp + Timestamp::now(), handler);
}

TimerHolder EventLoop::runEvery(const Timestamp &timestamp,
                                const BasicHandler &handler) {
    Timer timer;
    TimerHolder holder(timer);
    WeakTimerHolder weak_holder(holder);
    Timer &timer1 = holder.getTimer();
    timer1 = Timer(timestamp, [this, handler, weak_holder, timestamp] {
        if (!weak_holder.getWeakTimerPtr().expired()) {
            auto timer = weak_holder.getWeakTimerPtr().lock();
            timer->setTimeout(timestamp + Timestamp::now());
            tqueue_->insert(weak_holder.getWeakTimerPtr().lock());
            handler();
        }
    });
    tqueue_->insert(holder.getTimerPtr());
    return holder;
}

EventLoop *EventLoop::robinLoop1(int fd) {
    return rf1_(fd);
}

EventLoop *EventLoop::robinLoop(int fd) {
    return rf_(fd);
}
