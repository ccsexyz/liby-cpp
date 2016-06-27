#include "EventLoopGroup.h"
#include "Channel.h"
#include "Connection.h"
#include "EventQueue.h"
#include "Poller.h"
#include "Resolver.h"
#include "Socket.h"
#include "TcpClient.h"
#include "TcpServer.h"
#include <mutex>
#include <signal.h>

using namespace Liby;

EventLoopGroup::EventLoopGroup(int n, const std::string &chooser)
        : n_(n), chooser_(chooser) {
    for (int i = 0; i < n + 1; i++) {
        loops_.emplace_back(std::make_shared<EventLoop>(chooser));
        ploops_.push_back(loops_.back().get());
    }
    ploops_[0]->bind_to_thread();
}

void EventLoopGroup::run(BoolFunctor bf) {
    Signal::signal(SIGPIPE, [] { error("receive SIGPIPE"); });

    bf_ = bf;
    for (int i = 0; i < n_; i++) {
        threads_.emplace_back([this](int index) { worker_thread(index); },
                i + 1);
    }

    worker_thread(0);

    for (int i = 0; i < n_; i++) {
        loops_[i + 1]->wakeup();
    }
}

void EventLoopGroup::worker_thread(int index) {
    assert(index >= 0 && index <= n_);

    EventLoop &loop = *loops_[index];

    if (loop.curr_thread_loop() == nullptr)
        loop.bind_to_thread();

    loop.setRobinFunctor([this](int fd) { return robinLoop(fd); });
    loop.setRobinFunctor1([this](int fd) { return robinLoop1(fd); });

    loop.run([this] {
        static std::mutex m;
        std::lock_guard<std::mutex> G_(m);
        return bf_();
    });
}

EventLoop *EventLoopGroup::robinLoop1(int fd) {
    if (n_ == 0) {
        return ploops_[0];
    } else {
        return ploops_[fd % n_ + 1];
    }
}

EventLoop *EventLoopGroup::robinLoop(int fd) { return ploops_[fd % (n_ + 1)]; }

std::shared_ptr<TcpServer> EventLoopGroup::creatTcpServer(const std::string &host,
                                                          const std::string &service) {
    auto eps = Resolver::resolve(host, service);
    if (eps.empty()) {
        error("host or service unavaiable");
        return nullptr;
    }

    SockPtr sockPtr = std::make_shared<Socket>(eps.front());
    sockPtr->listen();

    std::shared_ptr<TcpServer> tcpServer =
            std::make_shared<TcpServer>(ploops_[0]);
    tcpServer->setServerSocket(sockPtr);

    tcpServer->start();

    return tcpServer;
}

std::shared_ptr<TcpClient> EventLoopGroup::creatTcpClient(const std::string &host,
                                                          const std::string &service) {
    std::shared_ptr<TcpClient> tcpClient = std::make_shared<TcpClient>();
    std::weak_ptr<TcpClient> weakTcpClient = tcpClient;

    Resolver::async_resolve(
            host, service, [this, weakTcpClient](std::list<Endpoint> eps) {
                if (weakTcpClient.expired() || eps.empty())
                    return;

                auto tcpClient = weakTcpClient.lock();
                SockPtr sockPtr = std::make_shared<Socket>(eps.front());
                sockPtr->connect();

                tcpClient->setEventLoop(robinLoop1(sockPtr->fd()));
                tcpClient->setSocket(sockPtr);

                tcpClient->start();
            });
    return tcpClient;
}
