#ifndef LIBY_CPP_EVENTQUEUE_H
#define LIBY_CPP_EVENTQUEUE_H

#include "BlockingQueue.h"
#include "util.h"

namespace Liby {
class EventQueue : clean_ {
public:
    EventQueue(Poller *poller);

    ~EventQueue();

    void setPoller(Poller *poller);

    void start();

    void destroy();

    void wakeup();

    void pushHandler(const BasicHandler &handler);

private:
    Poller *poller_;
    int eventfd_ = -1;
    std::unique_ptr<FileDescriptor> eventfp_;
    std::unique_ptr<Channel> eventChanelPtr_;
    BlockingQueue<BasicHandler> eventHandlers_;
};
}

#endif // LIBY_CPP_EVENTQUEUE_H
