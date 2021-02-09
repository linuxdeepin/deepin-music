#定义需要的cmake版本
cmake_minimum_required(VERSION 3.0)

project(music-test)

#cmake属性
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(ISTEST "1")#单元测试标志

# 添加gocv覆盖率文件的输出
#set(CMAKE_CXX_FLAGS "-g -fprofile-arcs -ftest-coverage")

#set(GCC_COVERAGE_COMPILE_FLAGS "-fprofile-arcs -ftest-coverage")
#set(GCC_COVERAGE_LINK_FLAGS "-lgcov")

if (CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "mips64")
    set(CMAKE_CXX_FLAGS "-g -fprofile-arcs -ftest-coverage")
    add_definitions(-DSYSTEM_MIPS)
else()
    set(CMAKE_CXX_FLAGS "-fsanitize=undefined,address,leak -fno-omit-frame-pointer -g -fprofile-arcs -ftest-coverage -lgcov")
#    set(CMAKE_C_FLAGS "-fsanitize=undefined,address,leak -fno-omit-frame-pointer")
#    set(CMAKE_Ｌ_FLAGS "-fsanitize=undefined,address,leak -fno-omit-frame-pointer")
endif()

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")
#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}")

# 设置不删除生成的文件夹内容文件（设置此项是为了翻译后的文件不被删除）
set_directory_properties(PROPERTIES CLEAN_NO_CUSTOM 1)

option(USE_DXCB "integration with dxcb platform plugin" OFF)
option(DMR_DEBUG "turn on debug output" off)
option(DTK_DMAN_PORTAL "turn on dman portal support" off)

execute_process(COMMAND uname -m OUTPUT_VARIABLE MACH
    ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)


if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(DMR_DEBUG on)
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS on)

#不需要包含googletest头文件
#include_directories(googletest)
#include_directories(googletest/googlemock/include)
#include_directories(googletest/googletest/include)

aux_source_directory(./ mainSource)#单元测试相关代码

find_package(PkgConfig REQUIRED)
include_directories(${PROJECT_BINARY_DIR})
include_directories(${PROJECT_SOURCE_DIR})

if(ISTEST MATCHES "1")
    set(QtModule Core Gui Widgets Svg Multimedia Xml Network Sql DBus Test)
else()
    set(QtModule Core Gui Widgets Svg Multimedia Xml Network Sql DBus)
endif()

if(ISTEST MATCHES "1")
    set(CMD_NAME deepin-music-test)
else()
    set(CMD_NAME deepin-music)
endif()

#添加覆盖率命令
if(ISTEST MATCHES "1")
    add_test(${CMD_NAME} COMMAND ${CMD_NAME})

    add_custom_target(test
        COMMAND mkdir -p coverageResult
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
    message(${CMAKE_BINARY_DIR})

    add_custom_command(TARGET test
        COMMAND echo "=================== CREAT LCOV REPROT BEGIN ===================="
    #1.清理原先的gcov信息
    COMMAND lcov --directory ./CMakeFiles/${CMD_NAME}.dir --zerocounters
#    COMMAND lcov --directory ./CMakeFiles/${CMD_NAME}.dir --zerocounters
    COMMAND ./build/music-player/${CMD_NAME}

    #2.收集gcov信息到.info文件中
    COMMAND lcov --directory . --capture --output-file ./coverageResult/${CMD_NAME}_Coverage.info

    #3.过滤一些我们不感兴趣的文件的覆盖率信息
    COMMAND echo " =================== do filter begin ==================== "
    COMMAND lcov --remove ./coverageResult/${CMD_NAME}_Coverage.info
    '*/${CMD_NAME}_autogen/*' '*/${CMD_NAME}_autogen/*' '*/usr/include/*' '*/tests/*' '*/googletest/*'
    -o ./coverageResult/${CMD_NAME}_Coverage.info
    COMMAND echo " =================== do filter end ==================== "

    #3.将.info信息生成报告到reprot文件夹中
    COMMAND genhtml -o ./coverageResult/report ./coverageResult/${CMD_NAME}_Coverage.info

    COMMAND echo " =================== CREAT LCOV REPROT END ==================== "

    COMMAND echo " Coverage files have been output to ${CMAKE_BINARY_DIR}/coverageResult/report "

#    COMMAND nohup x-www-browser ${CMAKE_BINARY_DIR}/coverageResult/report/index.html &

    WORKING_DIRECTORY ${CMAKE_BINARY_DIR} )

    add_dependencies(test ${CMD_NAME})
endif()

#先查找到这些qt相关的模块以供链接使用
find_package(Qt5 REQUIRED ${QtModule})

add_subdirectory(../src build)
