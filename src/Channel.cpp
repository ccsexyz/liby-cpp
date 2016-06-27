#include "Channel.h"
#include "Poller.h"

using namespace Liby;
using namespace std;

Channel::~Channel() { removeChannel(); }

void Channel::removeChannel() {
    assert(poller_);
    poller_->removeChanel(this);
}

void Channel::updateChannel() {
    assert(poller_);
    poller_->updateChanel(this, readable_, writable_);
}

void Channel::addChannel() {
    assert(poller_);
    poller_->addChanel(this);
}

void Channel::handleReadEvent() {
    if (readEventCallback_) {
        readEventCallback_();
    } else {
        debug("Error in channel %p: no readEventCallback", this);
    }
}

void Channel::handleWritEvent() {
    if (writEventCallback_) {
        writEventCallback_();
    } else {
        debug("Error in channel %p: no writEventCallback", this);
    }
}

void Channel::handleErroEvent() {
    if (erroEventCallback_) {
        erroEventCallback_();
    } else {
        debug("Error in channel %p: no erroEventCallback", this);
    }
}
