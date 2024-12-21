# 查找 GTest
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

# 添加测试可执行文件
add_executable(test_thread test/test_base_thread.cc
                           base_thread.cc
                           ../current_thread/current_thread.cc)

# 链接 Logger 库和 GTest 到测试可执行文件
target_link_libraries(test_thread gtest gtest_main pthread)