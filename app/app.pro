DEFINES += QT_MESSAGELOGCONTEXT

include($$PWD/../vendor/src/chinese2pinyin/chinese2pinyin.pri)
include($$PWD/../vendor/src/libcue/libcue.pri)

QT       += core gui widgets svg dbus multimedia multimediawidgets xml network sql
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TEMPLATE    = app
TARGET      = deepin-music
CONFIG      += c++11 link_pkgconfig

unix{
    PKGCONFIG += dtkbase dtkutil dtkwidget taglib icu-uc icu-i18n
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


    INCLUDEPATH += D:\Develop\Library\icu\include
    LIBS += -LD:\Develop\Library\icu\lib64 -licuuc
    LIBS += -LD:\Develop\Library\icu\lib64 -licuin
}


SOURCES += \
    main.cpp \
    view/playerframe.cpp \
    view/titlebar.cpp \
    view/footer.cpp \
    view/musiclistview.cpp \
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
    core/player.cpp \
    view/widget/slider.cpp \
    view/widget/infodialog.cpp \
    view/widget/cover.cpp \
    view/widget/menu.cpp \
    view/widget/settingsdialog.cpp \
    view/widget/modebuttom.cpp \
    core/lyricservice.cpp \
    core/mediadatabase.cpp \
    view/helper/widgethellper.cpp \
    view/widget/searchresult.cpp \
    view/widget/searchedit.cpp \
    view/widget/pushbutton.cpp \
    core/mediadatabasewriter.cpp \
    core/util/pinyin.cpp \
    view/widget/musicitemdelegate.cpp \
    core/util/lyric.cpp \
    core/util/cueparser.cpp \
    view/widget/lyriclinedelegate.cpp \
    core/util/musicmeta.cpp \
    view/widget/clickablelabel.cpp \
    core/util/icu.cpp \
#    core/util/filesystemwatcher.cpp \
    core/util/inotifyengine.cpp

HEADERS += \
    view/playerframe.h \
    view/titlebar.h \
    view/footer.h \
    view/musiclistview.h \
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
    core/player.h \
    view/widget/slider.h \
    view/widget/infodialog.h \
    view/widget/cover.h \
    view/widget/menu.h \
    view/widget/settingsdialog.h \
    view/widget/modebuttom.h \
    core/lyricservice.h \
    core/mediadatabase.h \
    view/helper/widgethellper.h \
    view/widget/searchresult.h \
    view/widget/searchedit.h \
    view/widget/pushbutton.h \
    core/mediadatabasewriter.h \
    core/util/pinyin.h \
    view/widget/musicitemdelegate.h \
    core/util/lyric.h \
    core/util/cueparser.h \
    view/widget/lyriclinedelegate.h \
    core/util/musicmeta.h \
    view/widget/clickablelabel.h \
    core/util/icu.h \
#    core/util/filesystemwatcher.h \
    core/util/inotifyengine.h

RESOURCES += \
    resource/theme/theme.qrc \
    resource/resource.qrc

# Automating generation .qm files from .ts files
system($$PWD/translate_generation.sh)

TRANSLATIONS += $$PWD/translations/$${TARGET}.ts \
                $$PWD/translations/$${TARGET}_zh_CN.ts

include($$PWD/install.pri)

#unix:!macx: LIBS += -L$$OUT_PWD/../vendor/src/mpris/ -lmpris

#INCLUDEPATH += $$PWD/../vendor/src/mpris
#DEPENDPATH += $$PWD/../vendor/src/mpris
