#include "PollerSelect.h"
#include "Channel.h"

using namespace Liby;

PollerSelect::PollerSelect() {
}

void PollerSelect::addChanel(Channel *ch) {
    assert(ch && ch->get_fd() >= 0);

    int fd = ch->get_fd();
    if (fd >= FD_SETSIZE) {
        throw "fd >= 1024";
    }

    if (fd >= maxfd_) {
        maxfd_ = fd + 1;
    }

    setChannel(fd, ch);

    if (ch->readable())
        FD_SET(fd, &rset_);
    if (ch->writable())
        FD_SET(fd, &wset_);
    FD_SET(fd, &eset_);
}

void PollerSelect::updateChanel(Channel *ch, bool readable, bool writable) {
    assert(ch && ch->get_fd() >= 0);

    int fd = ch->get_fd();

    if (ch->readable()) {
        FD_SET(fd, &rset_);
    } else {
        FD_CLR(fd, &rset_);
    }

    if (ch->writable()) {
        FD_SET(fd, &wset_);
    } else {
        FD_CLR(fd, &wset_);
    }
}

void PollerSelect::removeChanel(Channel *ch) {
    assert(ch && ch->get_fd() >= 0);

    int fd = ch->get_fd();
    FD_CLR(fd, &rset_);
    FD_CLR(fd, &wset_);
    FD_CLR(fd, &eset_);

    verbose("try to remove fd %d channel %p", fd, ch);

    setChannel(fd, nullptr);

    if (fd == maxfd_) {
        while (--maxfd_) {
            if (FD_ISSET(maxfd_, &rset_) || FD_ISSET(maxfd_, &wset_) ||
                FD_ISSET(maxfd_, &eset_)) {
                break;
            }
        }
    }
}

void PollerSelect::loop_once(Timestamp *ts) {
    struct timeval timeout;
    struct timeval *pto = nullptr;
    if (ts != nullptr) {
        Timestamp now = Timestamp::now();
        if (*ts < now) {
            timeout = {0, 0};
        } else {
            Timestamp inter = *ts - now;
            timeout = {.tv_sec = inter.sec(), .tv_usec = inter.usec()};
        }
        pto = &timeout;
    }

    fd_set rset = rset_;
    fd_set wset = wset_;
    fd_set eset = eset_;

    int nready = ::select(maxfd_, &rset, &wset, &eset, pto);
    errorif(nready == -1, "select: %s", ::strerror(errno));

    for (int fd = 0; fd < maxfd_ && nready > 0; fd++) {
        bool flag = false;
        if (FD_ISSET(fd, &eset)) {
            flag = true;
            Channel *ch = getChannel(fd);
            ch->handleErroEvent();
            continue;
        }
        if (FD_ISSET(fd, &rset)) {
            flag = true;
            Channel *ch = getChannel(fd);
            ch->handleReadEvent();
        }
        if (FD_ISSET(fd, &wset)) {
            flag = true;
            Channel *ch = getChannel(fd);
            ch->handleWritEvent();
        }
    }
}
