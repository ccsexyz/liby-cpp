#include "TcpServer.h"
#include "Channel.h"
#include "Connection.h"
#include "EventLoop.h"
#include "Poller.h"
#include "Socket.h"

using namespace Liby;

TcpServer::TcpServer() : TcpServer(EventLoop::curr_thread_loop()) {}

TcpServer::TcpServer(EventLoop *loop)
    : poller_(loop->getPoller()), loop_(loop) {}

void TcpServer::start() {
    listenfd_ = socket_->fd();
    chan_ = std::make_unique<Channel>(poller_, listenfd_);
    chan_->enableRead()
        .onRead([this] { handleAcceptEvent(); })
        .onErro([this] { handleErroEvent(); })
        .addChannel();
}

void TcpServer::initConnection(Connection &conn) {
    conn.onRead(readEventCallback_).onErro([this](Connection &conn) {
        auto x = conn.shared_from_this();
        if (erroEventCallback_) {
            erroEventCallback_(conn);
        }
        x->destroy();
    });
}

void TcpServer::handleAcceptEvent() {
    assert(listenfd_ >= 0);
    for (;;) {
        try {
            auto sock = socket_->accept();
            if (!sock) {
                return;
            }
            assert(sock && loop_);
            ConnPtr connPtr = std::make_shared<Connection>(
                loop_->robinLoop(sock->fd()), sock);
            initConnection(*connPtr);
            connPtr->setChannel(
                std::make_shared<Channel>(connPtr->getPoller(), sock->fd()));
            connPtr->runEventHandler([this, connPtr] {
                connPtr->init();
                connPtr->init1();
                connPtr->enableRead();
                if (acceptorCallback_) {
                    acceptorCallback_(*connPtr);
                }
            });
        } catch (const char *err) {
            debug("accept: %s", err);
            handleErroEvent();
        }
    }
}

void TcpServer::handleErroEvent() {
    chan_.reset();
    acceptorCallback_ = nullptr;
    readEventCallback_ = nullptr;
    erroEventCallback_ = nullptr;
}
