#include "PollerPoll.h"
#include "Channel.h"

using namespace Liby;

PollerPoll::PollerPoll() {
    pollfds_.resize(defaultPollSize);
}

void PollerPoll::addChanel(Channel *ch) {
    assert(ch && ch->get_fd() >= 0);

    int fd = ch->get_fd();

    struct pollfd pollfd1;
    pollfd1.fd = fd;
#ifdef __linux__
    pollfd1.events = POLLRDHUP | POLLERR;
#elif defined(__APPLE__)
    pollfd1.events = POLLERR | POLLHUP;
#endif
    if (ch->readable())
        pollfd1.events |= POLLIN;
    if (ch->writable())
        pollfd1.events |= POLLOUT;

    if (pollfds_.size() <= static_cast<decltype(pollfds_.size())>(fd))
        pollfds_.resize(fd * 2 + 1);

    pollfds_[fd] = pollfd1;
    setChannel(fd, ch);
}

void PollerPoll::updateChanel(Channel *ch, bool readable, bool writable) {
    assert(ch && ch->get_fd() >= 0);

    int fd = ch->get_fd();
    struct pollfd *p = &pollfds_[fd];

    if (ch->readable()) {
        p->events |= POLLIN;
    } else {
        p->events &= ~POLLIN;
    }

    if (ch->writable()) {
        p->events |= POLLOUT;
    } else {
        p->events &= ~POLLOUT;
    }
}

void PollerPoll::removeChanel(Channel *ch) {
    assert(ch && ch->get_fd() >= 0);

    int fd = ch->get_fd();
    setChannel(fd, nullptr);
    pollfds_[fd].fd = -1;
}

void PollerPoll::loop_once(Timestamp *ts) {
    int interMs = -1;
    if (ts != nullptr) {
        Timestamp now = Timestamp::now();
        if (*ts < now) {
            interMs = 0;
        } else {
            interMs = static_cast<int>((*ts - now).toMillSec());
        }
    }

    int nready = ::poll(&pollfds_[0], pollfds_.size(), interMs);

    for (int i = 0; i < pollfds_.size() && nready > 0; i++) {
        bool flag = false;
        int fd = pollfds_[i].fd;
        int revent = pollfds_[i].revents;

        if (fd < 0)
            continue;

        if (revent & POLLERR
#ifdef __linux__
            || revent & POLLRDHUP
#endif
            ) {
            flag = true;
            Channel *ch = getChannel(fd);
            ch->handleErroEvent();
            continue;
        }

        if (revent & POLLIN) {
            flag = true;
            Channel *ch = getChannel(fd);
            ch->handleReadEvent();
        }

        if (revent & POLLOUT) {
            flag = true;
            Channel *ch = getChannel(fd);
            ch->handleWritEvent();
        }

        if (flag == true) {
            nready--;
        }
    }
}
