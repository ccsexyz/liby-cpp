#ifndef LIBY_CPP_HTTPSERVER_H
#define LIBY_CPP_HTTPSERVER_H

#include "../util.h"
#include "HttpDef.h"

namespace Liby {
namespace http {
class HttpServer final : clean_ {
public:
    HttpServer(const std::string &root,
               const std::shared_ptr<TcpServer> &server);

private:
    void handleAcceptEvent(Connection &conn);
    void handleReadEvent(Connection &conn);
    void handleErroEvent(Connection &conn);

private:
    std::string root_;
    std::shared_ptr<TcpServer> http_server_;
};
}
}

#endif // LIBY_CPP_HTTPSERVER_H
