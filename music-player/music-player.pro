include($$PWD/../config.pri)
include($$PWD/build.pri)
include($$PWD/../interface/interface.pri)

QT       += core gui widgets svg multimedia multimediawidgets xml network sql dbus
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TEMPLATE    = app
TARGET      = $$APP_TARGET
CONFIG      += c++11 link_pkgconfig
DESTDIR     = $$BUILD_DIST/bin

unix{
    DEFINES += SUPPORT_INOTIFY
    QT += x11extras
    PKGCONFIG += icu-uc icu-i18n libavutil libavformat libavcodec
    PKGCONFIG += x11 xext
    PKGCONFIG += dtkbase dtkutil dtkwidget
    PKGCONFIG += dtksettings dtksettingsview
#    PKGCONFIG += dtkbase dtkutil dtkwidget
#   PKGCONFIG += mpris-qt5 dbusextended-qt5
}

INCLUDEPATH += $$PWD/view/helper

SOURCES += \
    core/util/lyric.cpp \
    core/mediadatabase.cpp \
    core/mediadatabasewriter.cpp \
    core/playlist.cpp \
    core/playlistmanager.cpp \
    presenter/presenter.cpp \
    view/helper/widgethellper.cpp \
    view/widget/cover.cpp \
    view/widget/infodialog.cpp \
    view/widget/lyriclinedelegate.cpp \
    view/widget/modebuttom.cpp \
    view/widget/musicitemdelegate.cpp \
    view/widget/musiclistview.cpp \
    view/widget/playlistitem.cpp \
    view/widget/playlistview.cpp \
    view/widget/pushbutton.cpp \
    view/widget/searchedit.cpp \
    view/widget/searchresult.cpp \
    view/widget/slider.cpp \
    musicapp.cpp \
    main.cpp \
    view/widget/label.cpp \
    view/widget/tip.cpp \
    view/widget/hint.cpp \
    view/widget/soundvolume.cpp \
    view/widget/filter.cpp \
    core/player.cpp \
    view/widget/thinwindow.cpp \
    view/helper/thememanager.cpp \
    view/widget/titlebarwidget.cpp \
    view/widget/ddropdown.cpp \
    view/widget/dsettingdialog.cpp \
    view/widget/picturesequenceview.cpp \
    view/widget/searchmetalist.cpp \
    view/widget/searchmetaitem.cpp \
    core/medialibrary.cpp \
    core/util/global.cpp \
    core/metasearchservice.cpp \
    core/settings.cpp \
    core/pluginmanager.cpp \
    view/importwidget.cpp \
    view/titlebar.cpp \
    view/musiclistwidget.cpp \
    view/footer.cpp \
    view/lyricview.cpp \
    view/playlistwidget.cpp \
    view/mainframe.cpp
#    view/mainwindow.cpp \


HEADERS += \
    core/util/lyric.h \
    core/util/thearpool.h \
    core/mediadatabase.h \
    core/mediadatabasewriter.h \
    core/music.h \
    core/playlist.h \
    core/playlistmanager.h \
    presenter/presenter.h \
    view/helper/widgethellper.h \
    view/widget/cover.h \
    view/widget/infodialog.h \
    view/widget/lyriclinedelegate.h \
    view/widget/modebuttom.h \
    view/widget/musicitemdelegate_p.h \
    view/widget/musicitemdelegate.h \
    view/widget/musiclistview.h \
    view/widget/playlistitem.h \
    view/widget/playlistview.h \
    view/widget/pushbutton.h \
    view/widget/searchedit.h \
    view/widget/searchresult.h \
    view/widget/slider.h \
    musicapp.h \
    view/widget/label.h \
    view/widget/tip.h \
    view/importwidget.h \
    view/widget/hint.h \
    view/widget/soundvolume.h \
    view/widget/filter.h \
    core/player.h \
    presenter/presenter_p.h \
    view/widget/thinwindow.h \
    view/helper/thememanager.h \
    view/titlebar.h \
    view/widget/titlebarwidget.h \
    view/widget/ddropdown.h \
    view/widget/dsettingdialog.h \
    view/widget/picturesequenceview.h \
    view/widget/searchmetalist.h \
    view/widget/searchmetaitem.h \
    core/medialibrary.h \
    core/util/global.h \
    core/metasearchservice.h \
    core/settings.h \
    core/pluginmanager.h \
    view/musiclistwidget.h \
    view/footer.h \
    view/lyricview.h \
    view/playlistwidget.h \
    view/mainframe.h \
#    view/mainwindow.h \

RESOURCES += \
    resource/theme/theme.qrc \
    config.qrc

unix{
SOURCES += \
    view/helper/xutil.cpp \
    core/util/inotifyengine.cpp

HEADERS += \
    view/helper/xutil.h \
    core/util/inotifyengine.h

# Automating generation .qm files from .ts files
    system($$PWD/../tool/translate_generation.sh)
}

TRANSLATIONS += $$PWD/translations/$${TARGET}.ts \
                $$PWD/translations/$${TARGET}_zh_CN.ts

include($$PWD/install.pri)
