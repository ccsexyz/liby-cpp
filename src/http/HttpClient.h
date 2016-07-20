#ifndef LIBY_CPP_HTTPCLIENT_H
#define LIBY_CPP_HTTPCLIENT_H

#include "../EventLoopGroup.h"
#include "../util.h"
#include "HttpDef.h"

namespace Liby {
namespace http {
using ReplyHandler = std::function<void(Reply &reply)>;
class HttpClient final : clean_ {
public:
    HttpClient(EventLoopGroup &group, const std::string &host);

    void get(const Request &request, const ReplyHandler &handler);

    void destroy();

private:
    void handleConnection(ConnPtr conn);
    void handleReadEvent(Connection &conn);
    void handleErroEvent(Connection &conn);
    void sendRequest();

private:
    std::string host_;
    ReplyParser parser_;
    std::list<std::pair<Request, ReplyHandler>> requestLists_;
    std::list<std::pair<Request, ReplyHandler>> sendReqeustLists_;
    std::shared_ptr<TcpClient> http_client_;
    std::shared_ptr<Connection> http_conn_;
};
}
}

#endif // LIBY_CPP_HTTPCLIENT_H
