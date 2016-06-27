#ifndef BUFFER_BUFFER_H
#define BUFFER_BUFFER_H

#include <initializer_list>
#include <memory>
#include <stdio.h>
#include <string>
#include <vector>

namespace Liby {
class Buffer final {
public:
    static const int defaultPrependSize = 0;
    Buffer() = default;
    explicit Buffer(const off_t length);
    Buffer(const char *buf, off_t length);
    Buffer(const std::string &buf);
    Buffer(const std::vector<char> &buf);
    Buffer(const Buffer &that);
    Buffer(Buffer &that);
    Buffer(Buffer &&that);
    Buffer &operator=(const Buffer &that);
    ~Buffer();

    void setBuffer(char *buf, off_t length);

    void swap(Buffer &that);

    char *data();
    const char *data() const;
    char *wdata();
    bool empty() const;
    off_t size() const;
    off_t capacity() const;
    off_t availbleSize() const;
    void append(off_t n);
    void append(const Buffer &that);
    void append(const char *buf, off_t size);
    void append(const std::string &str);
    void prepend(const Buffer &that);
    void prepend(const char *buf, off_t size);
    void prepend(const std::string &str);
    void forward(off_t offset = 0);  // may lose data
    void backward(off_t offset = 0); // may lose data
    void reserve(off_t minSize);
    void resize(off_t minSize);
    void shrink(off_t len);
    void shrink_to_fit(off_t len = 0);
    void retrieve(off_t len);
    void retrieve();
    std::string retriveveAllAsString();
    std::string retriveveAsString(off_t len = 0);
    std::vector<char> retriveveAllAsVector();
    std::vector<char> retriveveAsVector(off_t len = 0);

    template <typename T> void put(T &e) { append(&e, sizeof(e)); }
    template <typename T> void take(T &e) {
        if (sizeof(e) <= size()) {
            e = *reinterpret_cast<T *>(data());
            retrieve(sizeof(e));
        }
    }

private:
    void resetBuffer();
    void copy(Buffer &that);
    void deepCopy(const Buffer &that);
    void safeAppend(const char *buf, off_t size);
    void safePreappend(const char *buf, off_t size);

private:
    char *buffer_ = NULL;
    off_t leftIndex_ = 0;
    off_t rightIndex_ = 0;
    off_t capacity_ = 0;
};
}

#endif // BUFFER_BUFFER_H
