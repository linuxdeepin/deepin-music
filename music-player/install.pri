isEmpty(PREFIX){
    PREFIX = /usr
}

target.path = $${PREFIX}/bin/

desktop_files.path = /usr/share/applications/
desktop_files.files = $$PWD/data/*.desktop

services.path = /usr/share/dbus-1/services
services.files = $$PWD/data/*.service

dman.path = /usr/share/dman/
dman.files = $$PWD/dman/*

translations.path = $${PREFIX}/share/$${TARGET}/translations
translations.files = $$PWD/translations/*.qm

INSTALLS += target desktop_files services dman translations

GitVersion = $$system(git rev-parse HEAD)
DEFINES += GIT_VERSION=\\\"$$GitVersion\\\"

