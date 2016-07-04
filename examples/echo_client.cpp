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
    unsigned long totalBytes = bytesPerClients * active_clients;

    vector<char> buf(msg_len, 'A');
    vector<unsigned long> bytes(concurrency, 0);

    EventLoopGroup group(4, "EPOLL");
    auto start = Timestamp::now();
    for (int i = 0; i < concurrency; i++) {
        auto echo_client = group.creatTcpClient(argv[1], argv[2]);
        if (i < active_clients) {
            echo_client->runAfter(Timestamp(8, 0), [echo_client] {});
            unsigned long *pBytes = &bytes[i];
            auto echo_client_ = echo_client.get();
            echo_client->onConnect([&buf, pBytes, &active_clients,
                                    bytesPerClients,
                                    &group](std::shared_ptr<Connection> conn) {
                if (!conn) {
                    return;
                }

                conn->onRead([&buf, pBytes, bytesPerClients, &active_clients,
                              &group](Connection &c) {
                    *pBytes += c.read().size();
                    c.send(c.read());

                    if (*pBytes >= bytesPerClients) {
                        c.destroy();
                        group.robinLoop1(0)->runEventHandler(
                            [&active_clients] {
                                active_clients--;
                            });
                    }
                });
                conn->onErro([&group, &active_clients](Connection &c) {
                    group.robinLoop1(0)->runEventHandler(
                        [&active_clients] {
                            active_clients--;
                        });
                });

                conn->send(&buf[0], buf.size());
            });
        } else {
            echo_client->runEvery(Timestamp(1000, 0), [echo_client] {});
        }
    }

    group.run([&active_clients] { return active_clients > 0; });

    auto end = Timestamp::now();
    info("总时间 %g 秒", (end - start).toSecF());
    info("totalBytes = %ld", totalBytes);
    info("速度 %lf MiB/s", totalBytes / (end - start).toSecF() / 1024 / 1024);
    info(" QPS %d", (int)(concurrency * msg_num / (end - start).toSecF()));

    return 0;
}
