#include "../src/Liby.h"
using namespace Liby;
int main(void) {
    EventLoopGroup group;
    auto daytime_server = group.creatTcpServer("localhost", "9378");
    daytime_server->onAccept([](Connection &conn) {
        auto x = conn.shared_from_this();
        info("send %s", Timestamp::now().toString().data());
        conn.send(Timestamp::now().toString().data());
        conn.send("\n", [x] { x->destroy(); });
    });
    group.run();
    return 0;
}
