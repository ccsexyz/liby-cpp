#ifndef LIBY_CPP_HTTPDEF_H
#define LIBY_CPP_HTTPDEF_H

#include "../Buffer.h"
#include <string>
#include <unordered_map>

namespace Liby {
namespace http {
enum {
    HTTP_OK = 200,
    HTTP_CREATED = 201,
    HTTP_ACCEPTED = 202,
    HTTP_NO_CONTENT = 204,
    HTTP_MULTIPLE_CHOICES = 300,
    HTTP_MOVED_PERMANENTLY = 301,
    HTTP_MOVED_TEMPORARILY = 302,
    HTTP_NOT_MODIFIED = 304,
    HTTP_BAD_REQUEST = 400,
    HTTP_UNAUTHORIZED = 401,
    HTTP_FORBIDDEN = 403,
    HTTP_NOT_FOUND = 404,
    HTTP_INTERNAL_SERVER_ERROR = 500,
    HTTP_NOT_IMPLEMENTED = 501,
    HTTP_BAD_GATEWAY = 502,
    HTTP_SERVICE_UNAVAILABLE = 503
};

enum { INVALID_METHOD, GET, POST, OPTIONS, HEAD };

struct Reply {
public:
    int status_;
    Buffer content_;
    std::unordered_map<std::string, std::string> headers_;

    static std::string Error(int errcode);
    std::string toString();
};

struct ReplyParser final : public Reply {
public:
    enum {
        ParseStarting = 0,
        ParsingVersion = 1,
        ParsingStatus = 2,
        ParsingStatusString = 3,
        ParsingHeaders = 4,
        ParsingBody = 5,
        Good = 6,
        InvalidVersion = 7,
        InvalidStatus = 8,
        InvalidStatusString = 9,
        InvalidHeader = 10
    };
    int bytes_ = 0;

    void parse(const char *begin, const char *end);

    void clear();

    bool isError() const { return progress_ >= InvalidVersion; }

    bool isGood() const { return progress_ == Good; }

private:
    bool ParseHttpVersion(const char *begin, const char *end);
    bool ParseStatus(const char *begin, const char *end);
    bool ParseStatusString(const char *begin, const char *end);
    bool ParseHeader(const char *begin, const char *end);
    bool ParseBody(const char *begin, const char *end);

private:
    int progress_ = ParseStarting;
};

struct Request {
public:
    int method_ = INVALID_METHOD;
    std::string uri_;
    std::string query_;
    std::unordered_map<std::string, std::string> headers_;
    Buffer body_;

    void print();
    std::string toString(); //不转换body_部分
};

struct RequestParser final : public Request {
public:
    enum {
        ParsingMethod = 1,  // 刚开始分析
        ParsingURI = 2,     // 正在分析URI
        ParsingVersion = 3, // 正在分析http版本
        ParsingHeaders = 4, // 正在分析headers但数据不完整
        Good = 5,           // 分析完成,有效请求
        InvalidMethod = 6,
        InvalidURI = 7,
        InvalidVersion = 8,
        InvalidHeader = 9
    };

    int progress_ = ParsingMethod;
    int bytes_ = 0; // Request部分(不包含body)的长度
    int http_version_major_ = 0;
    int http_version_minor_ = 0;

    void parse(const char *begin, const char *end);
    void clear();

    bool isError() const { return progress_ >= InvalidMethod; }

    bool isGood() const { return progress_ == Good; }

private:
    bool ParseHttpVersion(const char *begin, const char *end);
    bool ParseURI(const char *begin, const char *end);
    bool ParseMethod(const char *begin, const char *end);
    bool ParseHeader(const char *begin, const char *end);
};

std::string getMimeType(const std::string &uri);
}
}

#endif // LIBY_CPP_HTTPDEF_H
