include($$PWD/../config.pri)
include($$PWD/build.pri)
include($$PWD/../interface/interface.pri)

QT       += core gui widgets svg dbus multimedia multimediawidgets xml network sql
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TEMPLATE    = app
TARGET      = $$APP_TARGET
CONFIG      += c++11 link_pkgconfig
DESTDIR     = $$BUILD_DIST/bin

unix{
    QT += x11extras
    PKGCONFIG += dtkbase dtkutil dtkwidget taglib icu-uc icu-i18n libavutil libavformat libavcodec
    PKGCONFIG += x11 xext
#   PKGCONFIG += mpris-qt5 dbusextended-qt5
}

INCLUDEPATH += $$PWD/view/helper

SOURCES += \
    core/util/cueparser.cpp \
    core/util/icu.cpp \
    core/util/lyric.cpp \
    core/util/musicmeta.cpp \
    core/util/pinyin.cpp \
    core/lyricservice.cpp \
    core/mediadatabase.cpp \
    core/mediadatabasewriter.cpp \
    core/mediafilemonitor.cpp \
    core/playlist.cpp \
    core/playlistmanager.cpp \
    core/pluginmanager.cpp \
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
    view/widget/settingsdialog.cpp \
    view/widget/slider.cpp \
    view/footer.cpp \
    view/lyricview.cpp \
    view/mainwindow.cpp \
    view/musiclistwidget.cpp \
    view/playlistwidget.cpp \
    musicapp.cpp \
    main.cpp \
    view/widget/label.cpp \
    view/widget/tip.cpp \
    view/importwidget.cpp \
    view/widget/hint.cpp \
    view/widget/soundvolume.cpp \
    view/widget/filter.cpp \
    core/qplayer.cpp \
    core/player.cpp \
    view/widget/thinwindow.cpp \
    view/helper/thememanager.cpp \
    view/titlebar.cpp \
    view/widget/titlebarwidget.cpp


HEADERS += \
    core/util/cueparser.h \
    core/util/icu.h \
    core/util/lyric.h \
    core/util/musicmeta.h \
    core/util/pinyin.h \
    core/util/singleton.h \
    core/util/thearpool.h \
    core/lyricservice.h \
    core/mediadatabase.h \
    core/mediadatabasewriter.h \
    core/mediafilemonitor.h \
    core/music.h \
    core/playlist.h \
    core/playlistmanager.h \
    core/pluginmanager.h \
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
    view/widget/settingsdialog.h \
    view/widget/slider.h \
    view/footer.h \
    view/lyricview.h \
    view/mainwindow.h \
    view/musiclistwidget.h \
    view/playlistwidget.h \
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
    view/widget/titlebarwidget.h

RESOURCES += \
    resource/theme/theme.qrc

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

DISTFILES += \
    data/deepin-music.desktop

