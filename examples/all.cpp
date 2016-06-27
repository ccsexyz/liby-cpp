#include "../src/Liby.h"
#include <list>
using namespace Liby;
int main(void) {
    EventLoopGroup group;
    std::list<std::weak_ptr<Connection>> conns;

    auto chat_server = group.creatTcpServer("localhost", "9379");
    chat_server->onAccept([&](Connection &conn) {
        auto x = conn.shared_from_this();
        std::weak_ptr<Connection> weak_x = x;
        conns.push_back(weak_x);
    });
    chat_server->onRead([&](Connection &conn) {
        std::string msg = conn.read().retriveveAllAsString();
        for (auto it = conns.begin(); it != conns.end();) {
            auto saved_it = it++;
            if (saved_it->expired()) {
                conns.erase(saved_it);
            } else {
                auto x = saved_it->lock();
                if (x.get() != &conn)
                    x->send(msg);
            }
        }
    });

    auto daytime_server = group.creatTcpServer("localhost", "9378");
    daytime_server->onAccept([](Connection &conn) {
        auto x = conn.shared_from_this();
        info("send %s", Timestamp::now().toString().data());
        conn.send(Timestamp::now().toString().data());
        conn.send("\n", [x] { x->destroy(); });
    });

    auto discard_server = group.creatTcpServer("localhost", "9376");
    discard_server->onRead([](Connection &conn) {
        info("discard: %s", conn.read().retriveveAllAsString().data());
    });

    auto echo_server = group.creatTcpServer("localhost", "9377");
    echo_server->onRead([](Connection &conn) { conn.send(conn.read()); });

    group.run();
    return 0;
}
