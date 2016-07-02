#ifndef LIBY_CPP_POLLERKQUEUE_H
#define LIBY_CPP_POLLERKQUEUE_H

#include "Poller.h"
#include <sys/event.h>
#include <sys/time.h>

namespace Liby {
class PollerKevent final : public Poller {
public:
    PollerKevent();

    virtual void addChanel(Channel *ch) override;
    virtual void updateChanel(Channel *ch, bool readable,
                              bool writable) override;
    virtual void removeChanel(Channel *ch) override;
    virtual void loop_once(Timestamp *ts) override;

private:
    void updateKevents();

private:
    int kq_ = -1;
    fdPtr evPtr_;
    int eventsSize_ = 0;
    enum { defaultKeventSize = 48 };
    std::vector<struct kevent> events_;
    std::vector<struct kevent> changes_;
};
}

#endif // LIBY_CPP_POLLERKQUEUE_H
