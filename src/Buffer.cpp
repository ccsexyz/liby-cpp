#include "Buffer.h"
#include <algorithm>
#include <assert.h>
#include <cstring>

using namespace Liby;

Buffer::Buffer(const off_t length)
    : leftIndex_(defaultPrependSize), rightIndex_(defaultPrependSize),
      capacity_(length + defaultPrependSize) {
    buffer_ = new char[capacity_];
}

Buffer::~Buffer() { resetBuffer(); }

Buffer::Buffer(const char *buf, off_t length) {
    buffer_ = new char[length];
    ::memcpy(buffer_, buf, length);
    leftIndex_ = 0;
    rightIndex_ = length;
    capacity_ = length;
}

Buffer::Buffer(const std::vector<char> &buf) : Buffer(&buf[0], buf.size()) {}

Buffer::Buffer(const std::string &buf) : Buffer(buf.data(), buf.size() + 1) {}

Buffer::Buffer(const Buffer &that) { deepCopy(that); }

Buffer::Buffer(Buffer &that) { deepCopy(that); }

Buffer::Buffer(Buffer &&that) { swap(that); }

Buffer &Buffer::operator=(const Buffer &that) {
    deepCopy(that);
    return *this;
}

void Buffer::copy(Buffer &that) {
    if (this == &that)
        return;
    resetBuffer();
    buffer_ = that.buffer_;
    that.buffer_ = nullptr;
    leftIndex_ = that.leftIndex_;
    rightIndex_ = that.rightIndex_;
    capacity_ = that.capacity_;
}

void Buffer::deepCopy(const Buffer &that) {
    if (this == &that)
        return;
    if(that.empty()) {
        retrieve();
        return;
    }
    resetBuffer();
    capacity_ = that.size() + defaultPrependSize;
    buffer_ = new char[capacity_];
    ::memcpy(buffer_ + defaultPrependSize, that.data(), that.size());
    leftIndex_ = defaultPrependSize;
    rightIndex_ = that.size() + defaultPrependSize;
}

void Buffer::setBuffer(char *buf, off_t length) {
    buffer_ = buf;
    leftIndex_ = 0;
    rightIndex_ = length;
    capacity_ = length;
}

void Buffer::swap(Buffer &that) {
    std::swap(that.buffer_, buffer_);
    std::swap(that.leftIndex_, leftIndex_);
    std::swap(that.rightIndex_, rightIndex_);
    std::swap(that.capacity_, capacity_);
}

char *Buffer::data() {
    assert(buffer_ && rightIndex_ >= leftIndex_);
    return buffer_ + leftIndex_;
}

const char *Buffer::data() const {
    assert(buffer_ && rightIndex_ >= leftIndex_);
    return buffer_ + leftIndex_;
}

off_t Buffer::size() const {
    assert(rightIndex_ >= leftIndex_);
    return rightIndex_ - leftIndex_;
}

off_t Buffer::capacity() const {
    assert(capacity_ > 0 && rightIndex_ >= leftIndex_);
    return capacity_;
}

void Buffer::append(off_t n) {
    assert(n >= 0 && rightIndex_ + n <= capacity_);
    rightIndex_ += n;
}

void Buffer::append(const Buffer &that) {
    if(!that.empty())
        append(that.data(), that.size());
}

void Buffer::append(const char *buf, off_t len) {
    assert(buf && len > 0);
    if(empty()) {
        Buffer buffer(buf, len);
        swap(buffer);
        return;
    }
    if (capacity_ - rightIndex_ >= len) {
        ;
    } else if (len + size() <= capacity_) {
        forward();
    } else {
        off_t minSize = size() + len + leftIndex_;
        reserve(minSize);
    }
    safeAppend(buf, len);
}

void Buffer::append(const std::string &str) {
    append(str.data(), str.size() + 1);
}

void Buffer::prepend(const std::string &str) {
    prepend(str.data(), str.size() + 1);
}

void Buffer::safeAppend(const char *buf, off_t len) {
    ::memcpy(buffer_ + rightIndex_, buf, len);
    rightIndex_ += len;
}

void Buffer::prepend(const Buffer &that) { prepend(that.data(), that.size()); }

void Buffer::prepend(const char *buf, off_t len) {
    assert(buf && len > 0);
    if (leftIndex_ >= len) {
        ;
    } else if (len + size() <= capacity_) {
        backward(len - leftIndex_);
    } else {
        Buffer nb(len + size());
        nb.append(*this);
        swap(nb);
    }
    safePreappend(buf, len);
}

void Buffer::safePreappend(const char *buf, off_t len) {
    ::memcpy(data() - len, buf, len);
    leftIndex_ -= len;
}

void Buffer::forward(off_t offset) {
    assert(offset >= 0);
    if (offset == 0) {
        if (leftIndex_ == 0) {
            return;
        }
        ::memmove(buffer_, buffer_ + leftIndex_, size());
        rightIndex_ -= leftIndex_;
        leftIndex_ = 0;
    } else if (offset >= rightIndex_) {
        leftIndex_ = rightIndex_ = 0;
    } else if (offset <= leftIndex_) {
        ::memmove(buffer_ + leftIndex_ - offset, buffer_ + leftIndex_,
                  rightIndex_ - leftIndex_);
        leftIndex_ -= offset;
        rightIndex_ -= offset;
    } else {
        ::memmove(buffer_, buffer_ + rightIndex_ - offset,
                  rightIndex_ - offset);
        leftIndex_ = 0;
        rightIndex_ -= offset;
    }
}

void Buffer::backward(off_t offset) {
    assert(offset >= 0);
    if (offset == 0) {
        if (rightIndex_ == capacity_) {
            return;
        }
        ::memmove(buffer_ + leftIndex_ + capacity_ - rightIndex_,
                  buffer_ + leftIndex_, capacity_ - rightIndex_);
        leftIndex_ += capacity_ - rightIndex_;
        rightIndex_ = capacity_;
    } else if (offset >= capacity_ - leftIndex_) {
        leftIndex_ = rightIndex_ = capacity_;
    } else if (offset <= capacity_ - rightIndex_) {
        ::memmove(buffer_ + leftIndex_ + offset, buffer_ + leftIndex_,
                  rightIndex_ - leftIndex_);
        leftIndex_ += offset;
        rightIndex_ += offset;
    } else {
        ::memmove(buffer_ + leftIndex_ + offset, buffer_ + leftIndex_,
                  capacity_ - leftIndex_ - offset);
        leftIndex_ += capacity_ - leftIndex_ - offset;
        rightIndex_ = capacity_;
    }
}

void Buffer::reserve(off_t minSize) {
    if (minSize < capacity_)
        return;
    off_t newSize = capacity_ << 1;
    while (newSize < minSize)
        newSize = newSize << 1;
    resize(newSize);
}

void Buffer::resize(off_t minSize) {
    if (minSize < capacity_)
        return;
    char *newBuffer = new char[minSize];
    ::memcpy(newBuffer + leftIndex_, buffer_ + leftIndex_, size());
    delete[] buffer_;
    buffer_ = newBuffer;
    capacity_ = minSize;
}

void Buffer::retrieve() { leftIndex_ = rightIndex_ = 0; }

void Buffer::retrieve(off_t len) {
    assert(len >= 0);
    if (size() == 0) {
        return;
    } else if (len > size()) {
        leftIndex_ = rightIndex_ = 0;
    } else {
        leftIndex_ += len;
    }
    //    if (size() < capacity_ / 2) {
    //        Buffer b(*this);
    //        swap(b);
    //    }
}

std::string Buffer::retriveveAllAsString() { return retriveveAsString(size()); }

std::string Buffer::retriveveAsString(off_t len) {
    assert(len >= 0);
    if (len == 0 || len > size())
        return retriveveAsString(size());
    std::string ret(data(), len);
    retrieve(len);
    return ret;
}

std::vector<char> Buffer::retriveveAllAsVector() {
    return retriveveAsVector(size());
}

std::vector<char> Buffer::retriveveAsVector(off_t len) {
    assert(len >= 0);
    if (len == 0 || len > size())
        return retriveveAllAsVector();
    std::vector<char> ret(&buffer_[leftIndex_], &buffer_[rightIndex_]);
    retrieve(len);
    return ret;
}

void Buffer::resetBuffer() {
    if (buffer_) {
        delete[] buffer_;
        buffer_ = nullptr;
    }
}

void Buffer::shrink(off_t len) {
    if (len < capacity_) {
        char *newBuffer = new char[len];
        ::memcpy(newBuffer, buffer_, len);
        delete[] buffer_;
        buffer_ = newBuffer;
        capacity_ = len;
        leftIndex_ = rightIndex_ = 0;
    }
}

void Buffer::shrink_to_fit(off_t len) {
    if (len < size()) {
        shrink(size());
    } else if (len < capacity_ - leftIndex_) {
        char *newBuffer = new char[len + leftIndex_];
        ::memcpy(newBuffer + leftIndex_, buffer_ + leftIndex_, size());
        delete[] buffer_;
        buffer_ = newBuffer;
        capacity_ = len + leftIndex_;
    }
}

bool Buffer::empty() const { return leftIndex_ == rightIndex_; }

char *Buffer::wdata() {
    assert(buffer_ && rightIndex_ >= leftIndex_);
    return buffer_ + rightIndex_;
}

off_t Buffer::availbleSize() const {
    assert(buffer_ && rightIndex_ >= leftIndex_);
    return capacity_ - rightIndex_;
}
