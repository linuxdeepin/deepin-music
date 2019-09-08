include($$PWD/../config.pri)
include($$PWD/build.pri)
include($$PWD/../interface/interface.pri)

load(dtk_translation)

QT       += core gui widgets svg multimedia multimediawidgets xml network sql dbus
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TEMPLATE    = app
TARGET      = $$APP_TARGET
CONFIG      += c++11 link_pkgconfig
DESTDIR     = $$BUILD_DIST/bin

unix{
    DEFINES += SUPPORT_INOTIFY
    QT += x11extras
    PKGCONFIG += icu-uc icu-i18n
    PKGCONFIG += x11 xext
    QT += dtkwidget

    CONFIG(ENABLE_LIBAV) {
        PKGCONFIG += libavutil libavformat libavcodec
    } else {
        DEFINES += DISABLE_LIBAV
    }
}

INCLUDEPATH += $$PWD/view/helper
INCLUDEPATH += $$PWD/core

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
    view/widget/pushbutton.cpp \
    view/widget/searchedit.cpp \
    view/widget/searchresult.cpp \
    view/widget/slider.cpp \
    view/widget/label.cpp \
    view/widget/soundvolume.cpp \
    view/widget/filter.cpp \
    core/player.cpp \
    view/widget/titlebarwidget.cpp \
    view/widget/ddropdown.cpp \
    view/widget/searchmetalist.cpp \
    view/widget/searchmetaitem.cpp \
    core/medialibrary.cpp \
    core/util/global.cpp \
    core/metasearchservice.cpp \
    core/pluginmanager.cpp \
    view/widget/listview.cpp \
    view/mainframe.cpp \
    view/importwidget.cpp \
    musicapp.cpp \
    main.cpp \
    view/widget/delegate/lyriclinedelegate.cpp \
    view/lyricwidget.cpp \
    view/widget/lyricview.cpp \
    view/widget/settingsdialog.cpp \
    core/util/threadpool.cpp \
    view/footerwidget.cpp \
    view/loadwidget.cpp \
    view/widget/closeconfirmdialog.cpp \
    core/musicsettings.cpp \
    view/playlistwidget.cpp \
    view/widget/playlistview.cpp \
    view/widget/delegate/playitemdelegate.cpp \
    view/widget/model/playlistmodel.cpp \
    view/musiclistwidget.cpp \
    view/widget/musiclistview.cpp \
    view/widget/musiclistitem.cpp \
    view/musiclistdatawidget.cpp \
    view/widget/musiclistdataview.cpp \
    view/widget/model/musiclistdatamodel.cpp \
    view/widget/delegate/musiclistdatadelegate.cpp \
    view/widget/musicimagebutton.cpp \
    view/widget/musicpixmapbutton.cpp \
    view/widget/musiclistviewitem.cpp

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
    view/widget/pushbutton.h \
    view/widget/searchedit.h \
    view/widget/searchresult.h \
    view/widget/slider.h \
    view/widget/label.h \
    view/importwidget.h \
    view/widget/soundvolume.h \
    view/widget/filter.h \
    core/player.h \
    presenter/presenter_p.h \
    view/widget/titlebarwidget.h \
    view/widget/ddropdown.h \
    view/widget/searchmetalist.h \
    view/widget/searchmetaitem.h \
    core/medialibrary.h \
    core/util/global.h \
    core/metasearchservice.h \
    core/pluginmanager.h \
    view/mainframe.h \
    view/widget/listview.h \
    view/widget/delegate/lyriclinedelegate.h \
    view/lyricwidget.h \
    view/widget/lyricview.h \
    core/util/threadpool.h \
    view/footerwidget.h \
    view/loadwidget.h \
    view/widget/closeconfirmdialog.h \
    core/musicsettings.h \
    view/playlistwidget.h \
    view/widget/delegate/playitemdelegate.h \
    view/widget/model/playlistmodel.h \
    view/widget/playlistview.h \
    view/widget/delegate/playitemdelegate_p.h \
    view/musiclistwidget.h \
    view/widget/musiclistview.h \
    view/widget/musiclistitem.h \
    view/musiclistdatawidget.h \
    view/widget/musiclistdataview.h \
    view/widget/model/musiclistdatamodel.h \
    view/widget/delegate/musiclistdatadelegate.h \
    view/widget/musicimagebutton.h \
    view/widget/musicpixmapbutton.h \
    view/widget/musiclistviewitem.h

RESOURCES += \
    resource/theme/theme.qrc \
    config.qrc \
    resource/resource.qrc

unix{
SOURCES += \
    core/util/inotifyengine.cpp

HEADERS += \
    core/util/inotifyengine.h
}

TRANSLATIONS += $$PWD/translations/$${TARGET}.ts \
                $$PWD/translations/$${TARGET}_zh_CN.ts

include($$PWD/install.pri)
