/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include <QIcon>

#include <DLog>
#include <DApplication>

#include <MprisPlayer>

#include "core/player.h"
#include "core/pluginmanager.h"
#include "musicapp.h"

#include <thememanager.h>

using namespace Dtk::Util;
using namespace Dtk::Widget;

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
    auto serviceName = "deepinmusic";
    auto mprisPlayer =  new MprisPlayer();
    mprisPlayer->setServiceName(serviceName);
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
