cmake_minimum_required(VERSION 3.0.0)

project(leemuduo)

# 设置 C++14 标准
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置库文件的输出目录为项目源目录下的 lib 文件夹
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/tests)

# 从 base、net 目录及其子目录添加所有源文件
file(GLOB_RECURSE SRC_LIST
    ./base/*.cc
    ./base/**/*.cc
    ./net/*.cc
    ./net/**/*.cc
)

# 从 base、net 目录及其子目录添加所有头文件
file(GLOB_RECURSE HEADER_LIST
    ./base/*.h
    ./base/**/*.h
    ./net/*.cc
    ./net/**/*.cc
)
message(STATUS "Source files: ${SRC_LIST}")

# 添加共享库
add_library(leemuduo SHARED ${SRC_LIST})

add_subdirectory(base/logger)
add_subdirectory(base/time_stamp)
add_subdirectory(net/inetaddress)