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

#include "musicapp.h"

using namespace Dtk::Util;
using namespace Dtk::Widget;

int main(int argc, char *argv[])
{
    DApplication app(argc, argv);
    app.setOrganizationName("deepin");
    app.setApplicationName("deepin-music");
    app.setApplicationVersion("3.0");

    DLogManager::registerConsoleAppender();
    DLogManager::registerFileAppender();

    qDebug() << app.applicationVersion();
    app.loadTranslator();

    app.setTheme("light");
    app.setWindowIcon(QIcon("/usr/share/icons/deepin/apps/scalable/deepin-music.svg"));

    MusicApp::instance().init();
    MusicApp::instance().showPlayer();

    return app.exec();
}
