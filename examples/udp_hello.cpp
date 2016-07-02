#include "../src/Liby.h"

using namespace std;
using namespace Liby;

int main(int argc, char **argv) {
    //    Logger::setLevel(Logger::LogLevel::VERBOSE);
    EventLoopGroup group;
    auto hello_server = group.creatUdpServer("0.0.0.0", "8888");
    hello_server->onAccept(
        [](UdpConnection &uconn) { uconn.send("Hello World\n"); });
    hello_server->onRead(
        [](UdpConnection &uconn) { uconn.send(uconn.read()); });
    group.run();
    return 0;
}
