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
