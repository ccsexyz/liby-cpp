#include "HttpDef.h"
#include "../Logger.h"
#include "../util.h"
#include <algorithm>
#include <assert.h>
#include <cstring>
#include <tgmath.h>

using namespace Liby;
using namespace Liby::http;

std::unordered_map<std::string, int> methods = {
    {"GET", GET}, {"POST", POST}, {"HEAD", HEAD}, {"OPTIONS", OPTIONS}};

std::unordered_map<int, std::string> methods_string = {
    {GET, "GET"}, {POST, "POST"}, {HEAD, "HEAD"}, {OPTIONS, "OPTIONS"}};

std::unordered_map<std::string, std::string> file_mime = {
    {".html", "text/html"},
    {".xml", "text/xml"},
    {".xhtml", "application/xhtml+xml"},
    {".txt", "text/plain"},
    {".rtf", "application/rtf"},
    {".pdf", "application/pdf"},
    {".word", "application/msword"},
    {".png", "image/png"},
    {".gif", "image/gif"},
    {".jpg", "image/jpeg"},
    {".jpeg", "image/jpeg"},
    {".au", "audio/basic"},
    {".mpeg", "video/mpeg"},
    {".mpg", "video/mpeg"},
    {".avi", "video/x-msvideo"},
    {".gz", "application/x-gzip"},
    {".tar", "application/x-tar"},
    {".css", "text/css"},
    {".cgi", "text/html"},
    {"", "text/plain"}};

std::unordered_map<int, std::string> status_string = {
    {HTTP_OK, "Ok"},
    {HTTP_CREATED, "Created"},
    {HTTP_ACCEPTED, "Accepted"},
    {HTTP_NO_CONTENT, "No Content"},
    {HTTP_MULTIPLE_CHOICES, "Multiple Choices"},
    {HTTP_MOVED_PERMANENTLY, "Moved Permanently"},
    {HTTP_MOVED_TEMPORARILY, "Moved Temporarily"},
    {HTTP_NOT_MODIFIED, "Not Modified"},
    {HTTP_BAD_REQUEST, "Bad RequestParser"},
    {HTTP_UNAUTHORIZED, "Unauthorized"},
    {HTTP_FORBIDDEN, "Forbidden"},
    {HTTP_NOT_FOUND, "Not Found"},
    {HTTP_INTERNAL_SERVER_ERROR, "Internal Server Error"},
    {HTTP_NOT_IMPLEMENTED, "Not Implemented"},
    {HTTP_BAD_GATEWAY, "Bad Gateway"},
    {HTTP_SERVICE_UNAVAILABLE, "Service Unavailable"}

};
std::unordered_map<int, std::string> status_number = {
    {HTTP_OK, "200"},
    {HTTP_CREATED, "201"},
    {HTTP_ACCEPTED, "202"},
    {HTTP_NO_CONTENT, "204"},
    {HTTP_MULTIPLE_CHOICES, "300"},
    {HTTP_MOVED_PERMANENTLY, "301"},
    {HTTP_MOVED_TEMPORARILY, "302"},
    {HTTP_NOT_MODIFIED, "304"},
    {HTTP_BAD_REQUEST, "400"},
    {HTTP_UNAUTHORIZED, "401"},
    {HTTP_FORBIDDEN, "403"},
    {HTTP_NOT_FOUND, "404"},
    {HTTP_INTERNAL_SERVER_ERROR, "500"},
    {HTTP_NOT_IMPLEMENTED, "501"},
    {HTTP_BAD_GATEWAY, "502"},
    {HTTP_SERVICE_UNAVAILABLE, "503"}};

static char parse_hex(const char *s) noexcept {
    int n1, n2;
    if (*s >= '0' && *s <= '9')
        n1 = *s - '0';
    else {
        char temp = *s | 32;
        n1 = temp - 'a' + 10;
    }
    if (*(s + 1) >= '0' && *(s + 1) <= '9')
        n2 = *(s + 1) - '0';
    else {
        char temp = *(s + 1) | 32;
        n2 = temp - 'a' + 10;
    }
    return (n1 * 16 + n2);
}

std::string Reply::toString() {
    std::string ret = "HTTP/1.1 ";
    ret += status_number[status_];
    ret += " ";
    ret += status_string[status_];
    ret += "\r\n";
    if (!content_.empty()) {
        headers_["Content-Length"] = std::to_string(content_.size());
    }
    for (auto &header : headers_) {
        ret += header.first;
        ret += ": ";
        ret += header.second;
        ret += "\r\n";
    }
    ret += "\r\n";
    if (!content_.empty()) {
        ret.append(content_.data(), content_.data() + content_.size());
    }
    return ret;
}

std::string Reply::Error(int errcode) {
    // TODO
    ClearUnuseVariableWarning(errcode);
    return "";
}

void RequestParser::parse(const char *begin, const char *end) {
    bool flag;
    const char *finish = nullptr;
    switch (progress_) {
    default:
    case Good:
        break;
    case ParsingMethod:
        finish = std::find(begin, end, ' ');
        if (finish == nullptr)
            return;
        flag = ParseMethod(begin, finish);
        if (flag == false)
            return;
        bytes_ += ++finish - begin;
        begin = finish;
    case ParsingURI:
        finish = std::find(begin, end, ' ');
        if (finish == nullptr)
            return;
        flag = ParseURI(begin, finish);
        if (flag == false)
            return;
        bytes_ += ++finish - begin;
        begin = finish;
    case ParsingVersion:
        finish = std::find(begin, end, '\n');
        if (finish == nullptr)
            return;
        flag = ParseHttpVersion(begin,
                                *(finish - 1) == '\r' ? finish - 1 : finish);
        if (flag == false)
            return;
        bytes_ += ++finish - begin;
        begin = finish;
    case ParsingHeaders: {
        while (begin != end) {
            finish = std::find(begin, end, '\n');
            if (finish == nullptr)
                return;
            if (begin == finish || begin + 1 == finish) {
                progress_ = Good;
                bytes_ += ++finish - begin;
                return;
            }
            flag =
                ParseHeader(begin, *(finish - 1) == '\r' ? finish - 1 : finish);
            if (flag == false)
                return;
            bytes_ += ++finish - begin;
            begin = finish;
        }
    } break;
    }
}

void RequestParser::clear() {
    progress_ = ParsingMethod;
    bytes_ = 0;
    method_ = INVALID_METHOD;
    uri_ = query_ = "";
    http_version_major_ = http_version_minor_ = 0;
    headers_.clear();
}

bool RequestParser::ParseHttpVersion(const char *begin, const char *end) {
    assert(begin <= end && begin && end);

    int length = end - begin;
    if (length != 8) {
        progress_ = InvalidVersion;
        return false;
    }
    if (!std::strncmp("HTTP/1.1", begin, 8)) {
        http_version_major_ = 1;
        http_version_minor_ = 1;
    } else if (!std::strncmp("HTTP/1.0", begin, 8)) {
        http_version_major_ = 1;
        http_version_minor_ = 0;
    } else {
        progress_ = InvalidVersion;
        return false;
    }
    progress_ = ParsingHeaders;
    return true;
}

bool RequestParser::ParseURI(const char *begin, const char *end) {
    assert(begin <= end && begin && end);

    do {
        int query_index = -1;
        int n = end - begin;
        const char *src = begin;
        char *temp = new (std::nothrow) char[n + 1];
        if (temp == nullptr)
            break;
        int i = 0, j = 0;
        for (; i < n; i++, j++) {
            if (src[i] == '%') {
                temp[j] = parse_hex(src + i + 1);
                i += 2;
            } else {
                temp[j] = src[i];
            }
            if (temp[j] == '?') {
                query_index = j;
                temp[j] = '\0';
            }
        }
        temp[j] = '\0';

        if (query_index != -1)
            query_ = &temp[query_index + 1];
        uri_ = temp;
        progress_ = ParsingVersion;
        return true;
    } while (0);

    progress_ = InvalidURI;
    return false;
}

bool RequestParser::ParseMethod(const char *begin, const char *end) {
    assert(begin <= end && begin && end);

    std::string method((begin), (end));
    auto it = methods.find(method);
    if (it == methods.end()) {
        progress_ = InvalidMethod;
        return false;
    } else {
        method_ = it->second;
        progress_ = ParsingURI;
        return true;
    }
}

bool RequestParser::ParseHeader(const char *begin, const char *end) {
    assert(begin <= end && begin && end);

    const char *mid = std::find(begin, end, ':');
    if (mid == nullptr) {
        progress_ = InvalidHeader;
        return false;
    }

    const char *left = mid - 1;
    const char *right = mid + 1;
    while (std::isspace(*left))
        left--;
    while (std::isspace(*right))
        right++;
    std::string key(begin, left + 1);
    std::string value(right, end);
    headers_[key] = value;
    return true;
}

std::string Liby::http::getMimeType(const std::string &uri) {
    std::string ret;
    unsigned long ops;
    ops = uri.rfind('.');
    if (ops == std::string::npos) {
        return "text/plain";
    } else {
        std::string type(&uri[ops], &uri[uri.size()]);
        auto it = file_mime.find(type);
        if (it == file_mime.end()) {
            return "text/plain";
        } else {
            return it->second;
        }
    }
}

void Request::print() {
    info("uri: %s", uri_.data());
    info("query: %s", query_.data());
    for (auto &x : headers_) {
        info("%s: %s", x.first.data(), x.second.data());
    }
}

std::string Request::toString() {
    std::string ret = methods_string[method_];
    ret += " ";
    ret += uri_;
    if (!query_.empty()) {
        ret += "?";
        ret += query_;
    }
    ret += " HTTP/1.1\r\n";
    for (auto &header : headers_) {
        ret += header.first;
        ret += ": ";
        ret += header.second;
        ret += "\r\n";
    }
    ret += "\r\n";
    if (!body_.empty()) {
        ret.append(body_.data(), body_.data() + body_.size());
    }
    return ret;
}

void ReplyParser::parse(const char *begin, const char *end) {
    bool flag;
    const char *finish = nullptr;
    switch (progress_) {
    default:
        break;
    case Good:
        break;
    case ParseStarting:
        progress_ = ParsingVersion;
    case ParsingVersion:
        finish = std::find(begin, end, ' ');
        if (finish == nullptr)
            return;
        flag = ParseHttpVersion(begin, finish);
        if (flag == false)
            return;
        bytes_ += ++finish - begin;
        begin = finish;
    case ParsingStatus:
        finish = std::find(begin, end, ' ');
        if (finish == nullptr)
            return;
        flag = ParseStatus(begin, finish);
        if (flag == false)
            return;
        bytes_ += ++finish - begin;
        begin = finish;
    case ParsingStatusString:
        finish = std::find(begin, end, '\n');
        if (finish == nullptr)
            return;
        flag = ParseStatusString(begin,
                                 *(finish - 1) == '\r' ? finish - 1 : finish);
        if (flag == false)
            return;
        bytes_ += ++finish - begin;
        begin = finish;
    case ParsingHeaders: {
        while (begin != end) {
            finish = std::find(begin, end, '\n');
            if (finish == nullptr)
                return;
            if (begin == finish || begin + 1 == finish) {
                progress_ = ParsingBody;
                bytes_ += ++finish - begin;
                begin = finish;
                break;
            }
            flag =
                ParseHeader(begin, *(finish - 1) == '\r' ? finish - 1 : finish);
            if (flag == false)
                return;
            bytes_ += ++finish - begin;
            begin = finish;
        }
    }
    case ParsingBody: {
        int len = std::stoi(headers_["Content-Length"]);
        if (len == 0) {
            progress_ = Good;
            return;
        } else if (end - begin < len) {
            return;
        } else if (end - begin >= len) {
            progress_ = Good;
            bytes_ += len;
            content_.append(begin, len);
            return;
        }
    }
    }
}

void ReplyParser::clear() {
    progress_ = ParseStarting;
    bytes_ = 0;
    status_ = 0;
    content_.retrieve();
    headers_.clear();
}

bool ReplyParser::ParseHttpVersion(const char *begin, const char *end) {
    assert(begin <= end && begin && end);

    int length = end - begin;
    if (length != 8) {
        progress_ = InvalidVersion;
        return false;
    }
    if (!std::strncmp("HTTP/1.1", begin, 8)) {
        progress_ = ParsingStatus;
        return true;
    } else {
        progress_ = InvalidVersion;
        return false;
    }
}

bool ReplyParser::ParseStatus(const char *begin, const char *end) {
    int status = std::stoi(std::string(begin, end));
    auto it = status_number.find(status);
    if (it == status_number.end()) {
        return false;
    } else {
        progress_ = ParsingStatusString;
        status_ = status;
        return true;
    }
}

bool ReplyParser::ParseStatusString(const char *begin, const char *end) {
    assert(begin && end);
    progress_ = ParsingHeaders;
    return true;
}

bool ReplyParser::ParseHeader(const char *begin, const char *end) {
    assert(begin <= end && begin && end);

    const char *mid = std::find(begin, end, ':');
    if (mid == nullptr) {
        progress_ = InvalidHeader;
        return false;
    }

    const char *left = mid - 1;
    const char *right = mid + 1;
    while (std::isspace(*left))
        left--;
    while (std::isspace(*right))
        right++;
    std::string key(begin, left + 1);
    std::string value(right, end);
    headers_[key] = value;
    return true;
}

bool ReplyParser::ParseBody(const char *begin, const char *end) {
    assert(begin && end);
    return false;
}
