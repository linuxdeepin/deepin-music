include($$PWD/chinese2pinyin/chinese2pinyin.pri)

win32{
    DEFINES += STATIC_LIB
    DEFINES += TAGLIB_STATIC

    INCLUDEPATH += $$PWD
	include($$PWD/libcue/libcue.pri)

    LIBRARY_DIR=D:\Develop\Library\thrid

    DTK_INCLUDE = $${INCLUDE_INSTALL_DIR}\libdtk-1.0
    DTK_DEPEND  = $${INCLUDE_INSTALL_DIR}\libdtk-1.0
    DTK_LIBRARY = $${LIB_INSTALL_DIR}

    #dtkwidget
    INCLUDEPATH += $$DTK_INCLUDE\DWidget
    DEPENDPATH  += $$DTK_DEPEND\DWidget
    LIBS += -L$$DTK_LIBRARY -ldtkwidget
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

