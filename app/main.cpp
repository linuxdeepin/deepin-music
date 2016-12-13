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

#include "core/player.h"
#include "musicapp.h"

using namespace Dtk::Util;
using namespace Dtk::Widget;

int main(int argc, char *argv[])
{
#if defined(STATIC_LIB)
    DWIDGET_INIT_RESOURCE();
#endif

    if (DApplication::isDXcbPlatform()) {
        DApplication::loadDXcbPlugin();
    }

    DApplication app(argc, argv);
    app.setOrganizationName("deepin");
    app.setApplicationName("deepin-music");
    app.setApplicationVersion("3.0");

    DLogManager::registerConsoleAppender();
    DLogManager::registerFileAppender();

    app.loadTranslator();

    app.setTheme("light");
    app.setWindowIcon(QIcon(":/image/deepin-music.svg"));

    // For Windows, must init media player in main thread!!!
    Player::instance()->init();
    MusicApp::instance()->init();

    return app.exec();
}
