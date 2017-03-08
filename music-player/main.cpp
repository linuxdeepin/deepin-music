/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include <QIcon>
#include <QUrl>
#include <QThread>
#include <QProcess>
#include <QDBusInterface>
#include <QCommandLineParser>

#include <DLog>
#include <DApplication>

#include <MprisPlayer>

#include "core/player.h"
#include "core/pluginmanager.h"
#include "core/settings.h"
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

    QCommandLineParser parser;
    parser.setApplicationDescription("Deepin music player.");
    parser.addHelpOption();
    parser.addVersionOption();

    parser.addPositionalArgument("file", QCoreApplication::tr("Music file path"));
    parser.process(app);

    QString toOpenFile;
    if (1 == parser.positionalArguments().length()) {
        // import and playser
        toOpenFile = parser.positionalArguments().first();
    }
    qDebug() << app.arguments();

    app.setOrganizationName("deepin");
    app.setApplicationName("deepin-music");
    app.setApplicationVersion("3.0");

    app.setTheme("light");
    ThemeManager::instance()->setTheme("light");

    DLogManager::registerConsoleAppender();
    DLogManager::registerFileAppender();

    if (!app.setSingleInstance("deepinmusic")) {
        qDebug() << "another deppin music has started";
        if (!toOpenFile.isEmpty()) {
            QFileInfo fi(toOpenFile);
            QUrl url = QUrl::fromLocalFile(fi.absoluteFilePath());
            QDBusInterface iface("org.mpris.MediaPlayer2.deepinmusic",
                                 "/org/mpris/MediaPlayer2",
                                 "org.mpris.MediaPlayer2.Player",
                                 QDBusConnection::sessionBus());
            /*auto reply = */iface.asyncCall("OpenUri", url.toString());
        }
        exit(0);
    }

    app.loadTranslator();
    app.setApplicationDisplayName(DApplication::tr("Deepin Music"));

#ifdef Q_OS_UNIX
    auto serviceName = "deepinmusic";
    auto mprisPlayer =  new MprisPlayer();
    mprisPlayer->setServiceName(serviceName);
#endif

    app.setWindowIcon(QIcon(":/common/image/deepin-music.svg"));

    PluginManager::instance()->init();
    // For Windows, must init media player in main thread!!!
    Player::instance()->init();

#ifdef Q_OS_UNIX
    auto playThread = new QThread;
    Player::instance()->moveToThread(playThread);
    playThread->start();
    playThread->setPriority(QThread::HighestPriority);
#endif

    MusicApp::instance()->init();

#ifdef Q_OS_UNIX
    MusicApp::instance()->initMpris(mprisPlayer);
#endif

    if (!toOpenFile.isEmpty()) {
        auto fi = QFileInfo(toOpenFile);
        auto url = QUrl::fromLocalFile(fi.absoluteFilePath());
        Settings::instance()->setOption("base.play.to_open_uri", url.toString());
        Settings::instance()->sync();
    }

    return app.exec();
}
