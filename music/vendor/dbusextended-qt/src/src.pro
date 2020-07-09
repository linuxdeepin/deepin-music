include($$PWD/../common.pri)
include($$PWD/../../../config.pri)

TEMPLATE = lib
CONFIG += qt

QT = core dbus

TARGET = $${QTDBUSEXTENDEDLIB}

DESTDIR     = $$BUILD_DIST/lib

QMAKE_SUBSTITUTES = $${TARGET}.prf.in

DEFINES += QT_DBUS_EXTENDED_LIBRARY

# Generate pkg-config support by default
# Note that we HAVE TO also create prl config as QMake implementation
# mixes both of them together.
CONFIG += create_pc create_prl no_install_prl

SOURCES += \
    dbusextendedabstractinterface.cpp \
    dbusextendedpendingcallwatcher.cpp

HEADERS += \
    dbusextended.h \
    dbusextendedabstractinterface.h \
    dbusextendedpendingcallwatcher_p.h

INSTALL_HEADERS = \
    DBusExtended \
    DBusExtendedAbstractInterface \
    dbusextended.h \
    dbusextendedabstractinterface.h

isEmpty(PREFIX){
    target.path = $$[QT_INSTALL_LIBS]
    headers.path = $$[QT_INSTALL_HEADERS]/MprisQt
} else {
    target.path = $${PREFIX}/lib
    headers.path = $${PREFIX}/include/DBusExtended
}

headers.files = $$INSTALL_HEADERS
prf.files = $${TARGET}.prf
prf.path = $$[QMAKE_MKSPECS]/features
INSTALLS += target headers

QMAKE_PKGCONFIG_LIBDIR = $$target.path
QMAKE_PKGCONFIG_INCDIR = $$headers.path
QMAKE_PKGCONFIG_DESTDIR = pkgconfig
QMAKE_PKGCONFIG_NAME = DBusExtended
QMAKE_PKGCONFIG_REQUIRES += Qt5Core Qt5DBus
