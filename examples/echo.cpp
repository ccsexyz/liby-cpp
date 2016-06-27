#include "../src/Liby.h"
using namespace Liby;
int main(void) {
    EventLoopGroup group;
    auto echo_server = group.creatTcpServer("localhost", "9377");
    echo_server->onRead([](Connection &conn) { conn.send(conn.read()); });
    group.run();
    return 0;
}
