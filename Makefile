PREFIX = /usr/local

all:
	cd tools; ./generate_mo.py; cd ..

install:
	mkdir -p ${DESTDIR}${PREFIX}/bin
	mkdir -p ${DESTDIR}${PREFIX}/share/applications
	mkdir -p ${DESTDIR}${PREFIX}/share/deepin-music
	mkdir -p ${DESTDIR}${PREFIX}/share/icons/hicolor/scalable/apps
	cp -r src ${DESTDIR}${PREFIX}/share/deepin-music
	cp -r skin ${DESTDIR}${PREFIX}/share/deepin-music
	cp -r locale/mo ${DESTDIR}${PREFIX}/share/locale
	cp -r app_theme ${DESTDIR}${PREFIX}/share/deepin-music
	cp -r image ${DESTDIR}${PREFIX}/share/deepin-music
	cp -r plugins ${DESTDIR}${PREFIX}/share/deepin-music
	cp -r wizard ${DESTDIR}${PREFIX}/share/deepin-music
	cp image/deepin-music-player.svg ${DESTDIR}${PREFIX}/share/icons/hicolor/scalable/apps
	cp deepin-music-player.desktop ${DESTDIR}${PREFIX}/share/applications
	ln -sf ${PREFIX}/share/deepin-music/src/main.py ${DESTDIR}${PREFIX}/bin/deepin-music-player
