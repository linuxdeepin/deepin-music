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
    PKGCONFIG += dtkwidget dtksettingsview dtksettings dtkutil dtkbase
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
    view/widget/modebuttom.cpp \
    view/widget/musiclistview.cpp \
    view/widget/playlistitem.cpp \
    view/widget/playlistview.cpp \
    view/widget/pushbutton.cpp \
    view/widget/searchedit.cpp \
    view/widget/searchresult.cpp \
    view/widget/slider.cpp \
    view/widget/label.cpp \
    view/widget/tip.cpp \
    view/widget/soundvolume.cpp \
    view/widget/filter.cpp \
    core/player.cpp \
    view/helper/thememanager.cpp \
    view/widget/titlebarwidget.cpp \
    view/widget/ddropdown.cpp \
    view/widget/searchmetalist.cpp \
    view/widget/searchmetaitem.cpp \
    core/medialibrary.cpp \
    core/util/global.cpp \
    core/metasearchservice.cpp \
    core/settings.cpp \
    core/pluginmanager.cpp \
    view/widget/listview.cpp \
    view/mainframe.cpp \
    view/importwidget.cpp \
    view/titlebar.cpp \
    view/musiclistwidget.cpp \
    view/playlistwidget.cpp \
    musicapp.cpp \
    main.cpp \
    view/widget/model/musiclistmodel.cpp \
    view/widget/delegate/lyriclinedelegate.cpp \
    view/widget/delegate/musicitemdelegate.cpp \
    view/lyricwidget.cpp \
    view/widget/lyricview.cpp \
    view/widget/titeledit.cpp \
    view/widget/settingsdialog.cpp \
    core/util/threadpool.cpp \
    view/footerwidget.cpp \
    view/widget/waterprogress.cpp \
    view/loadwidget.cpp

HEADERS += \
    musicapp.h \
    core/util/lyric.h \
    core/mediadatabase.h \
    core/mediadatabasewriter.h \
    core/music.h \
    core/playlist.h \
    core/playlistmanager.h \
    presenter/presenter.h \
    view/helper/widgethellper.h \
    view/widget/cover.h \
    view/widget/infodialog.h \
    view/widget/modebuttom.h \
    view/widget/musiclistview.h \
    view/widget/playlistitem.h \
    view/widget/playlistview.h \
    view/widget/pushbutton.h \
    view/widget/searchedit.h \
    view/widget/searchresult.h \
    view/widget/slider.h \
    view/widget/label.h \
    view/widget/tip.h \
    view/importwidget.h \
    view/widget/soundvolume.h \
    view/widget/filter.h \
    core/player.h \
    presenter/presenter_p.h \
    view/helper/thememanager.h \
    view/titlebar.h \
    view/widget/titlebarwidget.h \
    view/widget/ddropdown.h \
    view/widget/searchmetalist.h \
    view/widget/searchmetaitem.h \
    core/medialibrary.h \
    core/util/global.h \
    core/metasearchservice.h \
    core/settings.h \
    core/pluginmanager.h \
    view/musiclistwidget.h \
    view/playlistwidget.h \
    view/mainframe.h \
    view/widget/listview.h \
    view/widget/model/musiclistmodel.h \
    view/widget/delegate/lyriclinedelegate.h \
    view/widget/delegate/musicitemdelegate_p.h \
    view/widget/delegate/musicitemdelegate.h \
    view/lyricwidget.h \
    view/widget/lyricview.h \
    view/widget/titeledit.h \
    core/util/threadpool.h \
    view/footerwidget.h \
    view/widget/waterprogress.h \
    view/loadwidget.h

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
    !system(python $$PWD/../tool/translate_generation.py $$PWD): error("Failed to generate translation")
}

TRANSLATIONS += $$PWD/translations/$${TARGET}.ts \
                $$PWD/translations/$${TARGET}_zh_CN.ts

include($$PWD/install.pri)
