#include "Socket.h"
#include "FileDescriptor.h"
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>

using namespace Liby;

SockPtr Socket::accept() {
    assert(fd() >= 0);
    struct sockaddr address;
    socklen_t address_len;
    int clfd = ::accept(fd(), &address, &address_len);
    if (clfd < 0) {
        if (errno == EAGAIN) {
            return nullptr;
        } else if (errno == EINTR) {
            return accept();
        } else {
            int i = errno;
            throw ::strerror(errno);
        }
    } else {
        return std::make_shared<Socket>(
            std::make_shared<FileDescriptor>(clfd),
            Endpoint((struct sockaddr_in *)(&address)));
    }
}

Socket::Socket(bool isUdp_) : isUdp_(isUdp_) {}

Socket::Socket(const fdPtr &fp, bool isUdp_)
    : isUdp_(isUdp_), fd_(fp->fd()), fp_(fp) {}

Socket::Socket(const Endpoint &ep, bool isUdp_) : isUdp_(isUdp_), ep_(ep) {}

Socket::Socket(const fdPtr &fp, const Endpoint &ep, bool isUdp)
    : isUdp_(isUdp), fd_(fp->fd()), fp_(fp), ep_(ep) {}

void Socket::connect() {
    if (!fp_) {
        fd_ = ::socket(AF_INET, isUdp_ ? SOCK_DGRAM : SOCK_STREAM, 0);
        if (fd_ < 0) {
            throw ::strerror(errno);
        }
        fp_ = std::make_shared<FileDescriptor>(fd_);
    }

    setNoblock();

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    //    address.sin_len = sizeof(address);
    struct in_addr in_addr1;
    in_addr1.s_addr = ep_.addr_;
    address.sin_addr = in_addr1;
    address.sin_port = ep_.port_;

    int ret = ::connect(fd_, (struct sockaddr *)&address, sizeof(address));
    if (ret < 0 && errno != EINPROGRESS) {
        throw ::strerror(errno);
    }
}

void Socket::listen() {
    if (!fp_) {
        fd_ = ::socket(AF_INET, isUdp_ ? SOCK_DGRAM : SOCK_STREAM, 0);
        if (fd_ < 0) {
            throw ::strerror(errno);
        }

        fp_ = std::make_shared<FileDescriptor>(fd_);
    }

    setNoblock();

    int reuse = 1;
    if (::setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) < 0) {
        throw_err();
    }

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    //    address.sin_len = sizeof(address);
    struct in_addr in_addr1;
    in_addr1.s_addr = ep_.addr_;
    address.sin_addr = in_addr1;
    address.sin_port = ep_.port_;

    if (::bind(fd_, (struct sockaddr *)&address, sizeof(address)) < 0) {
        throw_err();
    }
    if (isUdp_ == false && ::listen(fd_, 20) < 0) {
        throw_err();
    }
}

void Socket::setNoblock(bool flag) noexcept {
    int opt;
    opt = ::fcntl(fd_, F_GETFL);
    if (opt < 0) {
        throw_err();
    }
    opt = flag ? (opt | O_NONBLOCK) : (opt & ~O_NONBLOCK);
    if (::fcntl(fd_, F_SETFL, opt) < 0) {
        throw_err();
    }
}

void Socket::setNonagle(bool flag) noexcept {
    assert(!isUdp_);
    int nodelay = flag ? 1 : 0;
    if (::setsockopt(fd_, IPPROTO_TCP, TCP_NODELAY, &nodelay, sizeof(int)) <
        0) {
        throw_err();
    }
}
