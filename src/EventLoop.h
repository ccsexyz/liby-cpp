#ifndef LIBY_CPP_EVENTLOOP_H
#define LIBY_CPP_EVENTLOOP_H

#include "TimerQueue.h"
#include "util.h"

namespace Liby {
class Poller;
class EventQueue;
class TimerQueue;
class EventLoop final : clean_ {
public:
    EventLoop(const std::string &chooser = "EPOLL");
    Poller *getPoller() const {
        assert(poller_);
        return &*poller_;
    }

    EventLoop *robinLoop(int fd);
    EventLoop *robinLoop1(int fd);

    EventLoop &setRobinFunctor(const RobinFunctor &rf) {
        rf_ = rf;
        return *this;
    }
    EventLoop &setRobinFunctor1(const RobinFunctor &rf1) {
        rf1_ = rf1;
        return *this;
    }

    static EventLoop *curr_thread_loop();

    void bind_to_thread();

    void run(BoolFunctor bf);
    void wakeup();
    TimerHolder runAt(const Timestamp &timestamp, const BasicHandler &handler);
    TimerHolder runAfter(const Timestamp &timestamp,
                         const BasicHandler &handler);
    TimerHolder runEvery(const Timestamp &timestamp,
                         const BasicHandler &handler);

    void runEventHandler(const BasicHandler &handler);

private:
private:
    std::unique_ptr<Poller> poller_;
    std::unique_ptr<EventQueue> equeue_;
    std::unique_ptr<TimerQueue> tqueue_;
    RobinFunctor rf_;
    RobinFunctor rf1_;
};
}

#endif // LIBY_CPP_EVENTLOOP_H
