/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include <QUrl>
#include <QIcon>
#include <QDBusInterface>
#include <QDBusPendingCall>
#include <QCommandLineParser>

#include <DLog>
#include <DApplication>

#include "view/mainframe.h"
#include "core/mediadatabase.h"
#include "core/medialibrary.h"
#include "core/metasearchservice.h"
#include "core/player.h"
#include "core/pluginmanager.h"
#include "core/settings.h"
#include "core/util/threadpool.h"
#include "thememanager.h"
#include "musicapp.h"

#ifdef Q_OS_LINUX
#include <unistd.h>
#endif
#ifdef Q_OS_WIN
//#include <QFontDatabase>
#endif

using namespace Dtk::Util;
using namespace Dtk::Widget;

int main(int argc, char *argv[])
{
#if defined(STATIC_LIB)
    DWIDGET_INIT_RESOURCE();
    QCoreApplication::addLibraryPath(".");
#endif
    DApplication::loadDXcbPlugin();


    DApplication app(argc, argv);
    app.setOrganizationName("deepin");
    app.setApplicationName("deepin-music");
    app.setApplicationVersion("3.0");

    DLogManager::registerConsoleAppender();
    DLogManager::registerFileAppender();

    QCommandLineParser parser;
    parser.setApplicationDescription("Deepin music player.");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("file", "Music file path");
    parser.process(app);

    // handle open file
    QString toOpenFile;
    if (1 == parser.positionalArguments().length()) {
        toOpenFile = parser.positionalArguments().first();
    }

    if (!app.setSingleInstance("deepinmusic")) {
        qDebug() << "another deppin music has started";
        if (!toOpenFile.isEmpty()) {
            QFileInfo fi(toOpenFile);
            QUrl url = QUrl::fromLocalFile(fi.absoluteFilePath());
            QDBusInterface iface("org.mpris.MediaPlayer2.DeepinMusic",
                                 "/org/mpris/MediaPlayer2",
                                 "org.mpris.MediaPlayer2.Player",
                                 QDBusConnection::sessionBus());
            iface.asyncCall("OpenUri", url.toString());
        }
        exit(0);
    }

    app.loadTranslator();

    app.setWindowIcon(QIcon(":/common/image/deepin-music.svg"));
    app.setApplicationDisplayName(QObject::tr("Deepin Music"));

    AppSettings::instance()->init();
    if (!toOpenFile.isEmpty()) {
        auto fi = QFileInfo(toOpenFile);
        auto url = QUrl::fromLocalFile(fi.absoluteFilePath());
        AppSettings::instance()->setOption("base.play.to_open_uri", url.toString());
    }

    // set theme
    qDebug() << "TRACE:" << "set theme";
    auto theme = AppSettings::instance()->value("base.play.theme").toString();
    auto themePrefix = AppSettings::instance()->value("base.play.theme_prefix").toString();
    ThemeManager::instance()->setPrefix(themePrefix);
    ThemeManager::instance()->setTheme(theme);
    // DMainWindow must create on main function, so it can deconstruction before QApplication
    MainFrame mainframe;
    MusicApp *music = new MusicApp(&mainframe);
    music->init();

    app.connect(&app, &QApplication::lastWindowClosed,
    &app, [ = ]() {
        qDebug() << "sync config start";
        AppSettings::instance()->sync();
#ifdef Q_OS_LINUX
        sync();
#endif
        qDebug() << "sync config finish, app exit";
    });

    return app.exec();
}
