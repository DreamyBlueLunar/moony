#include "../logger.h"
#include <gtest/gtest.h>

#define MUDEBUG

TEST(TEST_LOGGER, TEST_LOGGER_WITH_EXPLICIT_STRING) {
    LOG_FMT_INFO("%s\n", "test for LOG_FMT_INFO");
    LOG_FMT_ERROR("%s\n", "test for LOG_FMT_ERROR");
    LOG_FMT_DEBUG("%s\n", "test for LOG_FMT_DEBUG");
    LOG_FMT_FATAL("%s\n", "test for LOG_FMT_FATAL");
}

TEST(TEST_LOGGER, TEST_LOGGER_WITH_CHAR_PTR) {
    char* msg_info = (char*)"test for LOG_FMT_INFO";
    char* msg_error = (char*)"test for LOG_FMT_ERROR";
    char* msg_fatal = (char*)"test for LOG_FMT_FATAL";
    char* msg_debug = (char*)"test for LOG_FMT_DEBUG";

    LOG_FMT_INFO("%s\n", msg_info);
    LOG_FMT_ERROR("%s\n", msg_error);
    LOG_FMT_DEBUG("%s\n", msg_debug);
    LOG_FMT_FATAL("%s\n", msg_fatal);
}

TEST(TEST_LOGGER, TEST_LOGGER_WITH_STR) {
    std::string msg_info = "test for LOG_FMT_INFO";
    std::string msg_error = "test for LOG_FMT_ERROR";
    std::string msg_fatal = "test for LOG_FMT_FATAL";
    std::string msg_debug = "test for LOG_FMT_DEBUG";

    LOG_FMT_INFO("%s\n", msg_info.c_str());
    LOG_FMT_ERROR("%s\n", msg_error.c_str());
    LOG_FMT_DEBUG("%s\n", msg_debug.c_str());
    LOG_FMT_FATAL("%s\n", msg_fatal.c_str());
}
