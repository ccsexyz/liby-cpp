#ifndef LIBY_CPP_POLLEREPOLL_H
#define LIBY_CPP_POLLEREPOLL_H

#include "Poller.h"
#ifdef __linux__
#include <sys/epoll.h>

namespace Liby {
class PollerEpoll : public Poller {
public:
    PollerEpoll();

    virtual void addChanel(Channel *ch) override;
    virtual void updateChanel(Channel *ch, bool readable,
                              bool writable) override;
    virtual void removeChanel(Channel *ch) override;
    virtual void loop_once(Timestamp *ts) override;

private:
    enum { defaultEpollSize = 48 };
    int pollerfd_ = -1;
    size_t eventsSize_ = 0;
    fdPtr pollerfp_;
    std::vector<struct epoll_event> events_;
};
}

#else

namespace Liby {
class PollerEpoll : public Poller {
public:
    PollerEpoll() = default;
};
}

#endif

#endif // LIBY_CPP_POLLEREPOLL_H
