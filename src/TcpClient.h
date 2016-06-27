#ifndef LIBY_CPP_TCPCLIENT_H
#define LIBY_CPP_TCPCLIENT_H

#include "EventLoop.h"
#include "TimerMixin.h"
#include "util.h"

namespace Liby {
class TcpClient final : clean_,
                        public std::enable_shared_from_this<TcpClient>,
                        public TimerMixin {
public:
    TcpClient() = default;
    TcpClient(EventLoop *loop, SockPtr socket);
    TcpClient &onConnect(const Connector &connector);
    void start();

    TcpClient &setSocket(SockPtr socket) {
        assert(socket);
        socket_ = socket;
        return *this;
    }

    TcpClient &setEventLoop(EventLoop *loop) {
        assert(loop && loop->getPoller());
        loop_ = loop;
        poller_ = loop->getPoller();
        return *this;
    }

public:
    std::shared_ptr<BaseContext> context_;
    std::shared_ptr<BaseContext> &context() { return context_; }

private:
    void destroy();

private:
    EventLoop *loop_;
    Poller *poller_;
    SockPtr socket_;
    ChanPtr chan_;
    Connector connector_;
    std::shared_ptr<TcpClient> self_;
};
}

#endif // LIBY_CPP_TCPCLIENT_H
