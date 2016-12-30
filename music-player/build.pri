include($$PWD/../vendor/src/chinese2pinyin/chinese2pinyin.pri)
include($$PWD/../vendor/src/libcue/libcue.pri)

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
        LIBS += -L"C:\Program Files (x86)\taglib\lib" -ltag
        LIBS += -L"C:\Program Files (x86)\taglib\lib" -ltag_c
        LIBS += -L"D:\Develop\Library\zlib-1.2.8\contrib\vstudio\vc11\x64\ZlibStatRelease" -lzlibstat
    }


    INCLUDEPATH += D:\Develop\Library\icu\include
    LIBS += -LD:\Develop\Library\icu\lib64 -licuuc
    LIBS += -LD:\Develop\Library\icu\lib64 -licuin
}

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../libdmusic/release/ -ldmusic
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../libdmusic/debug/ -ldmusic
else:unix: LIBS += -L$$OUT_PWD/../libdmusic/ -ldmusic

INCLUDEPATH += $$PWD/../libdmusic
DEPENDPATH += $$PWD/../libdmusic

LIBS += -L$$BUILD_DIST/lib/ -ldbusextended-qt5

INCLUDEPATH += $$PWD/../vendor/src/dbusextended-qt/src
DEPENDPATH += $$PWD/../vendor/src/dbusextended-qt/src

LIBS += -L$$BUILD_DIST/lib/ -lmpris-qt5

INCLUDEPATH += $$PWD/../vendor/src/mpris-qt/src
DEPENDPATH += $$PWD/../vendor/src/mpris-qt/src
