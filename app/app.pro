DEFINES += QT_MESSAGELOGCONTEXT

QT       += core gui widgets svg dbus multimedia multimediawidgets xml network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TEMPLATE    = app
TARGET      = deepin-music
CONFIG      += c++11 link_pkgconfig

unix{
    PKGCONFIG   += dtkbase dtkutil dtkwidget taglib
}

win32{
    DEFINES += STATIC_LIB
    DEFINES += TAGLIB_STATIC

    INCLUDEPATH += D:\Develop\deepin-tool-kit\dwidget
    INCLUDEPATH += D:\Develop\deepin-tool-kit\dwidget\widgets
    INCLUDEPATH += D:\Develop\deepin-tool-kit\dwidget\platforms\windows

    DEPENDPATH += D:\Develop\deepin-tool-kit\dwidget

    INCLUDEPATH += D:\Develop\deepin-tool-kit\dutil
    INCLUDEPATH += D:\Develop\deepin-tool-kit\dutil\dlog
    DEPENDPATH += D:\Develop\deepin-tool-kit\dutil

    INCLUDEPATH += D:\Develop\deepin-tool-kit\dbase
    DEPENDPATH += D:\Develop\deepin-tool-kit\dbase

    INCLUDEPATH += D:\Develop\Library\zlib\include
    INCLUDEPATH += "C:\Program Files (x86)\taglib\include\taglib"

    CONFIG(release, debug|release) {
        LIBS += -LD:\Develop\deepin-tool-kit\build\dwidget\release -ldtkwidget
        LIBS += -LD:\Develop\deepin-tool-kit\build\dutil\release -ldtkutil
        LIBS += -LD:\Develop\deepin-tool-kit\build\dbase\release -ldtkbase
        LIBS += -L"C:\Program Files (x86)\taglib\lib" -ltag
        LIBS += -L"C:\Program Files (x86)\taglib\lib" -ltag_c
        LIBS += -L"D:\Develop\Library\zlib-1.2.8\contrib\vstudio\vc11\x64\ZlibStatRelease" -lzlibstat
    }
    else:CONFIG(debug, debug|release){
        LIBS += -LD:\Develop\deepin-tool-kit\build\dwidget\debug -ldtkwidget
        LIBS += -LD:\Develop\deepin-tool-kit\build\dutil\debug -ldtkutil
        LIBS += -LD:\Develop\deepin-tool-kit\build\dbase\debug -ldtkbase
    }
}


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
    view/widget/cover.cpp \
    view/widget/menu.cpp \
    view/widget/settingsdialog.cpp \
    view/widget/modebuttom.cpp

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
    view/widget/cover.h \
    view/widget/menu.h \
    view/widget/settingsdialog.h \
    view/widget/modebuttom.h

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
