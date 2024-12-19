#pragma once

#include <sstream>
#include <string>
#include <stdlib.h>

#include "../noncopyable.h"
#include "../time_stamp/time_stamp.h"

#define MUDEBUG
#define LOG_BUF_LEN 1024

// C 库函数 int snprintf(char *str, size_t size, const char *format, ...) 
// 设将可变参数(...)按照 format 格式化成字符串，并将字符串复制到 str 中，
// size 为要写入的字符的最大数目，超过 size 会被截断，最多写入 size-1 个字符。
// LOG_INFO("%s, %d", arg1, arg2);
#define LOG_FMT_INFO(format_msg, ...)  do {                             \
        lee::logger& logger = lee::logger::get_instance();          \
        logger.set_log_level(INFO);                                 \
        char buffer[LOG_BUF_LEN] = {0};                             \
        snprintf(buffer, LOG_BUF_LEN, format_msg, ##__VA_ARGS__);  \
        logger.log(buffer);                                         \
    } while(0)

#define LOG_FMT_ERROR(format_msg, ...)  do {                            \
        lee::logger& logger = lee::logger::get_instance();          \
        logger.set_log_level(ERROR);                                \
        char buffer[LOG_BUF_LEN] = {0};                             \
        snprintf(buffer, LOG_BUF_LEN, format_msg, ##__VA_ARGS__);  \
        logger.log(buffer);                                         \
    } while(0)

#define LOG_FMT_FATAL(format_msg, ...)  do {                            \
        lee::logger& logger = lee::logger::get_instance();          \
        logger.set_log_level(FATAL);                                \
        char buffer[LOG_BUF_LEN] = {0};                             \
        snprintf(buffer, LOG_BUF_LEN, format_msg, ##__VA_ARGS__);  \
        logger.log(buffer);                                         \
        exit(-1);                                                   \
    } while(0)

#ifdef MUDEBUG
#define LOG_FMT_DEBUG(format_msg, ...)  do {                            \
        lee::logger& logger = lee::logger::get_instance();          \
        logger.set_log_level(DEBUG);                                \
        char buffer[LOG_BUF_LEN] = {0};                             \
        snprintf(buffer, LOG_BUF_LEN, format_msg, ##__VA_ARGS__);  \
        logger.log(buffer);                                         \
    } while(0)
#else
#define LOG_FMT_DEBUG(format_msg, ...)
#endif

enum log_level {
    INFO,
    ERROR,
    FATAL,
    DEBUG
};

namespace lee {
class logger : noncopyable {
public:
    static logger& get_instance();
    void set_log_level(int log_level);
    void log(const std::string& msg);

private:
    logger() = default;
    ~logger() = default;

    int log_level_;
}; 
} // namespace lee
