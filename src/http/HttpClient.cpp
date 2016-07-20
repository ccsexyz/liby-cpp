#include "HttpClient.h"
#include "../Connection.h"
#include "../TcpClient.h"

using namespace Liby;
using namespace Liby::http;

HttpClient::HttpClient(EventLoopGroup &group, const std::string &host) {
    host_ = host;
    http_client_ = group.creatTcpClient(host, "80");
    http_client_->onConnect([this](ConnPtr conn) { handleConnection(conn); });
}

void Liby::http::HttpClient::get(const Request &request,
                                 const ReplyHandler &handler) {
    requestLists_.emplace_back(std::make_pair(request, handler));
    if (http_conn_)
        sendRequest();
}

void Liby::http::HttpClient::destroy() {
    requestLists_.clear();
    http_client_.reset();
    http_conn_.reset();
}

void HttpClient::handleConnection(ConnPtr conn) {
    http_conn_ = conn;
    http_conn_->onRead([this](Connection &c) { handleReadEvent(c); });
    http_conn_->onErro([this](Connection &c) { handleErroEvent(c); });
    if (!requestLists_.empty())
        sendRequest();
}

void HttpClient::handleReadEvent(Connection &conn) {
    while (1) {
        Buffer &readBuf = conn.read();
        if (readBuf.empty())
            return;

        int savedBytes = parser_.bytes_;
        parser_.parse(readBuf.data(), readBuf.data() + readBuf.size());
        readBuf.retrieve(parser_.bytes_ - savedBytes);

        if (parser_.isError()) {
            handleErroEvent(conn);
            return;
        }

        if (parser_.isGood()) {
            auto &handler = sendReqeustLists_.front().second;
            if (handler) {
                handler(parser_);
            }
            parser_.clear();
            sendReqeustLists_.pop_front();
        }
    }
}

void HttpClient::handleErroEvent(Connection &) { destroy(); }

void HttpClient::sendRequest() {
    auto &request = requestLists_.front().first;
    Buffer buffer(1024);
    buffer.append(request.toString());
    buffer.append(request.body_);
    http_conn_->send(buffer, [this] {
        http_conn_->enableRead();
        if (requestLists_.empty())
            return;
        sendRequest();
    });
    sendReqeustLists_.push_back(requestLists_.front());
    requestLists_.pop_front();
}
