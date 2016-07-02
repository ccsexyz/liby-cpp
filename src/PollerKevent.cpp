#include "PollerKevent.h"
#include "Channel.h"
#include "FileDescriptor.h"

using namespace Liby;

PollerKevent::PollerKevent() {
    kq_ = ::kqueue();
    evPtr_ = std::make_shared<FileDescriptor>(kq_);
    events_.resize(defaultKeventSize);
    changes_.reserve(defaultKeventSize);
}

void PollerKevent::addChanel(Channel *ch) {
    assert(ch && ch->get_fd() >= 0);

    struct kevent changes;
    if (ch->readable()) {
        EV_SET(&changes, ch->get_fd(), EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0,
               nullptr);
        changes_.push_back(changes);
    }
    if (ch->writable()) {
        EV_SET(&changes, ch->get_fd(), EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0,
               nullptr);
        changes_.push_back(changes);
    }

    eventsSize_++;
    setChannel(ch->get_fd(), ch);
    updateKevents();
}

void PollerKevent::updateChanel(Channel *ch, bool readable, bool writable) {
    assert(ch && ch->get_fd() >= 0);

    int fd = ch->get_fd();
    struct kevent changes;
    if (ch->readable()) {
        EV_SET(&changes, fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, nullptr);
    } else {
        EV_SET(&changes, fd, EVFILT_READ, EV_DELETE, 0, 0, nullptr);
    }
    changes_.push_back(changes);
    if (ch->writable()) {
        EV_SET(&changes, fd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, nullptr);
    } else {
        EV_SET(&changes, fd, EVFILT_WRITE, EV_DELETE, 0, 0, nullptr);
    }
    changes_.push_back(changes);
}

void PollerKevent::removeChanel(Channel *ch) {
    assert(ch && ch->get_fd() >= 0);

    int fd = ch->get_fd();
    struct kevent changes;
    EV_SET(&changes, fd, EVFILT_READ, EV_DELETE, 0, 0, nullptr);
    changes_.push_back(changes);
    EV_SET(&changes, fd, EVFILT_WRITE, EV_DELETE, 0, 0, nullptr);
    changes_.push_back(changes);
    eventsSize_--;

    setChannel(fd, nullptr);
}

void PollerKevent::loop_once(Timestamp *ts) {
    struct timespec timeout;
    struct timespec *pto = nullptr;
    if (ts != nullptr) {
        Timestamp now = Timestamp::now();
        if (now < *ts) {
            Timestamp inter = *ts - now;
            timeout = {.tv_sec = inter.sec(), .tv_nsec = inter.usec() * 1000};
        } else {
            timeout = {.tv_sec = 0, .tv_nsec = 0};
        }
        pto = &timeout;
    }

    int nready = ::kevent(kq_, nullptr, 0, &events_[0], events_.size(), pto);

    for (int i = 0; i < nready; i++) {
        struct kevent &event = events_[i];
        int fd = static_cast<int>(event.ident);

        Channel *ch = getChannel(fd);
        if (!ch) {
            continue;
        }

        if (event.flags & EV_EOF || event.flags & EV_ERROR) {
            ch->handleErroEvent();
            continue;
        }
        if (event.filter == EVFILT_READ) {
            ch->handleReadEvent();
        }
        if (event.filter == EVFILT_WRITE) {
            ch->handleWritEvent();
        }
    }

    updateKevents();

    if (eventsSize_ > static_cast<int>(events_.size())) {
        events_.resize(
            static_cast<decltype(events_.size())>(eventsSize_ * 2 + 1));
    }
}

void PollerKevent::updateKevents() {
    if (changes_.empty()) {
        return;
    }

    size_t index = 0;
    size_t start = 0;
    size_t n = changes_.size();

    for (; index < n; index++) {
        start = index;
        struct kevent *p = &changes_[index];

        for (; index < n && getChannel(changes_[index].ident) != nullptr;
             index++)
            ;

        if (index != start) {
            ::kevent(kq_, &changes_[start], index - start, nullptr, 0, nullptr);
        }
    }

    changes_.clear();
}
