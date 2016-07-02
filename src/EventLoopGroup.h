#ifndef _LIBY_CPP_EVENTLOOPGROUP_H_
#define _LIBY_CPP_EVENTLOOPGROUP_H_

#include "EventLoop.h"
#include <thread>

namespace Liby {
class TcpServer;
class TcpClient;
class UdpSocket;
class EventLoopGroup final : clean_ {
public:
    EventLoopGroup(int n = 0, const std::string &chooser = "EPOLL");
    void run(BoolFunctor bf = []() -> bool { return true; });

    EventLoop *robinLoop(int fd);
    EventLoop *robinLoop1(int fd);

    std::shared_ptr<TcpServer> creatTcpServer(const std::string &host,
                                              const std::string &service);
    std::shared_ptr<TcpClient> creatTcpClient(const std::string &host,
                                              const std::string &service);
    std::shared_ptr<UdpSocket> creatUdpServer(const std::string &host,
                                              const std::string &service);
    std::shared_ptr<UdpSocket> creatUdpClient(const std::string &host,
                                              const std::string &service);
    std::shared_ptr<UdpSocket> creatUdpClient();

private:
    void worker_thread(int index);

private:
    int n_ = 0;
    BoolFunctor bf_;
    std::string chooser_;
    std::vector<std::shared_ptr<EventLoop>> loops_;
    std::vector<EventLoop *> ploops_;
    std::vector<std::thread> threads_;
};
}

#endif
