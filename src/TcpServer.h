#ifndef LIBY_CPP_TCPSERVER_H
#define LIBY_CPP_TCPSERVER_H

#include "TimerMixin.h"
#include "util.h"

namespace Liby {
class TcpServer final : clean_, public TimerMixin {
public:
    TcpServer();
    TcpServer(EventLoop *loop);
    void start();
    TcpServer &setServerSocket(SockPtr socket) {
        assert(socket);
        socket_ = socket;
        return *this;
    }
    TcpServer &onRead(const ConnCallback &cb) {
        readEventCallback_ = cb;
        return *this;
    }
    TcpServer &onErro(const ConnCallback &cb) {
        erroEventCallback_ = cb;
        return *this;
    }
    TcpServer &onAccept(const ConnCallback &cb) {
        acceptorCallback_ = cb;
        return *this;
    }

private:
    void initConnection(Connection &conn);
    void handleAcceptEvent();
    void handleErroEvent();

private:
    int listenfd_ = -1;
    SockPtr socket_;
    Poller *poller_ = nullptr;
    EventLoop *loop_ = nullptr;
    std::unique_ptr<Channel> chan_;
    ConnCallback acceptorCallback_;
    ConnCallback readEventCallback_;
    ConnCallback erroEventCallback_;
};
}

#endif // LIBY_CPP_TCPSERVER_H
