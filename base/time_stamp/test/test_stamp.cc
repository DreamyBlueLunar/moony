#include <gtest/gtest.h>
#include <iostream>

#include "../time_stamp.h"

TEST(TEST_TIME_STAMP, GET_TIME_OF_NOW) {
    std::cout << lee::time_stamp::now().to_string() << std::endl;
}