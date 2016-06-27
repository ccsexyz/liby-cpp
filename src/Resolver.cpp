#include "Resolver.h"
#include "BlockingQueue.h"
#include "Endpoint.h"
#include "EventLoop.h"
#include <atomic>
#include <netdb.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>
#include <tuple>

using namespace Liby;
using std::thread;
using std::tuple;
using std::string;

class ResolverThread final : clean_ {
public:
    ResolverThread() : keep_running(true) {}

    void func();

    void resolve(const std::string &name, const std::string &port,
                 EventLoop *loop, ResolverHandler handler);

private:
    std::atomic_bool keep_running;
    BlockingQueue<tuple<string, string, EventLoop *, ResolverHandler>>
        requests_;
};

void ResolverThread::func() {
    while (keep_running) {
        auto request = requests_.pop_wait();
        auto name = std::get<0>(request);
        auto port = std::get<1>(request);
        auto loop = std::get<2>(request);
        auto handler = std::get<3>(request);

        if (!handler) {
            continue;
        }

        std::list<Endpoint> eps;

        try {
            eps = Resolver::resolve(name, port);
        } catch (const char *err) { debug("resolver error: %s", err); }

        loop->runEventHandler([eps, handler] { handler(eps); });
    }
}

void ResolverThread::resolve(const std::string &name, const std::string &port,
                             EventLoop *loop, ResolverHandler handler) {
    verbose("try to resolve %s: %s", name.data(), port.data());
    requests_.push_notify(std::make_tuple(name, port, loop, handler));
}

class ResolverImp final : clean_ {
public:
    ResolverImp(ResolverThread &rt) : t_(&ResolverThread::func, &rt) {}

    ~ResolverImp() {
        if (t_.joinable())
            t_.join();
    }

private:
    thread t_;
};

void Resolver::async_resolve(const std::string &host,
                             const std::string &service,
                             const ResolverHandler &handler) {
    static ResolverThread rt;
    static ResolverImp ri(rt);

    EventLoop *loop = EventLoop::curr_thread_loop();
    rt.resolve(host, service, loop, handler);
}

std::list<Endpoint> Resolver::resolve(const std::string &host,
                                      const std::string &service) {
    struct addrinfo *ailist;
    std::list<Endpoint> ret;

    int err = ::getaddrinfo(host.data(), service.data(), nullptr, &ailist);
    if (err != 0) {
        ; // throw err string
    } else {
        DeferCaller caller([ailist] { ::freeaddrinfo(ailist); });

        struct addrinfo *aip = ailist;
        for (; aip != NULL; aip = aip->ai_next) {
            ret.push_back(Endpoint(aip));
        }
    }
    return ret;
}
