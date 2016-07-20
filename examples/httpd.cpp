#include "../src/Liby.h"
#include "../src/http/HttpServer.h"

using namespace Liby;
using namespace Liby::http;

int main(void) {
    EventLoopGroup group;
    HttpServer httpd("/home/ccsexyz",
                     group.creatTcpServer("localhost", "8080"));
    group.run();
    return 0;
}
