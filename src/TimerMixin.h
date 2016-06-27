#ifndef LIBY_CPP_TIMERMIXIN_H
#define LIBY_CPP_TIMERMIXIN_H

#include "TimerQueue.h"
#include "util.h"
#include <unordered_map>

namespace Liby {
class TimerMixin {
public:
    TimerMixin();
    virtual ~TimerMixin();
    TimerId runAt(const Timestamp &timestamp, const BasicHandler &handler);
    TimerId runAfter(const Timestamp &timestamp, const BasicHandler &handler);
    TimerId runEvery(const Timestamp &timestamp, const BasicHandler &handler);
    void cancelTimer(TimerId id);
    void cancelAllTimer();

private:
    TimerId runHelper(TimerHolder &&timerHolder);

private:
    EventLoop *loop_;
    std::unordered_map<TimerId, TimerHolder> timerHolders_;
};
}

#endif // LIBY_CPP_TIMERMIXIN_H
