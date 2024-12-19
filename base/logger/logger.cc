#include "logger.h"

#include <iostream>

lee::logger& lee::logger::get_instance(void) {
    static lee::logger obj;
    return obj;
}

void lee::logger::set_log_level(int log_level) {
    log_level_ = log_level;
}

// 日志格式：[INFO] time: msg
void lee::logger::log(const std::string& msg) {
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

    std::cout << lee::time_stamp::now().to_string() << ": " << msg;
}
