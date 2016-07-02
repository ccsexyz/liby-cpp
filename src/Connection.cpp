#include "Connection.h"
#include "Channel.h"
#include "EventLoop.h"
#include "FileDescriptor.h"
#include "Poller.h"
#include "Socket.h"

using namespace Liby;

Connection::Connection(EventLoop *loop, const SockPtr &socket)
    : poller_(loop->getPoller()), loop_(loop), socket_(socket), readBuf_(1024) {
    assert(socket);
}

Connection::Connection(const SockPtr &socket)
    : Connection(EventLoop::curr_thread_loop(), socket) {
}

void Connection::init() {
    self_ = shared_from_this();
    chan_->onRead([this] { handleReadEvent(); });
    chan_->onWrit([this] { handleWritEvent(); });
    chan_->onErro([this] { handleErroEvent(); });
}

void Connection::init1() {
    chan_->enableRead();
    chan_->addChannel();
}

void Connection::destroy() {
    if (destroy_)
        return;
    else
        destroy_ = true;
    verbose("try to destroy connection %p", this);
    chan_.reset();
    socket_.reset();
    writTasks_.clear();
    erroEventCallback_ = nullptr;
    readEventCallback_ = nullptr;
    self_.reset();
}

void Connection::setErro() {
}

void Connection::sendFile(const fdPtr &fp, off_t off, off_t len) {
}

void Connection::runEventHandler(const BasicHandler &handler) {
    assert(loop_ && poller_);

    if (!handler) {
        return;
    }

    loop_->runEventHandler(handler);
}

void Connection::send(const char *base, off_t len,
                      const BasicHandler &handler) {
    io_task task;
    task.buffer_ =
        std::make_shared<Buffer>(static_cast<const char *>(base), len);
    if (handler) {
        task.handler_ = std::make_unique<BasicHandler>(handler);
    }
    writTasks_.emplace_back(std::move(task));
    enableWrit();
}

void Connection::send(Buffer &buf, const BasicHandler &handler) {
    io_task task;
    task.buffer_ = std::make_shared<Buffer>(buf.capacity());
    task.buffer_->swap(buf);
    if (handler) {
        task.handler_ = std::make_unique<BasicHandler>(handler);
    }
    writTasks_.emplace_back(std::move(task));
    enableWrit();
}

void Connection::send(Buffer &&buf, const BasicHandler &handler) {
    io_task task;
    task.buffer_ = std::make_shared<Buffer>(std::move(buf));
    if (handler) {
        task.handler_ = std::make_unique<BasicHandler>(handler);
    }
    writTasks_.emplace_back(std::move(task));
    enableWrit();
}

void Connection::send(const char *buf, const BasicHandler &handler) {
    assert(buf);
    send(buf, ::strlen(buf), handler);
}

void Connection::send(const std::string &str, const BasicHandler &handler) {
    if (str.empty()) {
        return;
    } else {
        send(str.data(), str.length(), handler);
    }
}

int Connection::sync_read(char *buf, size_t len) {
    return 0;
}

int Connection::sync_send(Buffer &buffer) {
    return 0;
}

__thread char extraBuffer[16384];

void Connection::handleReadEvent() {
    Buffer &buffer = readBuf_;

    struct iovec iov[2];
    iov[0].iov_base = buffer.wdata();
    iov[0].iov_len = buffer.availbleSize();
    iov[1].iov_base = extraBuffer;
    iov[1].iov_len = sizeof(extraBuffer);

    int ret = socket_->readv(iov, 2);

    if (ret > 0) {
        if (static_cast<decltype(iov[0].iov_len)>(ret) > iov[0].iov_len) {
            buffer.append(iov[0].iov_len);
            buffer.append(extraBuffer, ret - iov[0].iov_len);
        } else {
            buffer.append(ret);
        }
    } else {
        if (ret != 0 && errno == EAGAIN) {
            return;
        } else {
            handleErroEvent();
            return;
        }
    }

    if (readEventCallback_) {
        auto x = shared_from_this();
        readEventCallback_(*this);
    }
}

void Connection::handleWritEvent() {
    //    if (writTasks_.empty()) {
    //
    //        return;
    //    }

    auto x = self_;
    DeferCaller([this] {
        if (destroy_)
            return;
        if (writTasks_.empty())
            enableWrit(false);
    });
    //    enableWrit(false);

    //    ssize_t bytes = 0; // 注: sendfile的字节数不计入bytes中
    while (!writTasks_.empty()) {
        auto &first = writTasks_.front();
        if (first.fp_) {
            off_t offset = first.offset_;
            int ret = socket_->sendfile(first.fp_->fd(), &offset, first.len_);
            if (ret <= 0) {
                if (ret != 0 && errno == EAGAIN) {
                    return;
                } else {
                    handleErroEvent();
                    return;
                }
            } else if (ret == first.len_) {
                if (first.handler_) {
                    (*first.handler_)();
                }
                writTasks_.pop_front();
                continue;
            } else {
                first.len_ -= ret;
                first.offset_ += ret;
                return;
            }
        } else {
            std::vector<struct iovec> iov;
            for (auto &x : writTasks_) {
                if (!x.fp_ && x.buffer_) {
                    struct iovec v;
                    v.iov_base = x.buffer_->data();
                    v.iov_len = x.buffer_->size();
                    iov.push_back(v);
                } else {
                    break;
                }
            }
            auto ret = socket_->writev(&iov[0], iov.size());
            if (ret > 0) {
                while (ret > 0) {
                    auto firstBufferSize = writTasks_.front().buffer_->size();
                    if (firstBufferSize <= ret) {
                        ret -= firstBufferSize;
                        if (writTasks_.front().handler_) {
                            (*writTasks_.front().handler_)();
                            if (destroy_)
                                return;
                        }
                        writTasks_.pop_front();
                    } else {
                        writTasks_.front().buffer_->retrieve(ret);
                        return;
                    }
                }
            } else {
                if (ret != 0 && errno == EAGAIN) {
                    return;
                } else {
                    handleErroEvent();
                    return;
                }
            }
        }
    }
}

void Connection::handleErroEvent() {
    auto x = shared_from_this();
    if (erroEventCallback_) {
        erroEventCallback_(*this);
    }
    destroy();
}

Connection &Connection::enableRead(bool flag) {
    chan_->enableRead(flag);
    chan_->updateChannel();
    return *this;
}

Connection &Connection::enableWrit(bool flag) {
    chan_->enableWrit(flag);
    chan_->updateChannel();
    return *this;
}

io_task::io_task(const io_task &that) {
    fp_ = that.fp_;
    offset_ = that.offset_;
    len_ = that.len_;
    buffer_ = that.buffer_;
}
