#include "Endpoint.h"
#include <arpa/inet.h>
#include <assert.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>

using namespace Liby;

Endpoint::Endpoint(struct sockaddr_in *sockaddr) {
    assert(sockaddr);
    port_ = sockaddr->sin_port;
    addr_ = sockaddr->sin_addr.s_addr;
}

Endpoint::Endpoint(struct addrinfo *info) {
    assert(info && info->ai_addr);
    struct sockaddr_in *sockaddr = (struct sockaddr_in *)info->ai_addr;
    port_ = sockaddr->sin_port;
    addr_ = sockaddr->sin_addr.s_addr;
}

Endpoint::Endpoint(const std::string &addr, uint16_t port) {
    int ret = ::inet_pton(AF_INET, addr.data(), &addr_);

    if (ret == 0) {
        throw "无效协议";
    } else if (ret < -1) {
        throw "遇到错误";
    } else if (ret != 1) {
        throw "未知错误";
    }

    port_ = htons(port);
}

Endpoint::Endpoint(const std::string &addr, const std::string &port)
    : Endpoint(addr, static_cast<uint16_t>(std::stoi(port))) {}
