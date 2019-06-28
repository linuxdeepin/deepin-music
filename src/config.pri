DEFINES += QT_MESSAGELOGCONTEXT

ARCH = $$QMAKE_HOST.arch

message("Build arch:" $$ARCH)

isEqual(ARCH, x86_64) | isEqual(ARCH, i686) {
    CONFIG += ENABLE_LIBAV
}

PROJECT_ROOT = $$PWD
BUILD_DIST = $$PROJECT_ROOT/dist
APP_TARGET = deepin-music
