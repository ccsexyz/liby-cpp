#include "../src/Liby.h"

using namespace std;
using namespace Liby;

struct RelayerContext : public BaseContext {
    weak_ptr<Connection> weak_conn;
};

class TcpRelayer : clean_ {
public:
    TcpRelayer(EventLoopGroup &group, const std::string &host,
               const std::string &service, const std::string &listenport)
        : group_(group), host_(host), service_(service) {
        tcpServer_ = group.creatTcpServer("localhost", listenport);
    }
    void start() {
        tcpServer_->onAccept([this](Connection &conn) { onAccept(conn); });
        tcpServer_->onRead([this](Connection &conn) { onRead(conn); });
        tcpServer_->onErro([this](Connection &conn) { onErro(conn); });
        tcpServer_->start();
    }
    void onAccept(Connection &conn) {
        shared_ptr<TcpClient> tcpClient =
            group_.creatTcpClient(host_, service_);
        TimerId id = tcpClient->runAfter(Timestamp(8, 0), [tcpClient] {});

        RelayerContext context;
        context.weak_conn = conn.shared_from_this();
        tcpClient->onConnect([this, context, id](shared_ptr<Connection> c) {
            if (c && !context.weak_conn.expired()) {
                RelayerContext context1;
                context1.weak_conn = c;
                c->context_ = make_shared<RelayerContext>(context);
                auto c1 = context.weak_conn.lock();
                c1->context_ = make_shared<RelayerContext>(context1);

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
        if (conn.context_) {
            RelayerContext *context =
                static_cast<RelayerContext *>(conn.context_.get());
            if (!context->weak_conn.expired()) {
                auto c = context->weak_conn.lock();
                c->send(conn.read());
                return;
            }
        }

        conn.destroy();
    }
    void onErro(Connection &conn) {
        if (conn.context_) {
            RelayerContext *context =
                static_cast<RelayerContext *>(conn.context_.get());
            if (!context->weak_conn.expired()) {
                auto c = context->weak_conn.lock();
                c->destroy();
            }
        }
    }

private:
    string host_;
    string service_;
    EventLoopGroup &group_;

    shared_ptr<TcpServer> tcpServer_;
};

int main(int argc, char **argv) {
    try {
        if (argc != 4) {
            throw "usage: ./tcp_relayer host service listenport";
        }
        EventLoopGroup group;
        TcpRelayer relayer(group, argv[1], argv[2], argv[3]);
        relayer.start();
        group.run();
    } catch (const char *err) { cout << err << endl; }
    return 0;
}
