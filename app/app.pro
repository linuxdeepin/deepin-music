DEFINES += QT_MESSAGELOGCONTEXT

QT       += core gui widgets svg dbus multimedia
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TEMPLATE    = app
TARGET      = deepin-music
CONFIG      += c++11 link_pkgconfig
PKGCONFIG   += dtkbase dtkutil dtkwidget taglib

SOURCES += \
    main.cpp \
    view/playerframe.cpp \
    view/titlebar.cpp \
    view/footer.cpp \
    view/musiclistview.cpp \
    view/musicitem.cpp \
    view/importwidget.cpp \
    view/lyricview.cpp \
    view/playlistwidget.cpp \
    view/playlistitem.cpp \
    presenter/apppresenter.cpp \
    musicapp.cpp \
    view/musiclistwidget.cpp \
    core/playlistmanager.cpp \
    core/playlist.cpp

HEADERS += \
    view/playerframe.h \
    view/titlebar.h \
    view/footer.h \
    view/musiclistview.h \
    view/musicitem.h \
    view/importwidget.h \
    view/lyricview.h \
    view/playlistwidget.h \
    view/playlistitem.h \
    presenter/apppresenter.h \
    musicapp.h \
    view/musiclistwidget.h \
    model/musiclistmodel.h \
    core/playlistmanager.h \
    core/playlist.h

RESOURCES += \
    resource/theme/theme.qrc \
    resource/resource.qrc

DISTFILES +=
