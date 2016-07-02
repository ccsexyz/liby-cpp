#ifndef _LIBY_CPP_UDPSOCKET_H
#define _LIBY_CPP_UDPSOCKET_H

#include "Endpoint.h"
#include "util.h"
#include <map>

namespace Liby {
class UdpConnection;
class UdpSocket;
using UdpConnCallback = std::function<void(UdpConnection &uconn)>;
using UdpCallback = std::function<void(UdpSocket &usocket)>;
struct udp_io_task final {
    udp_io_task() = default;
    udp_io_task(const udp_io_task &that) {
        dest_ = that.dest_;
        buffer_ = that.buffer_;
    }
    udp_io_task(udp_io_task &&that)
        : dest_(that.dest_), buffer_(std::move(that.buffer_)),
          handler_(std::move(that.handler_)) {
    }

    struct sockaddr_in *dest_;
    std::shared_ptr<Buffer> buffer_;
    std::unique_ptr<BasicHandler> handler_;
};

class UdpSocket : clean_, public std::enable_shared_from_this<UdpSocket> {
public:
    UdpSocket(EventLoop *loop, const SockPtr &socket);
    UdpSocket(const SockPtr &socket);

    void start();

    Buffer &getReadBuf(const Endpoint &ep);

    void send(const udp_io_task &task) {
        writTasks_.push_back(task);
        enableWrit();
    }
    void send(udp_io_task &&task) {
        writTasks_.emplace_back(std::move(task));
        enableWrit();
    }

    UdpSocket &onRead(const UdpConnCallback &cb) {
        readEventCallback_ = cb;
        return *this;
    }

    UdpSocket &onErro(const UdpCallback &cb) {
        erroEventCallback_ = cb;
        return *this;
    }

    UdpSocket &onAccept(const UdpConnCallback &cb) {
        acceptorCallback_ = cb;
        return *this;
    }

    UdpSocket &onConnect(const UdpConnCallback &cb) {
        connectorCallback_ = cb;
        return *this;
    }

    UdpSocket &enableRead(bool flag = true);
    UdpSocket &enableWrit(bool flag = true);

    void destroy();
    void destroyConn(const Endpoint &ep) {
        conns_.erase(ep);
    }

    void handleConnectEvent(const Endpoint &ep);

private:
    void handleReadEvent();
    void handleWritEvent();
    void handleErroEvent();

private:
    bool destroy_ = false;
    int sockfd_ = -1;
    SockPtr socket_;
    Poller *poller_ = nullptr;
    EventLoop *loop_ = nullptr;
    std::unique_ptr<Channel> chan_;
    std::map<Endpoint, std::shared_ptr<UdpConnection>> conns_;
    std::deque<udp_io_task> writTasks_;
    UdpConnCallback acceptorCallback_;
    UdpConnCallback readEventCallback_;
    UdpConnCallback connectorCallback_;
    UdpCallback erroEventCallback_;
    std::shared_ptr<UdpSocket> self_;
};
}

#endif
