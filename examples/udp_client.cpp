#include "../src/Channel.h"
#include "../src/FileDescriptor.h"
#include "../src/Liby.h"
#include "../src/Poller.h"

using namespace std;
using namespace Liby;

int main(int argc, char **argv) {
    if (argc != 3) {
        cerr << "usage: ./udp_client host service" << endl;
        return 1;
    }

    try {
        EventLoopGroup group;
        UdpConnection *p_uconn = nullptr;
        auto udp_client = group.creatUdpClient(argv[1], argv[2]);
        udp_client->onConnect([&](UdpConnection &uconn) {
            p_uconn = &uconn;
            //            uconn.send(" ");
        });
        udp_client->onRead([&](UdpConnection &uconn) {
            cout << uconn.read().retriveveAllAsString() << endl;
        });
        auto input_chan =
            std::make_shared<Channel>(Poller::curr_thread_poller(), 0);
        auto input_conn = std::make_shared<Connection>(
            std::make_shared<Socket>(std::make_shared<FileDescriptor>(0)));
        input_conn->setChannel(input_chan);
        input_conn->init();
        input_conn->init1();
        input_conn->onRead([&](Connection &in) {
            if (p_uconn == nullptr)
                return;
            auto str = in.read().retriveveAllAsString();
            //            cout << "input " << str << endl;
            p_uconn->send(str, [str] {
                //                cout << "try to send " << str << endl;
            });
        });
        group.run();
    } catch (const char *err) { cerr << err << endl; }
}
