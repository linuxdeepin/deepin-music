/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include <QtDBus>
#include <QDBusError>
#include <QDBusConnection>

#include <QIcon>
#include <DLog>
#include <DApplication>

#include <Mpris>
#include <MprisPlayer>
#include <MprisController>

#include "core/player.h"
#include "core/pluginmanager.h"
#include "musicapp.h"
#include "view/widget/thinwindow.h"

#include <thememanager.h>

using namespace Dtk::Util;
using namespace Dtk::Widget;

#include <QLabel>
#include <QLayout>

int main(int argc, char *argv[])
{
    DApplication::loadDXcbPlugin();

#if defined(STATIC_LIB)
    DWIDGET_INIT_RESOURCE();
#endif

    DApplication app(argc, argv);
    app.setOrganizationName("deepin");
    app.setApplicationName("deepin-music");
    app.setApplicationVersion("3.0");
    app.setTheme("light");
    ThemeManager::instance()->setTheme("light");

    DLogManager::registerConsoleAppender();
    DLogManager::registerFileAppender();

    app.loadTranslator();

#ifdef Q_OS_UNIX
    QDBusConnection conn = QDBusConnection::sessionBus();
    if (!conn.registerService("org.mpris.MediaPlayer2.deepinmusic")) {
        qDebug() << "registerService Failed, maybe service exist" << conn.lastError();
        return (0x0003);
    }

    if (!conn.registerObject("/",
                             "org.mpris.MediaPlayer2",
                             new Mpris(),
                             QDBusConnection::ExportAdaptors)) {
        qDebug() << "registerObject Failed" << conn.lastError();
        return (0x0002);
    }

    auto mprisPlayer =  new MprisPlayer();
#endif

    app.setWindowIcon(QIcon(":/common/image/deepin_music.svg"));

    PluginManager::instance()->init();
    // For Windows, must init media player in main thread!!!
    Player::instance()->init();
    MusicApp::instance()->init();

#ifdef Q_OS_UNIX
    MusicApp::instance()->initMpris(mprisPlayer);
#endif

    return app.exec();
}
