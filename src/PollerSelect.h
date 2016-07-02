#ifndef LIBY_CPP_POLLERSELECT_H
#define LIBY_CPP_POLLERSELECT_H

#include "Poller.h"
#include <sys/select.h>

namespace Liby {
class PollerSelect final : public Poller {
public:
    PollerSelect();
    virtual void addChanel(Channel *ch) override;
    virtual void updateChanel(Channel *ch, bool readable,
                              bool writable) override;
    virtual void removeChanel(Channel *ch) override;
    virtual void loop_once(Timestamp *ts) override;

private:
    fd_set rset_; // read event set
    fd_set wset_; // write event set
    fd_set eset_; // error event set
    int maxfd_ = -1;
};
}

#endif // LIBY_CPP_POLLERSELECT_H
