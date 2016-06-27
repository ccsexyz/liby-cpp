#ifndef LIBY_CPP_UTIL_H
#define LIBY_CPP_UTIL_H

#include "Logger.h"
#include <algorithm>
#include <cassert>
#include <deque>
#include <errno.h>
#include <errno.h>
#include <functional>
#include <iostream>
#include <memory>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <type_traits>
#include <unistd.h>
#include <unordered_set>
#include <utility>
#include <vector>

#if __cplusplus < 201402L
// support make_unique in c++ 11
namespace std {
template <typename T, typename... Args>
inline typename enable_if<!is_array<T>::value, unique_ptr<T>>::type
make_unique(Args &&... args) {
    return unique_ptr<T>(new T(std::forward<Args>(args)...));
}

template <typename T>
inline typename enable_if<is_array<T>::value && extent<T>::value == 0,
                          unique_ptr<T>>::type
make_unique(size_t size) {
    using U = typename remove_extent<T>::type;
    return unique_ptr<T>(new U[size]());
}

template <typename T, typename... Args>
typename enable_if<extent<T>::value != 0, void>::type
make_unique(Args &&...) = delete;
}

#endif

struct epoll_event;

namespace Liby {

class Buffer;
class Poller;
class Socket;
class Channel;
class FileDescriptor;
class Connection;
class EventLoop;

using TimerId = uint64_t;
using BasicHandler = std::function<void()>;
using fdPtr = std::shared_ptr<FileDescriptor>;
using ConnPtr = std::shared_ptr<Connection>;
using SockPtr = std::shared_ptr<Socket>;
using ChanPtr = std::shared_ptr<Channel>;
using ConnCallback = std::function<void(Connection &)>;
using Connector = std::function<void(std::shared_ptr<Connection>)>;
using RobinFunctor = std::function<EventLoop *(int)>;
using BoolFunctor = std::function<bool()>;

class clean_ {
public:
    clean_(const clean_ &) = delete;

    clean_(clean_ &&) = delete;

    clean_ &operator=(const clean_ &) = delete;

    clean_() = default;
};

class DeferCaller final : clean_ {
public:
    DeferCaller(std::function<void()> &&functor)
        : functor_(std::move(functor)) {}

    DeferCaller(const std::function<void()> &functor) : functor_(functor) {}

    ~DeferCaller() {
        if (functor_)
            functor_();
    }

    void cancel() { functor_ = nullptr; }

private:
    std::function<void()> functor_;
};

class ErrnoSaver final : clean_ {
public:
    ErrnoSaver() : savedErrno_(errno) {}

    ~ErrnoSaver() { errno = savedErrno_; }

private:
    int savedErrno_ = 0;
};

class ExitCaller final : clean_ {
public:
    static ExitCaller &getCaller();

    static void call(const std::function<void()> &functor);

private:
    void callImp(const std::function<void()> &functor);

    static void callOnExit();

private:
    ExitCaller();
};

class DeconstructCaller : clean_ {
public:
    DeconstructCaller(std::initializer_list<BasicHandler> &&callerList = {}) {
        if (callerList.size() > 0) {
            handlers_ = new (std::nothrow) std::list<BasicHandler>;
            if (handlers_ == nullptr)
                return;
            for (auto &functor_ : callerList) {
                handlers_->emplace_back(functor_);
            }
        }
    }

    virtual ~DeconstructCaller() {
        if (handlers_ == NULL)
            return;
        for (auto &functor_ : *handlers_) {
            functor_();
        }
    }

private:
    std::list<BasicHandler> *handlers_ = nullptr;
};

// using gettimeofday
class Timestamp final {
public:
    static const uint32_t kMicrosecondsPerSecond = (1000) * (1000);

    Timestamp() : Timestamp(0, 0) {}

    explicit Timestamp(struct timeval *ptv) : tv_(*ptv) {}

    explicit Timestamp(const struct timeval &tv) : tv_(tv) {}

    explicit Timestamp(time_t sec) : Timestamp(sec, 0) {}

    explicit Timestamp(time_t sec, suseconds_t usec) {
        tv_.tv_sec = sec;
        tv_.tv_usec = usec;
        if (static_cast<uint32_t>(usec) > kMicrosecondsPerSecond) {
            tv_.tv_sec += usec / kMicrosecondsPerSecond;
            tv_.tv_usec = usec % kMicrosecondsPerSecond;
        }
    }

    Timestamp(const Timestamp &that) { tv_ = that.tv_; }

    Timestamp(Timestamp &&that) { tv_ = that.tv_; }

    Timestamp &operator=(const Timestamp &that) {
        tv_ = that.tv_;
        return *this;
    }

    void swap(Timestamp &rhs) {
        auto temp(tv_);
        tv_ = rhs.tv_;
        rhs.tv_ = temp;
    }

    std::string toString() const;

    bool invalid() const { return tv_.tv_sec == 0 && tv_.tv_usec == 0; }

    bool valid() const { return !invalid(); }

    time_t sec() const { return tv_.tv_sec; }

    suseconds_t usec() const { return tv_.tv_usec; }

    uint64_t toMillSec() const {
        return tv_.tv_sec * 1000 + tv_.tv_usec / 1000;
    }

    uint64_t toMicroSec() const { return tv_.tv_sec * 1000000UL + tv_.tv_usec; }

    double toSecF() const {
        return static_cast<double>(tv_.tv_usec) / 1000000.0 +
               static_cast<double>(tv_.tv_sec);
    }

    double toMillSecF() const {
        return static_cast<double>(tv_.tv_usec) / 1000.0 +
               static_cast<double>(tv_.tv_sec * 1000);
    }

    // if this > rhs return 1
    // elif == rhs return 0
    // elif < rhs return -1
    int compareTo(const Timestamp &rhs) const {
        if (rhs.tv_.tv_sec > tv_.tv_sec) {
            return -1;
        } else if (rhs.tv_.tv_sec < tv_.tv_sec) {
            return 1;
        } else {
            if (rhs.tv_.tv_usec > tv_.tv_usec) {
                return -1;
            } else if (rhs.tv_.tv_usec < tv_.tv_usec) {
                return 1;
            } else {
                return 0;
            }
        }
    }

    static Timestamp now() {
        Timestamp ret;
        ret.gettimeofday();
        return ret;
    }

    static Timestamp invalidTime() { return Timestamp(); }

private:
    void gettimeofday() { ::gettimeofday(&tv_, NULL); }

private:
    struct timeval tv_;
};

inline bool operator<(const Timestamp &lhs, const Timestamp &rhs) {
    return lhs.compareTo(rhs) == -1;
}

inline bool operator==(const Timestamp &lhs, const Timestamp &rhs) {
    return lhs.compareTo(rhs) == 0;
}

inline bool operator>(const Timestamp &lhs, const Timestamp &rhs) {
    return lhs.compareTo(rhs) == 1;
}

inline Timestamp operator-(const Timestamp &lhs, const Timestamp &rhs) {
    auto sec = lhs.sec() - rhs.sec();
    auto lusec = lhs.usec();
    auto rusec = rhs.usec();

    if (lusec < rusec) {
        sec--;
        lusec += Timestamp::kMicrosecondsPerSecond;
    }
    lusec -= rusec;

    return Timestamp(sec, lusec);
}

inline Timestamp operator+(const Timestamp &lhs, const Timestamp &rhs) {
    return Timestamp(lhs.sec() + rhs.sec(), lhs.usec() + rhs.usec());
}

class Signal final {
public:
    static void signal(int signo, const std::function<void()> &handler);

    static void reset(int signo);
};

template <typename T> void ClearUnuseVariableWarning(T &&) { /*do nothing*/
}

template <typename T> class Trie final : clean_ {
public:
    static const int max_elements = 95;
    using class_type = Trie<T>;
    using value_type = T;

    Trie() {
        nodes_ = new class_type *[max_elements];
        for (int i = 0; i < max_elements; i++) {
            nodes_[i] = nullptr;
        }
    }

    ~Trie() {
        for (int i = 0; i < max_elements; i++) {
            delete nodes_[i];
        }
        delete element_;
        delete[] nodes_;
    }

    T &operator[](const std::string &str) { return operator[](str.data()); }

    T &operator[](const char *str) {
        if (str == nullptr)
            throw;

        class_type *p = this;
        for (; *str != '\0'; ++str) {
            int i = conv(*str);
            if (p->nodes_[i] == nullptr) {
                p->nodes_[i] = new class_type;
            }
            p = p->nodes_[i];
        }

        if (p->element_ == nullptr) {
            p->element_ = new T;
        }
        return *(p->element_);
    }

    bool find(const char *str) {
        if (str == nullptr)
            return false;

        class_type *p = this;
        for (; *str != '\0'; ++str) {
            int i = conv(*str);
            if (p->nodes_[i] == nullptr)
                return false;
            p = p->nodes_[i];
        }

        if (p->element_ == nullptr) {
            return false;
        } else {
            return true;
        }
    }

    bool find(const std::string &str) { return find(str.data()); }

    void insert(const char *str, const T &element) {
        if (str == nullptr)
            throw;

        class_type *p = this;
        for (; *str != '\0'; ++str) {
            int i = conv(*str);
            if (p->nodes_[i] == nullptr) {
                p->nodes_[i] = new class_type;
            }
            p = p->nodes_[i];
        }

        if (p->element_ == nullptr) {
            p->element_ = new T(element);
        } else {
            *(p->element_) = element;
        }
    }

    void insert(const std::string &str, const T &element_) {
        insert(str.data(), element_);
    }

private:
    int conv(int c) {
        if (c < 32 || c > 126)
            throw;
        return c - 32;
    }

private:
    T *element_ = nullptr;
    Trie<T> **nodes_;
};

struct BaseContext {
    virtual ~BaseContext() {
        // for Connection Context
    }
};

static void inline throw_err() { throw ::strerror(errno); }

static inline long get_open_max() noexcept { return ::sysconf(_SC_OPEN_MAX); }
}

#endif // LIBY_CPP_UTIL_H
