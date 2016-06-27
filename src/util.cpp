#include "util.h"
#include "Poller.h"
#include <fcntl.h>
#include <list>
#include <mutex>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <unordered_map>

using namespace Liby;

static std::unordered_map<int, std::function<void()>> functors;
static std::unordered_map<int, void (*)(int)> stored_funcs;

static void signal_handler(int signo) { functors[signo](); }

void Signal::signal(int signo, const std::function<void()> &handler) {
    functors[signo] = handler;
    stored_funcs[signo] = ::signal(signo, signal_handler);
}

void Signal::reset(int signo) {
    auto k = functors.find(signo);
    if (k != functors.end()) {
        functors.erase(k);
    }
    auto k1 = stored_funcs.find(signo);
    if (k1 != stored_funcs.end()) {
        ::signal(signo, stored_funcs[signo]);
        stored_funcs.erase(k1);
    }
}

static std::mutex ExitCallerMutex;
static std::list<std::function<void()>> ExitCallerFunctors;

ExitCaller::ExitCaller() { ::atexit(ExitCaller::callOnExit); }

ExitCaller &ExitCaller::getCaller() {
    static ExitCaller caller;
    return caller;
}

void ExitCaller::call(const std::function<void()> &functor) {
    ExitCaller::getCaller().callImp(functor);
}

void ExitCaller::callImp(const std::function<void()> &functor) {
    std::lock_guard<std::mutex> G_(ExitCallerMutex);
    ExitCallerFunctors.push_back(functor);
}

void ExitCaller::callOnExit() {
    std::lock_guard<std::mutex> G_(ExitCallerMutex);
    std::for_each(
        ExitCallerFunctors.begin(), ExitCallerFunctors.end(),
        [](decltype(*ExitCallerFunctors.begin()) &functor) { functor(); });
}

__thread time_t savedSec;
__thread char savedSecString[128]; // 似乎clang++并不支持thread_local的对象

std::string Timestamp::toString() const {
    char usecString[48];
    sprintf(usecString, " %06ld", tv_.tv_usec);
    if (tv_.tv_sec != savedSec) {
        struct tm result;
        ::localtime_r(&(tv_.tv_sec), &result);
        savedSec = tv_.tv_sec;
        sprintf(savedSecString, "%4d.%02d.%02d - %02d:%02d:%02d -",
                result.tm_year + 1900, result.tm_mon + 1, result.tm_mday,
                result.tm_hour, result.tm_min, result.tm_sec);
    }
    return std::string(savedSecString) + usecString;
}
