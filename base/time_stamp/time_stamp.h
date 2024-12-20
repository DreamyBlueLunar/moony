#pragma once

#include "../copyable.h"

#include <iostream>
#include <string>

namespace moony {
class time_stamp : copyable{
public:
    time_stamp();
    explicit time_stamp(int);

    static time_stamp now();
    std::string to_string() const;

private:
    int64_t microSecondsSinceEpoch_;
};
}