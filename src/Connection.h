#ifndef _LIBY_CPP_CONNECTION_H_
#define _LIBY_CPP_CONNECTION_H_

#include "Buffer.h"
#include "TimerMixin.h"
#include "util.h"

namespace Liby {

struct io_task final {
    // for sendfile

    io_task() = default;
    io_task(const io_task &that);
    io_task(io_task &&that) = default;

    std::shared_ptr<FileDescriptor> fp_;
    off_t offset_ = 0;
    off_t len_ = 0;

    std::shared_ptr<Buffer> buffer_;
    std::unique_ptr<BasicHandler> handler_;
};

class Connection final : clean_,
                         public std::enable_shared_from_this<Connection>,
                         public TimerMixin {
public:
    Connection(EventLoop *loop, const SockPtr &socket);
    Connection(const SockPtr &socket);
    //    virtual ~Connection();

    Connection &setChannel(ChanPtr chan) {
        assert(chan);
        chan_ = chan;
        return *this;
    }

    Poller *getPoller() {
        assert(poller_);
        return poller_;
    }

    SockPtr &getSocket() { return socket_; }

    Connection &onRead(ConnCallback cb) {
        readEventCallback_ = cb;
        return *this;
    }
    // Connection &onWrit(ConnCallback cb);
    Connection &onErro(ConnCallback cb) {
        erroEventCallback_ = cb;
        return *this;
    }

    void init();
    void init1();

    void destroy();

    void setErro();

    void sendFile(const fdPtr &fp, off_t off, off_t len);

    void runEventHandler(const BasicHandler &handler);

    // void send(void *base, off_t len);
    // void send(Buffer &buf);
    // void send(Buffer &&buf);
    // void send(const char *buf);
    // void send(const std::string &str);

    template <typename T>
    void send_object(T &&t, const BasicHandler &handler = nullptr) {
        send((const char *)(&t), sizeof(t), handler);
    }
    void send(const char *base, off_t len,
              const BasicHandler &handler = nullptr);
    void send(Buffer &buf, const BasicHandler &handler = nullptr);
    void send(Buffer &&buf, const BasicHandler &handler = nullptr);
    void send(const char *buf, const BasicHandler &handler = nullptr);
    void send(const std::string &str, const BasicHandler &handler = nullptr);

    Buffer &read() { return readBuf_; }

    int sync_read(char *buf, size_t len);
    int sync_send(Buffer &buffer);

    std::shared_ptr<BaseContext> &context() { return context_; }

    Connection &enableRead(bool flag = true);
    Connection &enableWrit(bool flag = true);

private:
    void handleReadEvent();
    void handleWritEvent();
    void handleErroEvent();

public:
    std::shared_ptr<BaseContext> context_;

private:
    bool destroy_ = false;
    Poller *poller_ = nullptr;
    EventLoop *loop_ = nullptr;
    ConnPtr self_;
    SockPtr socket_;
    ChanPtr chan_;
    Buffer readBuf_;
    std::deque<io_task> writTasks_;
    ConnCallback readEventCallback_;
    ConnCallback erroEventCallback_;
};
}

#endif
