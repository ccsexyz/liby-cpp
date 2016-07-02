#include "UdpConnection.h"
#include "UdpSocket.h"

using namespace Liby;

void UdpConnection::send(const char *base, off_t len,
                         const BasicHandler &handler) {
    assert(socket_);
    udp_io_task utask;
    utask.buffer_ = std::make_shared<Buffer>(base, len);
    utask.dest_ = &peer_;
    if (handler) {
        utask.handler_ = std::make_unique<BasicHandler>(handler);
    }
    socket_->send(std::move(utask));
}

void UdpConnection::send(Liby::Buffer &buf, const BasicHandler &handler) {
    assert(socket_);
    udp_io_task utask;
    utask.buffer_ = std::make_shared<Buffer>(buf.capacity());
    utask.buffer_->swap(buf);
    utask.dest_ = &peer_;
    if (handler) {
        utask.handler_ = std::make_unique<BasicHandler>(handler);
    }
    socket_->send(std::move(utask));
}

void UdpConnection::send(Liby::Buffer &&buf, const BasicHandler &handler) {
    assert(socket_);
    udp_io_task utask;
    utask.buffer_ = std::make_shared<Buffer>(std::move(buf));
    utask.dest_ = &peer_;
    if (handler) {
        utask.handler_ = std::make_unique<BasicHandler>(handler);
    }
    socket_->send(std::move(utask));
}

void UdpConnection::send(const char *buf, const BasicHandler &handler) {
    assert(socket_);
    udp_io_task utask;
    utask.buffer_ = std::make_shared<Buffer>(buf, ::strlen(buf));
    utask.dest_ = &peer_;
    if (handler) {
        utask.handler_ = std::make_unique<BasicHandler>(handler);
    }
    socket_->send(std::move(utask));
}

void UdpConnection::send(const std::string &str, const BasicHandler &handler) {
    assert(socket_);
    send(str.data(), str.length(), handler);
}

void UdpConnection::destroy() {
    assert(socket_);
    socket_->destroyConn(Endpoint(&peer_));
}
