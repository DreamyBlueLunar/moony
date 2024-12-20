#include "logger.h"

#include <iostream>

moony::logger& moony::logger::get_instance(void) {
    static logger obj;
    return obj;
}

void moony::logger::set_log_level(int log_level) {
    log_level_ = log_level;
}

// 日志格式：[INFO] time: msg
void moony::logger::log(const std::string& msg) {
    switch (log_level_) {
        case INFO:
            std::cout << "[INFO] ";
            break;
        case ERROR:
            std::cout << "[ERROR] ";
            break;
        case FATAL:
            std::cout << "[FATAL] ";
            break;
        case DEBUG:
            std::cout << "[DEBUG] ";
            break;
        default:
            break;
    }

    std::cout << moony::time_stamp::now().to_string() << ": " << msg;
}
