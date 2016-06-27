#include "../src/Liby.h"
#include <iostream>
using namespace Liby;
int main(void) {
    EventLoopGroup group;
    auto discard_server = group.creatTcpServer("localhost", "9376");
    discard_server->onRead([](Connection &conn) {
        info("discard: %s", conn.read().retriveveAllAsString().data());
    });
    group.run();
    return 0;
}
