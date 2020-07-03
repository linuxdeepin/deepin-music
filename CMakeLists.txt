#定义需要的cmake版本
cmake_minimum_required(VERSION 3.0)

project(music)

#cmake属性
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

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

find_package(PkgConfig REQUIRED)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories(${PROJECT_BINARY_DIR})
include_directories(${PROJECT_SOURCE_DIR})
#Multimediawidgets是否已使用？
set(QtModule Core Gui Widgets Svg Multimedia Xml Network Sql DBus)
#configure_file
#先查找到这些qt相关的模块以供链接使用
find_package(Qt5 REQUIRED ${QtModule})


add_subdirectory(music)
