#include "../src/Liby.h"

using namespace std;
using namespace Liby;

void print_usage() {
    cerr << "usage: ./echo_client server_name server_port concurrency "
            "[active_clients] msg_num, msg_len"
         << endl;
}

int main(int argc, char **argv) {
    //    Logger::setLevel(Logger::LogLevel::VERBOSE);
    int msg_num, msg_len, concurrency;
    atomic_int active_clients;
    atomic_int connected_clients(0);
    atomic_int finished_clients(0);
    if (argc == 7) {
        msg_num = ::atoi(argv[5]);
        msg_len = ::atoi(argv[6]);
        concurrency = ::atoi(argv[3]);
        active_clients = ::atoi(argv[4]);
    } else if (argc == 6) {
        msg_num = ::atoi(argv[4]);
        msg_len = ::atoi(argv[5]);
        active_clients = concurrency = ::atoi(argv[3]);
    } else {
        print_usage();
        return 1;
    }
    unsigned long bytesPerClients = msg_len * msg_num;

    vector<char> buf(msg_len, 'A');
    vector<unsigned long> bytes(concurrency, 0);
    vector<Connection *> conns;

    EventLoopGroup group(4, "EPOLL");
    auto start = Timestamp::now();
    for (int i = 0; i < concurrency; i++) {
        auto echo_client = group.creatTcpClient(argv[1], argv[2]);
        if (i < active_clients) {
            echo_client->runAfter(Timestamp(8, 0), [echo_client] {});
            unsigned long *pBytes = &bytes[i];
            echo_client->onConnect([&buf, pBytes, &start, &active_clients,
                                    &conns, &finished_clients,
                                    &connected_clients, bytesPerClients,
                                    &group](std::shared_ptr<Connection> conn) {
                if (!conn) {
                    return;
                }

                connected_clients++;
                conns.push_back(conn.get());
                if (connected_clients != active_clients) {
                    static int i = 0;
                    i++;
                    //                    info("%d", i);
                    conn->enableRead(false);
                } else {
                    start = Timestamp::now();
                    for (auto c : conns) {
                        c->runEventHandler([c, &buf] {
                            c->enableRead(true);
                            c->send(&buf[0], buf.size());
                        });
                    }
                }

                conn->onRead([&buf, pBytes, bytesPerClients, &active_clients,
                              &finished_clients, &group](Connection &c) {
                    *pBytes += c.read().size();
                    c.send(c.read());

                    info("herohahaha in loop %p",
                         EventLoop::curr_thread_loop());

                    if (*pBytes >= bytesPerClients) {
                        c.destroy();
                        group.robinLoop1(0)->runEventHandler(
                            [&active_clients, &finished_clients] {
                                finished_clients++;
                                if (--active_clients <= 0) {
                                    ::exit(0);
                                }
                            });
                    }
                });
                conn->onErro([&group, &active_clients](Connection &) {
                    group.robinLoop1(0)->runEventHandler([&active_clients] {
                        if (--active_clients <= 0) {
                            ::exit(0);
                        }
                    });
                });

                //                conn->send(&buf[0], buf.size());
            });
        } else {
            echo_client->runEvery(Timestamp(1000, 0), [echo_client] {});
        }
    }

    ExitCaller::call([&] {
        auto end = Timestamp::now();
        unsigned long totalBytes = bytesPerClients * finished_clients;
        info("总时间 %g 秒", (end - start).toSecF());
        info("totalBytes = %ld", totalBytes);
        info("速度 %lf MiB/s",
             totalBytes / (end - start).toSecF() / 1024 / 1024);
        info(" QPS %d", (int)(concurrency * msg_num / (end - start).toSecF()));
    });

    group.run([&active_clients] { return active_clients > 0; });

    return 0;
}
