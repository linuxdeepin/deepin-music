# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

#project(deepin-music)
#set(CMD_NAME deepin-music)

include_directories(${CMAKE_INCLUDE_CURRENT_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

find_package(Qt5Core)
find_package(Qt5Gui)
find_package(Qt5Widgets)
find_package(Qt5Svg)
#find_package(Qt5Multimedia)
find_package(Qt5Xml)
find_package(Qt5Sql)
find_package(Qt5DBus)
find_package(Qt5LinguistTools)
#find_package(Qt5X11Extras)

include(GNUInstallDirs)
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
   set(CMAKE_INSTALL_PREFIX /usr)
endif()

# 判断系统环境
# 紧凑模式只在1060（dtk 5.6.4以后）上支持
execute_process(COMMAND cat /etc/os-version OUTPUT_VARIABLE OS_INFO_STR)
string(REGEX MATCHALL "MinorVersion=[0-9]+" MINOR_STR "${OS_INFO_STR}")
string(REGEX MATCH "[0-9]+" MINOR_VERSION "${MINOR_STR}")
if (MINOR_VERSION MATCHES "1060")
    message("--------------------- os build on 1060")
    add_definitions(-DOS_BUILD_1060)
endif()

add_subdirectory(libdmusic)
add_subdirectory(music-player)
add_subdirectory(libmusic-plugin)
