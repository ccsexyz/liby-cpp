#include "FileDescriptor.h"
#include <fcntl.h>

void Liby::FileDescriptor::set_noblock(bool flag) {
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
