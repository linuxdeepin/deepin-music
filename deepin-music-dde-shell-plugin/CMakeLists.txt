# SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

set(QT_VERSION_MAJOR 6)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Qml Quick DBus)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# DCI icon resources (toolbar play/pause/next/previous)
qt_add_resources(DCI_QRC_SOURCES dci.qrc)

qt_add_executable(deepin-music-dde-shell-plugin
    main.cpp
    musicapplet.h
    musicapplet.cpp
    musicprober.h
    musicprober.cpp
    themeiconprovider.h
    themeiconprovider.cpp
    ${DCI_QRC_SOURCES}
)

qt_add_qml_module(deepin-music-dde-shell-plugin
    URI org.deepin.music
    VERSION 1.0
    QML_FILES
        package/main.qml
)

target_link_libraries(deepin-music-dde-shell-plugin PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Qml
    Qt${QT_VERSION_MAJOR}::Quick
    Qt${QT_VERSION_MAJOR}::DBus
)

install(TARGETS deepin-music-dde-shell-plugin
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
