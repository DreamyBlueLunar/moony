#include "time_stamp.h"

#include <time.h>

moony::time_stamp::time_stamp(): micro_seconds_since_epoch_(0) {}

moony::time_stamp::time_stamp(int micro_seconds_since_epoch):
    micro_seconds_since_epoch_(micro_seconds_since_epoch) {}

moony::time_stamp moony::time_stamp::now() {
    return time_stamp(time(NULL));
}

std::string moony::time_stamp::to_string() const {
    tm* tm = localtime(&micro_seconds_since_epoch_);
    char buf[32] = {0};

    snprintf(buf, 32, "%4d/%02d/%02d %02d:%02d:%02d", 
                tm->tm_year + 1900,
                tm->tm_mon + 1, 
                tm->tm_mday, 
                tm->tm_hour + 8, // 我不知道他这个算的是哪个时区，+8就是北京时间
                tm->tm_min, 
                tm->tm_sec);

    return buf;
}