#ifndef _LIBY_CPP_ENDPOINT_H_
#define _LIBY_CPP_ENDPOINT_H_

#include <stdint.h>
#include <string>

// 简单包装 struct sockaddr_in
// 只支持 AF_INET

struct sockaddr_in;
struct addrinfo;

namespace Liby {
struct Endpoint {
    Endpoint(uint32_t addr, uint16_t port) : port_(port), addr_(addr) {
    }
    Endpoint(const std::string &addr, const std::string &port);
    Endpoint(const std::string &addr, uint16_t port);
    Endpoint(struct sockaddr_in *sockaddr);
    Endpoint(struct addrinfo *info);

    uint16_t port_;
    uint32_t addr_;

    friend bool operator<(const Endpoint &ep1, const Endpoint &ep2) {
        if (ep1.addr_ < ep2.addr_)
            return true;
        if (ep1.addr_ == ep2.addr_ && ep1.port_ < ep2.port_)
            return true;
        return false;
    }
};
}

#endif
