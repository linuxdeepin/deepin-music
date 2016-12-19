include($$PWD/../vendor/src/chinese2pinyin/chinese2pinyin.pri)
include($$PWD/../vendor/src/libcue/libcue.pri)

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../vendor/src/dbusextended-qt/src/release/ -ldbusextended-qt5
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../vendor/src/dbusextended-qt/src/debug/ -ldbusextended-qt5
else:unix: LIBS += -L$$BUILD_DIST/lib/ -ldbusextended-qt5

INCLUDEPATH += $$PWD/../vendor/src/dbusextended-qt/src
DEPENDPATH += $$PWD/../vendor/src/dbusextended-qt/src

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../vendor/src/mpris-qt/src/release/ -lmpris-qt5
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../vendor/src/mpris-qt/src/debug/ -lmpris-qt5
else:unix: LIBS += -L$$BUILD_DIST/lib/ -lmpris-qt5

INCLUDEPATH += $$PWD/../vendor/src/mpris-qt/src
DEPENDPATH += $$PWD/../vendor/src/mpris-qt/src
