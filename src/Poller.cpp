#include "Poller.h"

using namespace Liby;

// per thread one poller
__thread Poller *th_poller = nullptr;

Poller::Poller() : channels_(get_open_max(), nullptr) {}

Poller *Poller::curr_thread_poller() {
    assert(th_poller);
    return th_poller;
}

void Poller::bind_to_thread() {
    if (th_poller) {
        throw "th_poller cannot be bound more than 1 time";
    }
    th_poller = this;
}

void Poller::nextLoop(const BasicHandler &handler) {
    nextLoopHandlers_.push_back(handler);
}

void Poller::afterLoop(const BasicHandler &handler) {
    afterLoopHandlers_.push_back(handler);
}

void Poller::nextTick(const BasicHandler &handler) {
    nextTickHandlers_.push_back(handler);
}

void Poller::runNextLoopHandlers() {
    if(nextLoopHandlers_.empty())
        return;
    for(auto &handler : nextLoopHandlers_)
        handler();
    nextLoopHandlers_.clear();
}

void Poller::runAfterLoopHandlers() {
    if(afterLoopHandlers_.empty())
        return;
    for(auto &handler : afterLoopHandlers_)
        handler();
    afterLoopHandlers_.clear();
}

void Poller::runNextTickHandlers() {
    if(nextTickHandlers_.empty())
        return;
    for(auto &handler : nextTickHandlers_)
        handler();
    nextTickHandlers_.clear();
}
