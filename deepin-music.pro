TEMPLATE = subdirs
CONFIG = ordered

include(config.pri)

SUBDIRS +=  vendor/src/vendor.pro

SUBDIRS +=  libdmusic \
            music-player \
            plugin \
