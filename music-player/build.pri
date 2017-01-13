win32{
    DEFINES += STATIC_LIB

    # Deepin tool kit
    DTK_INCLUDE = D:\Develop\Library\deepin-tool-kit
    DTK_DEPEND  = D:\Develop\Library\deepin-tool-kit
    DTK_LIBRARY = D:\Develop\Library\deepin-tool-kit\build

    INCLUDEPATH += $$DTK_INCLUDE\dwidget
    INCLUDEPATH += $$DTK_INCLUDE\dwidget\widgets
    INCLUDEPATH += $$DTK_INCLUDE\dwidget\platforms\windows

    DEPENDPATH  += $$DTK_DEPEND\dwidget

    INCLUDEPATH += $$DTK_INCLUDE\dutil
    INCLUDEPATH += $$DTK_INCLUDE\dutil\dlog
    DEPENDPATH  += $$DTK_DEPEND\dutil

    INCLUDEPATH += $$DTK_INCLUDE\dbase
    DEPENDPATH  += $$DTK_DEPEND\dbase


    CONFIG(release, debug|release) {
        LIBS += -L$$DTK_LIBRARY\dwidget\release -ldtkwidget
        LIBS += -L$$DTK_LIBRARY\dutil\release -ldtkutil
        LIBS += -L$$DTK_LIBRARY\dbase\release -ldtkbase
    }
    else:CONFIG(debug, debug|release){
        LIBS += -L$$DTK_LIBRARY\dwidget\debug -ldtkwidget
        LIBS += -L$$DTK_LIBRARY\dutil\debug -ldtkutil
        LIBS += -L$$DTK_LIBRARY\dbase\debug -ldtkbase
    }

    # icu
    ICU_ROOT = D:\Develop\Library\icu
    INCLUDEPATH += $$ICU_ROOT\include
    LIBS += -L$$ICU_ROOT\lib -licuuc
    LIBS += -L$$ICU_ROOT\lib -licuin

    # ffmpeg
    FFMPEG_ROOT = D:\Develop\Library\ffmpeg-3.2.2-win32-dev
    INCLUDEPATH += $$FFMPEG_ROOT\include
    LIBS += -L$$FFMPEG_ROOT\lib -lavcodec
    LIBS += -L$$FFMPEG_ROOT\lib -lavformat
    LIBS += -L$$FFMPEG_ROOT\lib -lavutil

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
