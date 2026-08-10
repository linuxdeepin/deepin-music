# SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
# SPDX-License-Identifier: GPL-3.0-or-later
#
# ut_utils —— Utils 静态工具类（src/libdmusic/util/utils.cpp）单元测试

cmake_minimum_required(VERSION 3.10)
project(ut_utils)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core DBus)
find_package(Threads REQUIRED)

set(LIBDMUSIC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../src/libdmusic)
include_directories(
    ${LIBDMUSIC_DIR}
    ${LIBDMUSIC_DIR}/util
    ${LIBDMUSIC_DIR}/core
    ${LIBDMUSIC_DIR}/player
    ${LIBDMUSIC_DIR}/player/vlc)

link_directories(${CMAKE_BINARY_DIR}/src/libdmusic)

add_executable(${PROJECT_NAME} test_utils.cpp)
target_compile_options(${PROJECT_NAME} PRIVATE -fprofile-arcs -ftest-coverage -g)
target_link_options(${PROJECT_NAME} PRIVATE -fprofile-arcs)
target_link_libraries(${PROJECT_NAME} dmusic Qt6::Core Qt6::DBus gtest gtest_main Threads::Threads)

include(CTest)
add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})
