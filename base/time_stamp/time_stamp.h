#pragma once

#include <iostream>
#include <string>

namespace lee {
class time_stamp {
public:
    static time_stamp now();
    std::string to_string() const;

private:
    time_stamp();
    explicit time_stamp(int);
    
    int64_t microSecondsSinceEpoch_;
};
}