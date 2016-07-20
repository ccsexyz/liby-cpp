#ifdef __linux__
#include <sys/eventfd.h>
#endif
#include "Channel.h"
#include "EventQueue.h"
#include "FileDescriptor.h"
#include "Poller.h"
#include <string.h>
#include <unistd.h>

using namespace Liby;

EventQueue::EventQueue(Poller *poller) : poller_(poller) {
    fatalif(poller_ == nullptr, "Poller cannot be nullptr");
#ifdef __linux__
    eventfd_ = ::eventfd(1000, EFD_CLOEXEC | EFD_NONBLOCK);
    fatalif(eventfd_ < 0, "eventfd create: %s", ::strerror(errno));
    eventfp_ = std::make_unique<FileDescriptor>(eventfd_);
    eventfp_->set_noblock();
#elif defined(__APPLE__)
    int fds[2];
    if (::pipe(fds) < 0)
        fatal("pipe: %s", ::strerror(errno));
    eventfd_ = fds[0];
    event2fd_ = fds[1];
    eventfp_ = std::make_unique<FileDescriptor>(eventfd_);
    event2fp_ = std::make_unique<FileDescriptor>(event2fd_);
    eventfp_->set_noblock();
    event2fp_->set_noblock();
#endif
    eventChanelPtr_ = std::make_unique<Channel>(poller_, eventfd_);
}

EventQueue::~EventQueue() {
    //　使eventChanelPtr_析构时不去调用Poller基类的虚函数
    if (eventChanelPtr_) {
        //        eventChanelPtr_->setPoller(nullptr);
    }
}

void EventQueue::destroy() {
    eventfd_ = -1;
    eventfp_.reset();
#ifdef __APPLE__
    event2fd_ = -1;
    event2fp_.reset();
#endif
    eventChanelPtr_.reset();
}

void EventQueue::start() {
    eventChanelPtr_->enableRead();
    eventChanelPtr_->onErro([this] { destroy(); });
    eventChanelPtr_->onRead([this] {
        if (!eventfp_)
            return;

        char buf[128];
        int ret = ::read(eventfd_, buf, 128);
        if (ret <= 0)
            return;

        while (!eventHandlers_.empty()) {
            eventHandlers_.pop_front()();
        }
    });
    eventChanelPtr_->addChannel();
}

void EventQueue::wakeup() {
    static int64_t this_is_a_number = 1;
    int targetfd;
#ifdef __linux__
    targetfd = eventfd_;
#elif defined(__APPLE__)
    targetfd = event2fd_;
#endif
    ClearUnuseVariableWarning(
        ::write(targetfd, &this_is_a_number, sizeof(this_is_a_number)));
}

void EventQueue::pushHandler(const BasicHandler &handler) {
    eventHandlers_.push_back(handler);
}

void EventQueue::setPoller(Poller *poller) {
    poller_ = poller;
    eventChanelPtr_->setPoller(poller_);
}
