# 查找 GTest
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

# 添加测试可执行文件/
add_executable(test_event_loop_test 
                test/test_event_loop.cc 
                event_loop.cc 
                ../channel/channel.cc 
                ../poller/poller.cc
                ../poller/epoll_poller.cc 
                ../../base/logger/logger.cc 
                ../../base/time_stamp/time_stamp.cc 
                ../../base/current_thread/current_thread.cc 
                ../poller/default_poller.cc)

# 链接 Logger 库和 GTest 到测试可执行文件
target_link_libraries(test_event_loop_test gtest gtest_main pthread)