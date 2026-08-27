# SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

cmake_minimum_required(VERSION 3.16)
project(deepin-music-dde-shell-plugin LANGUAGES CXX)

# lrelease path — Qt6 uses /usr/lib/qt6/bin/lrelease (the /usr/bin wrapper
# needs QTDIR which may not be set in build environments).
if(QT_VERSION_MAJOR EQUAL 6)
    set(LRELEASE_PATH "/usr/lib/qt6/bin/lrelease")
else()
    set(LRELEASE_PATH "/usr/lib/qt5/bin/lrelease")
endif()

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Qml Quick DBus Widgets)
find_package(Dtk${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Build as a shared library loaded by dde-tray-loader (dock plugin V3).
# Target name differs from the main executable "deepin-music" to avoid
# CMP0002 conflict; OUTPUT_NAME keeps the installed .so as libdeepin-music.so.
add_library(deepin-music-shell SHARED
    musicplugin.h
    musicplugin.cpp
    musicapplet.h
    musicapplet.cpp
    musicprober.h
    musicprober.cpp
    resources/music.qrc
)

set_target_properties(deepin-music-shell PROPERTIES OUTPUT_NAME "deepin-music")

target_include_directories(deepin-music-shell PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    /usr/include/dde-dock
)

target_link_libraries(deepin-music-shell PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Qml
    Qt${QT_VERSION_MAJOR}::Quick
    Qt${QT_VERSION_MAJOR}::DBus
    Qt${QT_VERSION_MAJOR}::Widgets
    Dtk${QT_VERSION_MAJOR}::Core
    Dtk${QT_VERSION_MAJOR}::Gui
)

# Plugin installs to lib/dde-dock/plugins/ 
install(TARGETS deepin-music-shell
    LIBRARY DESTINATION lib/dde-dock/plugins
)

# Install translations (.qm files built by lrelease)
file(GLOB TS_FILES ${CMAKE_CURRENT_SOURCE_DIR}/translations/*.ts)
set(QM_DIR ${CMAKE_CURRENT_BINARY_DIR}/translations)
file(MAKE_DIRECTORY ${QM_DIR})

foreach(TS_FILE ${TS_FILES})
    get_filename_component(TS_BASE ${TS_FILE} NAME_WE)
    set(QM_FILE ${QM_DIR}/${TS_BASE}.qm)
    add_custom_command(
        OUTPUT ${QM_FILE}
        COMMAND ${LRELEASE_PATH} -removeidentical ${TS_FILE} -qm ${QM_FILE}
        DEPENDS ${TS_FILE}
        COMMENT "Compiling ${TS_BASE}.ts → ${TS_BASE}.qm"
    )
    list(APPEND QM_FILES ${QM_FILE})
endforeach()

add_custom_target(music_translations ALL DEPENDS ${QM_FILES})
add_dependencies(deepin-music-shell music_translations)

install(DIRECTORY ${QM_DIR}/
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/deepin-music-dde-shell-plugin/translations
    FILES_MATCHING PATTERN "*.qm"
)
