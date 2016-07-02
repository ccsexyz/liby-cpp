#ifndef LIBY_CPP_POLLERPOLL_H
#define LIBY_CPP_POLLERPOLL_H

#include "Poller.h"
#include "poll.h"

namespace Liby {
class PollerPoll final : public Poller {
public:
    PollerPoll();

    virtual void addChanel(Channel *ch) override;
    virtual void updateChanel(Channel *ch, bool readable,
                              bool writable) override;
    virtual void removeChanel(Channel *ch) override;

    virtual void loop_once(Timestamp *ts) override;

private:
    enum { defaultPollSize = 48 };
    std::vector<struct pollfd> pollfds_;
};
}

#endif // LIBY_CPP_POLLERPOLL_H
