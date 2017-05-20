win32{
    DEFINES += STATIC_LIB
    DEFINES += TAGLIB_STATIC

    INCLUDEPATH += $$PWD
	include($$PWD/../vendor/src/libcue/libcue.pri)

    LIBRARY_DIR=D:\Develop\Library\thrid

    DTK_INCLUDE = $${INCLUDE_INSTALL_DIR}\libdtk-1.0
    DTK_DEPEND  = $${INCLUDE_INSTALL_DIR}\libdtk-1.0
    DTK_LIBRARY = $${LIB_INSTALL_DIR}

    #dtkwidget
    INCLUDEPATH += $$DTK_INCLUDE\DWidget
    DEPENDPATH  += $$DTK_DEPEND\DWidget
    LIBS += -L$$DTK_LIBRARY -ldtkwidget
    #dtksettingsview
    INCLUDEPATH += $$DTK_INCLUDE\DSettingsView
    DEPENDPATH  += $$DTK_DEPEND\DSettingsView
    LIBS += -L$$DTK_LIBRARY -ldtksettingsview
    #dtksettings
    INCLUDEPATH += $$DTK_INCLUDE\DSettings
    DEPENDPATH  += $$DTK_DEPEND\DSettings
    LIBS += -L$$DTK_LIBRARY -ldtksettings
    #dtkutil
    INCLUDEPATH += $$DTK_INCLUDE\DUtil
    DEPENDPATH  += $$DTK_DEPEND\DUtil
    LIBS += -L$$DTK_LIBRARY -ldtkutil
    #dtkbase
    INCLUDEPATH += $$DTK_INCLUDE\DBase
    DEPENDPATH  += $$DTK_DEPEND\DBase
    LIBS += -L$$DTK_LIBRARY -ldtkbase

    #taglib
    INCLUDEPATH += $$INCLUDE_INSTALL_DIR
	LIBS += -L$$LIB_INSTALL_DIR -ltag -ltag_c -lzlibstatic

    # icu
    ICU_ROOT = $${LIBRARY_DIR}\icu
    INCLUDEPATH += $$ICU_ROOT\include
    LIBS += -L$$ICU_ROOT\lib -licuuc
    LIBS += -L$$ICU_ROOT\lib -licuin

    # ffmpeg
    FFMPEG_ROOT = $${LIBRARY_DIR}\ffmpeg-3.2.2-win32-dev
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
