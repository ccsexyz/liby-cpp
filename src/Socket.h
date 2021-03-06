#ifndef LIBY_CPP_SOCKET_H
#define LIBY_CPP_SOCKET_H

#include "Endpoint.h"
#include "util.h"
#ifdef __linux__
#include <sys/sendfile.h>
#endif
#include <sys/uio.h>

namespace Liby {
class Socket final : clean_ {
public:
    Socket(bool isUdp_ = false);
    Socket(const fdPtr &fp, bool isUdp_ = false);
    Socket(const Endpoint &ep, bool isUdp_ = false);
    Socket(const fdPtr &fp, const Endpoint &ep, bool isUdp = false);

    void connect();
    void listen();

    SockPtr accept();

    int fd() const { return fd_; }

    Socket &setFp(const fdPtr &fp);

    Socket &setEndpoint(const Endpoint &ep) {
        ep_ = ep;
        return *this;
    }

    int read(void *buf, size_t nbytes) { return ::read(fd_, buf, nbytes); }
    int readv(struct iovec *iov, int iovcnt) {
        return ::readv(fd_, iov, iovcnt);
    }
    int recvfrom(void *buf, size_t nbytes, int flags, struct sockaddr *address,
                 socklen_t *address_len) {
        return ::recvfrom(fd_, buf, nbytes, flags, address, address_len);
    }
    int write(void *buf, size_t nbytes) { return ::write(fd_, buf, nbytes); }
    int writev(struct iovec *iov, int iovcnt) {
        return ::writev(fd_, iov, iovcnt);
    }
    int sendto(void *buf, size_t nbytes, int flags, struct sockaddr *dest_addr,
               socklen_t dest_len) {
        return ::sendto(fd_, buf, nbytes, flags, dest_addr, dest_len);
    }
    int sendfile(int in_fd, off_t *offset, size_t count) {
#ifdef __linux__
        return ::sendfile(fd_, in_fd, offset, count);
#elif defined(__APPLE__)
        off_t len = static_cast<off_t>(count);
        return ::sendfile(in_fd, fd_, *offset, &len, nullptr, 0);
#endif
    }

    void setNoblock(bool flag = true);
    void setNonagle(bool flag = true);

private:
    void createSocket();

private:
    bool isUdp_ = false;
    int fd_ = -1;
    fdPtr fp_;
    Endpoint ep_ = Endpoint(0, 0);
};
}

#endif // LIBY_CPP_SOCKET_H
