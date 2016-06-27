#ifndef LIBY_CPP_CHANNEL_H
#define LIBY_CPP_CHANNEL_H

#include "util.h"

// channel use fd, but don't hold fd

namespace Liby {
class Poller;
class Channel final : clean_ {
public:
    Channel() = default;
    Channel(Poller *poller, int fd) : fd_(fd), poller_(poller) {
        assert(fd >= 0 && poller);
    }
    ~Channel();
    Channel &setPoller(Poller *poller) {
        assert(poller);
        poller_ = poller;
        return *this;
    }
    Channel &set_fd(int fd) {
        assert(fd);
        fd_ = fd;
        return *this;
    }
    int get_fd() { return fd_; }
    Channel &enableRead(bool flag = true) {
        readable_ = flag;
        return *this;
    }
    Channel &enableWrit(bool flag = true) {
        writable_ = flag;
        return *this;
    }
    bool readable() { return readable_; }
    bool writable() { return writable_; }

    Channel &onRead(const BasicHandler &cb) {
        readEventCallback_ = cb;
        return *this;
    }
    Channel &onWrit(const BasicHandler &cb) {
        writEventCallback_ = cb;
        return *this;
    }
    Channel &onErro(const BasicHandler &cb) {
        erroEventCallback_ = cb;
        return *this;
    }

    bool isEventChanged() {
        return old_readable_ != readable_ || old_writable_ != writable_;
    }
    void changeEvent() {
        old_writable_ = writable_;
        old_readable_ = readable_;
    }

    void removeChannel();
    void updateChannel();
    void addChannel();

    void handleReadEvent();
    void handleWritEvent();
    void handleErroEvent();

private:
    BasicHandler readEventCallback_;
    BasicHandler writEventCallback_;
    BasicHandler erroEventCallback_;

    bool readable_ = false;
    bool writable_ = false;
    bool old_readable_ = false;
    bool old_writable_ = false;
    int fd_ = -1;
    Poller *poller_ = nullptr;
};
}

#endif // LIBY_CPP_CHANNEL_H
