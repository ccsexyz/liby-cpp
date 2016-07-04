#include "TcpClient.h"
#include "Channel.h"
#include "Connection.h"
#include "EventLoop.h"
#include "Poller.h"
#include "Socket.h"

using namespace Liby;

TcpClient::TcpClient(EventLoop *loop, SockPtr socket)
    : loop_(loop), poller_(loop->getPoller()), socket_(socket) {}

void TcpClient::start() {
    //    assert(connector_);
    assert(loop_ && socket_ && poller_);
    chan_ = std::make_shared<Channel>(poller_, socket_->fd());
    chan_->enableRead(false)
        .enableWrit()
        .onWrit([this] {
            auto x = shared_from_this();
//            if (!socket_) {
//                auto p = chan_.get();
//                socket_.reset();
//            }
            ConnPtr connPtr = std::make_shared<Connection>(loop_, socket_);
            connPtr->setChannel(chan_);
            if (context_) {
                connPtr->context_ = context_;
            }
            Connector connector = connector_;

            loop_->nextTick([&, connPtr, connector]{
                chan_->onWrit(nullptr);
                chan_->onErro(nullptr);
                destroy();

                connPtr->init();
                connPtr->enableRead();
                if (connector) {
                    connector(connPtr);
                }
            });

            chan_->enableRead(false);
            chan_->enableWrit(false);
            chan_->updateChannel();
        })
        .onErro([this] {
            auto x = shared_from_this();
            if (connector_) {
                connector_(nullptr);
            }
            destroy();
        })
        .addChannel();
    self_ = shared_from_this();
}

TcpClient &TcpClient::onConnect(const Connector &connector) {
    connector_ = connector;
    return *this;
}

void TcpClient::destroy() {
    poller_ = nullptr;
    loop_ = nullptr;
    chan_.reset();
    socket_
        .reset(); // socket_.reset必须在chan_之后,确保chan_析构时其使用的fd是有效的
    connector_ = nullptr;
    self_.reset();
}
