#include "HttpServer.h"
#include "../Connection.h"
#include "../FileDescriptor.h"
#include "../TcpServer.h"

using namespace Liby;
using namespace Liby::http;

struct HttpContext final : public BaseContext, clean_ {
    bool keep_alive_ = true;
    RequestParser requestParser;
};

HttpServer::HttpServer(const std::string &root,
                       const std::shared_ptr<TcpServer> &server)
    : root_(root), http_server_(server) {
    http_server_->onRead([this](Connection &conn) { handleReadEvent(conn); });
    http_server_->onAccept(
        [this](Connection &conn) { handleAcceptEvent(conn); });
    http_server_->onErro([this](Connection &conn) { handleErroEvent(conn); });
    http_server_->start();
}

void HttpServer::handleAcceptEvent(Connection &conn) {
    conn.context_ = std::make_shared<HttpContext>();
}

void HttpServer::handleReadEvent(Connection &conn) {
    assert(conn.context_);

    HttpContext *context = static_cast<HttpContext *>(conn.context_.get());

    while (1) {
        Buffer &readBuf = conn.read();
        if (readBuf.empty())
            return;

        int savedBytes = context->requestParser.bytes_;
        context->requestParser.parse(readBuf.data(),
                                     readBuf.data() + readBuf.size());
        readBuf.retrieve(context->requestParser.bytes_ - savedBytes);

        if (context->requestParser.isError()) {
            std::weak_ptr<Connection> weak_conn(conn.shared_from_this());
            conn.send(Reply::Error(404), [weak_conn] {
                if (!weak_conn.expired()) {
                    auto conn = weak_conn.lock();
                    conn->destroy();
                }
            });
            break;
        } else if (context->requestParser.isGood()) {
            auto fp =
                FileDescriptor::openFile(root_ + context->requestParser.uri_);
            if (!fp) {
                error("fp is null");
                return;
            }

            long filesize;
            try {
                filesize = fp->getFileSize();
            } catch (const char *err) {
                error("%s", err);
                // send error response
                break;
            }

            Reply rep;
            rep.status_ = HTTP_OK;
            rep.headers_["Content-Length"] = std::to_string(filesize);
            rep.headers_["Content-Type"] =
                getMimeType(context->requestParser.uri_);

            std::weak_ptr<Connection> weak_conn(conn.shared_from_this());
            conn.send(rep.toString(), [fp, weak_conn, filesize] {
                if (!weak_conn.expired()) {
                    auto conn = weak_conn.lock();
                    conn->sendFile(fp, 0, filesize, [conn] {
                        HttpContext *context =
                            static_cast<HttpContext *>(conn->context_.get());
                        if (!context->keep_alive_)
                            conn->destroyLater();
                    });
                }
            });
            if (context->requestParser.headers_["Connection"] != "keep-alive") {
                context->keep_alive_ = false;
                break;
            }
            context->requestParser.clear();
        } else {
            break;
        }
    }
}

void HttpServer::handleErroEvent(Connection &conn) { conn.destroy(); }
