#include "PollerEpoll.h"
#include "Channel.h"
#include "FileDescriptor.h"
#ifdef __linux__
#include <sys/epoll.h>
#include <sys/epoll.h>

using namespace Liby;

PollerEpoll::PollerEpoll() {
    pollerfd_ = ::epoll_create1(EPOLL_CLOEXEC);
    fatalif(pollerfd_ <= 0, "%s", ::strerror(errno));
    pollerfp_ = std::make_shared<FileDescriptor>(pollerfd_);

    events_.resize(defaultEpollSize);
}

void PollerEpoll::loop_once(Timestamp *ts) {
    int nfds = ::epoll_wait(pollerfd_, &events_[0], events_.size(), -1);
    verbose("nfds = %d events_.size() = %u", nfds, events_.size());
    for (int i = 0; i < nfds; i++) {
        int fd = events_[i].data.fd;
        Channel *ch = getChannel(fd);
        verbose("event in chan %p", ch);
        if (ch == NULL)
            continue;
        if (events_[i].events & (EPOLLRDHUP | EPOLLERR)) {
            ch->handleErroEvent();
        } else if (events_[i].events & EPOLLIN) {
            ch->handleReadEvent();
        } else if (events_[i].events & EPOLLOUT) {
            ch->handleWritEvent();
        }
    }
    // update channels
    //    for(int i = 0; i < nfds; i++) {
    //        int fd = events_[i].data.fd;
    //        Channel *ch = getChannel(fd);
    //        if(ch != nullptr && ch->isEventChanged()) {
    //            ch->changeEvent();
    //            info("update channel %p %d %d", ch, ch->readable(),
    //            ch->writable());
    //            updateChanel(ch, ch->readable(), ch->writable());
    //        }
    //    }
    if (eventsSize_ > events_.size()) {
        events_.resize(eventsSize_ * 2);
    }
}

void PollerEpoll::addChanel(Channel *ch) {
    assert(ch && ch->get_fd() >= 0);

    int fd = ch->get_fd();
    struct epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLHUP | EPOLLRDHUP;

    if (ch->readable()) {
        event.events |= EPOLLIN;
    }

    if (ch->writable()) {
        event.events |= EPOLLOUT;
    }

    int ret = ::epoll_ctl(pollerfd_, EPOLL_CTL_ADD, fd, &event);

    if (ret < 0) {
        auto it = errno;
        throw ::strerror(errno);
    } else {
        setChannel(fd, ch);
        eventsSize_++;
    }

    verbose("add channel fd = %d", ch->get_fd());
}

void PollerEpoll::updateChanel(Channel *ch, bool readable, bool writable) {
    assert(ch && ch->get_fd() >= 0);

    int fd = ch->get_fd();
    struct epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLHUP | EPOLLRDHUP;

    if (readable) {
        event.events |= EPOLLIN;
    }

    if (writable) {
        event.events |= EPOLLOUT;
    }

    int ret = ::epoll_ctl(pollerfd_, EPOLL_CTL_MOD, fd, &event);

    if (ret < 0) {
        throw ::strerror(errno);
    }
}

void PollerEpoll::removeChanel(Channel *ch) {
    assert(ch && ch->get_fd() >= 0);

    int fd = ch->get_fd();
    verbose("remove fd = %d, chan = %p\n", ch->get_fd(), ch);
    setChannel(fd, nullptr);
    eventsSize_--;

    int ret = ::epoll_ctl(pollerfd_, EPOLL_CTL_DEL, fd, nullptr);

    if (ret < 0) {
        int err = errno;
        throw ::strerror(errno);
    }
}

#endif
