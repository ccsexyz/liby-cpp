#ifndef LIBY_CPP_POLLER_H
#define LIBY_CPP_POLLER_H

#include "util.h"

namespace Liby {
class Channel;

class Poller : clean_ {
public:
    Poller();

    virtual ~Poller() {}

    void bind_to_thread();

    static Poller *curr_thread_poller();

    Channel *getChannel(int fd) { return channels_[fd]; }

    void setChannel(int fd, Channel *ch) { channels_[fd] = ch; }

public:
    virtual void addChanel(Channel *ch) = 0;

    virtual void updateChanel(Channel *ch, bool readable, bool writable) = 0;

    virtual void removeChanel(Channel *ch) = 0;

    virtual void loop_once() = 0;

private:
    std::vector<Channel *> channels_;
};
}

#endif // LIBY_CPP_POLLER_H
