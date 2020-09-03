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
set(CMAKE_CXX_FLAGS "-g -fprofile-arcs -ftest-coverage")

set(GCC_COVERAGE_COMPILE_FLAGS "-fprofile-arcs -ftest-coverage")
set(GCC_COVERAGE_LINK_FLAGS "-lgcov")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}")

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

include_directories(googletest)
include_directories(googletest/googlemock/include)
include_directories(googletest/googletest/include)

aux_source_directory(./ mainSource)#单元测试相关代码

find_package(PkgConfig REQUIRED)
include_directories(${PROJECT_BINARY_DIR})
include_directories(${PROJECT_SOURCE_DIR})

if(ISTEST MATCHES "1")
    set(QtModule Core Gui Widgets Svg Multimedia Xml Network Sql DBus Test)
else()
    set(QtModule Core Gui Widgets Svg Multimedia Xml Network Sql DBus)
endif()

#先查找到这些qt相关的模块以供链接使用
find_package(Qt5 REQUIRED ${QtModule})

add_subdirectory(../music build)
add_subdirectory(googletest)
