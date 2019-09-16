TEMPLATE = subdirs
CONFIG = ordered

isEmpty(PREFIX){
    PREFIX = /usr
}

SUBDIRS +=  vendor
SUBDIRS +=  libdmusic \
            music-player \
            plugin \
