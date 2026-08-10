# SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
# SPDX-License-Identifier: GPL-3.0-or-later
#
# ut_ykmeans —— KMEANS 模板类（src/libdmusic/util/ykmeans.h）单元测试
# 该类为头文件内联模板，无项目库依赖，独立编译。

cmake_minimum_required(VERSION 3.10)
project(ut_ykmeans)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Threads REQUIRED)

add_executable(${PROJECT_NAME} test_kmeans.cpp)
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/libdmusic/util)

# Debug 覆盖率插桩（与主工程 Debug 一致），供 lcov 统计被测模板函数覆盖率
target_compile_options(${PROJECT_NAME} PRIVATE -fprofile-arcs -ftest-coverage -g)
target_link_options(${PROJECT_NAME} PRIVATE -fprofile-arcs)

target_link_libraries(${PROJECT_NAME} gtest gtest_main Threads::Threads)

include(CTest)
add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})
