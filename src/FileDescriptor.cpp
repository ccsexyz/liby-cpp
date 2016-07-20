#include "FileDescriptor.h"
#include <fcntl.h>
#include <sys/stat.h>

using namespace Liby;

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

fdPtr Liby::FileDescriptor::openFile(const std::string &filepath) {
    int fd;
    if (!filepath.empty()) {
        if ((fd = ::open(filepath.data(), O_RDWR)) >= 0) {
            return std::make_shared<FileDescriptor>(fd);
        } else {
            error("open %s error: %s", filepath.data(), ::strerror(errno));
        }
    }
    return nullptr;
}

off_t FileDescriptor::getFileSize() {
    assert(fd() >= 0);

    struct stat st;
    int ret = ::fstat(fd(), &st);
    if (ret < 0) {
        throw_err();
    }
    return st.st_size;
}
