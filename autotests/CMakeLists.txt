# SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
# SPDX-License-Identifier: GPL-3.0-or-later
#
# autotests/CMakeLists.txt - Google Test suite for the deepin-music `dmusic`
# backend library. Tests link the real, gcov-instrumented `dmusic` shared
# library so lcov can report function coverage on src/libdmusic.

cmake_minimum_required(VERSION 3.16)
project(autotests VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(UnitTestUtils)

enable_testing()

# ---- Google Test ----
find_package(GTest REQUIRED)
include(GoogleTest)

# ---- Qt6 (mirror src/libdmusic) ----
find_package(Qt6 REQUIRED COMPONENTS
    Core Gui Widgets Svg Multimedia Xml Network Sql DBus Core5Compat)

# ---- System dependencies (mirror src/libdmusic) ----
find_package(PkgConfig REQUIRED)
pkg_check_modules(MPRIS REQUIRED IMPORTED_TARGET mpris-qt6)
pkg_check_modules(TAGLIB REQUIRED IMPORTED_TARGET taglib)
pkg_check_modules(DTK    REQUIRED IMPORTED_TARGET dtk6core)
pkg_check_modules(FFMPEG REQUIRED IMPORTED_TARGET libavcodec libavformat)
pkg_check_modules(SDL2   REQUIRED IMPORTED_TARGET sdl2)

# ---- Code coverage: gcov instrumentation in Debug ----
# autotests/run-ut.sh step_5 (lcov --capture) depends on this; the project root
# CMakeLists.txt already instruments the `dmusic` library in Debug, so the
# coverage data reflects src/libdmusic functions executed by these tests.
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage -O0")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lgcov")
    message(STATUS "UT: Code coverage enabled (gcov instrumentation)")
endif()

# ---- Test subdirectories (one GTest executable per module) ----
add_subdirectory(core)
add_subdirectory(player)
add_subdirectory(util)

message(STATUS "UT: Unit tests configuration complete")
