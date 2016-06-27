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
    group.run();
    return 0;
}
