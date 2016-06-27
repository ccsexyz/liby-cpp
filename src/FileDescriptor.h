#ifndef LIBY_CPP_FILEDESCRIPTOR_H
#define LIBY_CPP_FILEDESCRIPTOR_H

#include "util.h"

namespace Liby {
class FileDescriptor {
public:
    FileDescriptor() : fd_(-1) {}

    explicit FileDescriptor(int fd) : fd_(fd) { check_fd(fd); }

    ~FileDescriptor() { ::close(fd_); }

    int fd() const { return fd_; }

    void set_fd(int fd) { check_fd(fd); }

    void shutdown_read() { ::shutdown(fd_, SHUT_RD); }

    void shutdown_write() { ::shutdown(fd_, SHUT_WR); }

private:
    void check_fd(int fd) {
        if (fd < 0) {
            throw "fd < 0";
        }
    }

private:
    int fd_;
};
}

#endif // LIBY_CPP_FILEDESCRIPTOR_H
