DEFINES += QT_MESSAGELOGCONTEXT

QT       += core gui widgets svg dbus multimedia multimediawidgets xml network
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
    view/playlistitem.cpp \
    presenter/apppresenter.cpp \
    musicapp.cpp \
    view/musiclistwidget.cpp \
    core/playlistmanager.cpp \
    core/playlist.cpp \
    view/playlistview.cpp \
    view/playlistwidget.cpp \
    core/mediafilemonitor.cpp \
    view/resizablestackedwidget.cpp \
    core/player.cpp \
    view/widget/slider.cpp \
    view/widget/infodialog.cpp \
    view/widget/cover.cpp

HEADERS += \
    view/playerframe.h \
    view/titlebar.h \
    view/footer.h \
    view/musiclistview.h \
    view/musicitem.h \
    view/importwidget.h \
    view/lyricview.h \
    view/playlistitem.h \
    presenter/apppresenter.h \
    musicapp.h \
    view/musiclistwidget.h \
    model/musiclistmodel.h \
    core/playlistmanager.h \
    core/playlist.h \
    view/playlistview.h \
    view/playlistwidget.h \
    core/mediafilemonitor.h \
    view/resizablestackedwidget.h \
    core/player.h \
    view/widget/slider.h \
    view/widget/infodialog.h \
    view/widget/cover.h

RESOURCES += \
    resource/theme/theme.qrc \
    resource/resource.qrc

DISTFILES +=


isEmpty(PREFIX){
    PREFIX = /usr
}
binary.path = $${PREFIX}/bin
binary.files = deepin-music

INSTALLS    += binary
