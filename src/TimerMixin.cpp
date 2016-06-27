#include "TimerMixin.h"
#include "EventLoop.h"

using namespace Liby;

namespace {
struct TimerMixinHelper {
    TimerMixinHelper(Liby::EventLoop **loop) {
        if (!*loop) {
            //                *loop = EventLoop::curr_thread_loop();
            ;
        }
    }
};
}

TimerMixin::TimerMixin() { loop_ = EventLoop::curr_thread_loop(); }

TimerMixin::~TimerMixin() { ; }

TimerId TimerMixin::runAt(const Timestamp &timestamp,
                          const BasicHandler &handler) {
    static TimerMixinHelper helper(&loop_);
    return runHelper(loop_->runAt(timestamp, handler));
}

TimerId TimerMixin::runAfter(const Timestamp &timestamp,
                             const BasicHandler &handler) {
    static TimerMixinHelper helper(&loop_);
    return runHelper(loop_->runAfter(timestamp, handler));
}

TimerId TimerMixin::runEvery(const Timestamp &timestamp,
                             const BasicHandler &handler) {
    static TimerMixinHelper helper(&loop_);
    return runHelper(loop_->runEvery(timestamp, handler));
}

void TimerMixin::cancelTimer(TimerId id) { timerHolders_.erase(id); }

void TimerMixin::cancelAllTimer() { timerHolders_.clear(); }

TimerId TimerMixin::runHelper(TimerHolder &&timerHolder) {
    TimerId id = timerHolder.getTimer().id();
    timerHolders_[id] = timerHolder;
    return id;
}
