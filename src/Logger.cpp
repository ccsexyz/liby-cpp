#include "Logger.h"
#include "util.h"
#include <chrono>
#include <iostream>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <thread>
#include <unistd.h>

using namespace Liby;

Logger::Logger(LogLevel level) : level_(level) {
    //    std::thread logger_thread_([this] { logger_thread(); });
    //    logger_thread_.detach();
}

const char *Logger::getLevelString(LogLevel level) {
    static const char *levelString_[] = {"VERBOSE", "DEBUG", "INFO",
                                         "WARN",    "ERROR", "FATAL"};
    return levelString_[static_cast<int>(level)];
}

void Logger::log(LogLevel level, const char *file, const int line,
                 const char *func, const char *fmt...) {
    if (level < level_)
        return;
    int offset = 0;
    const int buffersize = 4096;
    char buffer[buffersize];
    char *buf = buffer;

    offset += snprintf(buf + offset, buffersize - offset,
                       "[%s] [%s]: %s: %d %s ", Logger::getLevelString(level),
                       Timestamp::now().toString().c_str(), file, line, func);

    va_list args;
    va_start(args, fmt);
    offset += vsnprintf(buf + offset, buffersize - offset, fmt, args);
    va_end(args);

    while (buf[offset] == '\n' || buf[offset] == '\0')
        offset--;
    buf[++offset] = '\n';
    buf[++offset] = '\0';

    int ret = ::write(STDERR_FILENO, buf, ::strlen(buf));
    // avoid the "variable set but not use" warning
    ClearUnuseVariableWarning(ret);

    //    queue_.push_notify(std::string(buf));
}

void Logger::logger_thread() {
    int64_t bytes = 0;
    //    ::alarm(1);
    //    Signal::signal(SIGALRM, [&bytes] {
    //        std::cout << "write " << bytes / (1024 * 1024) << " MB" <<
    //        std::endl;
    //        bytes = 0;
    //        ::alarm(1);
    //    });
    while (1) {
        auto logStr = queue_.pop_wait();
        if (logStr.empty())
            continue;
        bytes += logStr.size();
        ::fwrite(logStr.data(), logStr.size(), 1, stderr);
    }
}

std::string Logger::getTimeStr() { return Timestamp::now().toString(); }

Logger &Logger::getLogger() {
    static Logger logger;
    return logger;
}
