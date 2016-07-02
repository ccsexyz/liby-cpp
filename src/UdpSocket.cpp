#include "UdpSocket.h"
#include "Buffer.h"
#include "Channel.h"
#include "EventLoop.h"
#include "Socket.h"
#include "UdpConnection.h"
#include <netinet/in.h>

using namespace Liby;

Buffer &UdpSocket::getReadBuf(const Liby::Endpoint &ep) {
    auto it = conns_.find(ep);
    if (it == conns_.end()) {
        throw 0;
    } else {
        return it->second->getReadBuf();
    }
}

void UdpSocket::handleReadEvent() {
    auto self = self_;

    char buffer[4096];

    for (;;) {
        static struct sockaddr_in address;
        static socklen_t addr_len = sizeof(struct sockaddr_in);

        int n = socket_->recvfrom(buffer, sizeof(buffer), 0,
                                  (struct sockaddr *)&address, &addr_len);

        if (n < 0) {
            if (errno != EAGAIN) {
                handleErroEvent();
            }
            break;
        }

        Endpoint peer(&address);
        auto it = conns_.find(peer);
        if (it == conns_.end()) {
            // a new udp conn
            auto uconn = std::make_shared<UdpConnection>(this, peer);
            conns_.insert(std::make_pair(peer, uconn));
            if (acceptorCallback_) {
                acceptorCallback_(*uconn);
            }
            uconn->getReadBuf().append(buffer, n);
            if (readEventCallback_) {
                readEventCallback_(*uconn);
            }
        } else {
            UdpConnection &uconn = *(it->second);
            uconn.getReadBuf().append(buffer, n);
            if (readEventCallback_) {
                readEventCallback_(uconn);
            }
        }
    }
}

void UdpSocket::handleWritEvent() {
    auto self = self_;
    DeferCaller([this] {
        if (destroy_)
            return;
        if (writTasks_.empty())
            enableWrit(false);
    });

    while (!writTasks_.empty()) {
        auto &first = writTasks_.front();
        auto &buffer = *(first.buffer_);

        int n = socket_->sendto(buffer.data(), buffer.size(), 0,
                                (struct sockaddr *)first.dest_,
                                sizeof(struct sockaddr_in));

        if (n < 0) {
            if (n != EAGAIN) {
                handleErroEvent();
            }
            break;
        }

        buffer.retrieve(n);
        if (buffer.size() == 0) {
            if (first.handler_ && *(first.handler_))
                (*first.handler_)();
            writTasks_.pop_front();
        }
    }
}

void UdpSocket::handleErroEvent() {
    auto self = self_;
    if (erroEventCallback_) {
        erroEventCallback_(*this);
    }
    destroy();
}

void UdpSocket::destroy() {
    if (destroy_)
        return;

    destroy_ = true;
    acceptorCallback_ = nullptr;
    readEventCallback_ = nullptr;
    erroEventCallback_ = nullptr;
    writTasks_.clear();
    conns_.clear();
    socket_.reset();
    chan_.reset();
    self_.reset();
}

UdpSocket::UdpSocket(EventLoop *loop, const SockPtr &socket)
    : sockfd_(socket->fd()), socket_(socket), poller_(loop->getPoller()),
      loop_(loop) {
    chan_ = std::make_unique<Channel>(poller_, sockfd_);
}

UdpSocket::UdpSocket(const SockPtr &socket)
    : UdpSocket(EventLoop::curr_thread_loop(), socket) {
}

void UdpSocket::start() {
    self_ = shared_from_this();
    chan_->onRead([this] { handleReadEvent(); })
        .onWrit([this] { handleWritEvent(); })
        .onErro([this] { handleErroEvent(); })
        .enableRead()
        .addChannel();
}

UdpSocket &UdpSocket::enableWrit(bool flag) {
    chan_->enableWrit(flag);
    chan_->updateChannel();
    return *this;
}

UdpSocket &UdpSocket::enableRead(bool flag) {
    chan_->enableRead(flag);
    chan_->updateChannel();
    return *this;
}

void UdpSocket::handleConnectEvent(const Endpoint &ep) {
    auto it = conns_.find(ep);
    if (it != conns_.end())
        return;

    auto uconn = std::make_shared<UdpConnection>(this, ep);
    conns_.insert(std::make_pair(ep, uconn));
    if (connectorCallback_) {
        connectorCallback_(*uconn);
    }
}
