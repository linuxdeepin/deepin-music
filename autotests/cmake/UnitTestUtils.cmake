# SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
# SPDX-License-Identifier: GPL-3.0-or-later
#
# UnitTestUtils.cmake - helper to add a Google Test module that links the
# project's `dmusic` shared library (built from src/libdmusic). Linking the
# real, gcov-instrumented library lets the tests exercise the production code
# and lets lcov report function coverage on src/libdmusic.

# add_ut_module(<module_name>)
# Glob test_*.cpp in the calling directory, build one GTest executable that
# links dmusic + Qt6 + system deps, and register it with ctest.
function(add_ut_module MODULE_NAME)
    file(GLOB TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/test_*.cpp")
    if(NOT TEST_SOURCES)
        message(STATUS "UT: no test files in ${MODULE_NAME}, skipping")
        return()
    endif()

    set(STUB_SHADOW ${CMAKE_SOURCE_DIR}/autotests/3rdparty/stub/stub-shadow.cpp)
    set(UT_MAIN ${CMAKE_SOURCE_DIR}/autotests/ut_main.cpp)

    add_executable(test_${MODULE_NAME} ${TEST_SOURCES} ${STUB_SHADOW} ${UT_MAIN})

    target_include_directories(test_${MODULE_NAME} PRIVATE
        ${CMAKE_SOURCE_DIR}/autotests/3rdparty/stub
        ${CMAKE_SOURCE_DIR}/autotests/common
        ${CMAKE_SOURCE_DIR}/src/libdmusic
        ${CMAKE_SOURCE_DIR}/src/libdmusic/core
        ${CMAKE_SOURCE_DIR}/src/libdmusic/player
        ${CMAKE_SOURCE_DIR}/src/libdmusic/player/vlc
        ${CMAKE_SOURCE_DIR}/src/libdmusic/util
        /usr/include/vlc
        /usr/include/vlc/plugins
    )

    target_link_libraries(test_${MODULE_NAME} PRIVATE
        GTest::gtest
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Svg
        Qt6::Multimedia
        Qt6::Xml
        Qt6::Network
        Qt6::Sql
        Qt6::DBus
        Qt6::Core5Compat
        PkgConfig::MPRIS
        PkgConfig::TAGLIB
        PkgConfig::DTK
        PkgConfig::FFMPEG
        PkgConfig::SDL2
        dmusic
    )

    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_link_libraries(test_${MODULE_NAME} PRIVATE gcov)
    endif()

    target_compile_definitions(test_${MODULE_NAME} PRIVATE
        QT_QPA_PLATFORM=offscreen
        TEST_DATA_DIR="${CMAKE_SOURCE_DIR}/autotests/testdata"
    )

    gtest_discover_tests(test_${MODULE_NAME}
        PROPERTIES
            ENVIRONMENT "QT_QPA_PLATFORM=offscreen;ASAN_OPTIONS=detect_leaks=0:halt_on_error=0:abort_on_error=0;UBSAN_OPTIONS=halt_on_error=0:print_stacktrace=0"
            TIMEOUT 300
    )

    message(STATUS "UT: test_${MODULE_NAME} configured with ${TEST_SOURCES}")
endfunction()
