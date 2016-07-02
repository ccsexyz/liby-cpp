#ifndef _LIBY_CPP_UDPCONNECTION_H_
#define _LIBY_CPP_UDPCONNECTION_H_

#include "Buffer.h"
#include "Endpoint.h"
#include "util.h"

namespace Liby {
class UdpSocket;
class UdpConnection : clean_,
                      public std::enable_shared_from_this<UdpConnection> {
public:
    UdpConnection(UdpSocket *socket, const Endpoint &ep)
        : socket_(socket), readBuf_(1024) {
        assert(socket_);
        peer_.sin_addr.s_addr = ep.addr_;
        peer_.sin_port = ep.port_;
        peer_.sin_family = AF_INET;
    }

    Buffer &getReadBuf() { return readBuf_; }

    Buffer &read() { return readBuf_; }

    UdpSocket *getSocket() {
        assert(socket_);
        return socket_;
    }

    template <typename T>
    void send_object(T &&t, const BasicHandler &handler = nullptr) {
        send((const char *)(&t), sizeof(t), handler);
    }
    void send(const char *base, off_t len,
              const BasicHandler &handler = nullptr);
    void send(Buffer &buf, const BasicHandler &handler = nullptr);
    void send(Buffer &&buf, const BasicHandler &handler = nullptr);
    void send(const char *buf, const BasicHandler &handler = nullptr);
    void send(const std::string &str, const BasicHandler &handler = nullptr);

    void destroy();

private:
    UdpSocket *socket_ = nullptr;
    struct sockaddr_in peer_;
    Buffer readBuf_;
};
}

#endif
