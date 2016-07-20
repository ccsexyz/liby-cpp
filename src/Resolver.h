#ifndef LIBY_CPP_RESOLVER_H
#define LIBY_CPP_RESOLVER_H

#include "util.h"
#include <list>

namespace Liby {
struct Endpoint;
using ResolverHandler = std::function<void(std::list<Endpoint> eps)>;
class Resolver final : clean_ {
public:
    static std::list<Endpoint> resolve(const std::string &host,
                                       const std::string &service);

    static void async_resolve(const std::string &host,
                              const std::string &service,
                              const ResolverHandler &handler);
};
}

#endif // LIBY_CPP_RESOLVER_H
