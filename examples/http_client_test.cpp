#include "../src/Liby.h"
#include "../src/http/HttpClient.h"

using namespace Liby;
using namespace Liby::http;

int main(void) {
    EventLoopGroup group;
    HttpClient client(group, "www.dotamax.com");
    Request request;
    request.method_ = GET;
    request.uri_ = "/";
    auto handler = [](Reply &reply) {
        std::cout << reply.toString() << std::endl;
    };
    client.get(request, handler);
    group.run();
}
