#include "../src/Liby.h"
#include <arpa/inet.h>
using namespace Liby;
using namespace std;

struct ProxyContext : public BaseContext {
    ProxyContext(std::shared_ptr<Connection> conn) : weak_conn(conn) {}
    weak_ptr<Connection> weak_conn;
};

class TcpProxy : clean_ {
public:
    TcpProxy(EventLoopGroup &group, const std::string &listenport)
        : group_(group) {
        proxyServer_ = group.creatTcpServer("localhost", listenport);
    }
    void start() {
        proxyServer_->onAccept([this](Connection &conn) { onAccept(conn); });
        proxyServer_->onRead(
            [this](Connection &conn) { handleConnectRequest(conn); });
        proxyServer_->onErro([this](Connection &conn) { onErro(conn); });
    }
    void onAccept(Connection &conn) { ; }
    void handleConnectRequest(Connection &conn) {
        Buffer &readBuf = conn.read();
        if (readBuf.size() < 9) {
            return;
        }

        char *base = readBuf.data();

        char ver = base[0];
        char command = base[1];

        if (ver != 4 || command != 1) {
            error("bad version %d or command %d", ver, command);
            conn.destroy();
            return;
        }

        char address[50];
        if (::inet_ntop(AF_INET, base + 4, address, 50) == nullptr) {
            conn.destroy();
            return;
        }

        string host = address;
        string service = to_string(ntohs(*(uint16_t *)(base + 2)));

        info("host: %s service: %s", host.data(), service.data());
        readBuf.retrieve();

        shared_ptr<TcpClient> tcpClient = group_.creatTcpClient(host, service);
        TimerId id = tcpClient->runAfter(Timestamp(8, 0), [tcpClient] {});

        ProxyContext context = conn.shared_from_this();
        tcpClient->onConnect([this, context, id](shared_ptr<Connection> c) {
            if (c && !context.weak_conn.expired()) {
                ProxyContext context1 = c;
                c->context_ = make_shared<ProxyContext>(context);
                auto c1 = context.weak_conn.lock();
                c1->context_ = make_shared<ProxyContext>(context1);

                c->onRead(
                    [this](Connection &connection) { onRead(connection); });
                c->onErro(
                    [this](Connection &connection) { onErro(connection); });
                c1->onRead(
                    [this](Connection &connection) { onRead(connection); });
                c1->onErro(
                    [this](Connection &connection) { onErro(connection); });
                c->enableRead();
                c1->enableRead();

                char response[8];
                response[0] = 0x0;
                response[1] = 0x5A;
                c1->send(response, 8);

                return;
            }

            if (c) {
                c->destroy();
            }

            if (!context.weak_conn.expired()) {
                context.weak_conn.lock()->destroy();
            }
        });
    }
    void onRead(Connection &conn) {
        ProxyContext *proxyContext =
            static_cast<ProxyContext *>(conn.context_.get());
        if (proxyContext == nullptr || proxyContext->weak_conn.expired()) {
            conn.destroy();
            return;
        }

        auto c = proxyContext->weak_conn.lock();
        c->send(conn.read());
    }
    void onErro(Connection &conn) {
        ProxyContext *proxyContext =
            static_cast<ProxyContext *>(conn.context_.get());
        if (proxyContext == nullptr || proxyContext->weak_conn.expired()) {
            return;
        }

        auto c = proxyContext->weak_conn.lock();
        c->destroy();
    }

private:
    EventLoopGroup &group_;
    shared_ptr<TcpServer> proxyServer_;
};

int main(int argc, char **argv) {
    try {
        if (argc != 2) {
            throw "usage: ./tcp_proxy listenport";
        }
        EventLoopGroup group;
        TcpProxy tcpProxy(group, argv[1]);
        tcpProxy.start();
        group.run();
    } catch (const char *err) { std::cout << err << std::endl; }
    return 0;
}
