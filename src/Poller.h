#ifndef LIBY_CPP_POLLER_H
#define LIBY_CPP_POLLER_H

#include "util.h"

namespace Liby {
class Channel;

class Poller : clean_ {
public:
    Poller();

    virtual ~Poller() {}

    void bind_to_thread();

    static Poller *curr_thread_poller();

    Channel *getChannel(int fd) { return channels_[fd]; }

    void setChannel(int fd, Channel *ch) { channels_[fd] = ch; }

    // 在事件处理结束时调用
    void nextTick(const BasicHandler &handler);
    // 在Poller的下一次循环调用
    void nextLoop(const BasicHandler &handler);
    // 在Poller的这一次循环结束时调用
    void afterLoop(const BasicHandler &handler);

public:
    virtual void addChanel(Channel *) {}

    virtual void updateChanel(Channel *, bool, bool) {}

    virtual void removeChanel(Channel *) {}

    virtual void loop_once(Timestamp * = nullptr) {}

protected:
    void runNextTickHandlers();
    void runNextLoopHandlers();
    void runAfterLoopHandlers();

private:
    std::vector<Channel *> channels_;
    std::list<BasicHandler> nextTickHandlers_;
    std::list<BasicHandler> nextLoopHandlers_;
    std::list<BasicHandler> afterLoopHandlers_;
};
}

#endif // LIBY_CPP_POLLER_H
