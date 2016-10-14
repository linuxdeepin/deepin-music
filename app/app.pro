QT       += core gui widgets svg dbus
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TEMPLATE    = app
TARGET      = deepin-music
CONFIG      += c++11 link_pkgconfig
PKGCONFIG   += dtkbase dtkutil dtkwidget

SOURCES += \
    main.cpp
