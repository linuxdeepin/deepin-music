/*
 * Copyright (C) 2016 ~ 2018 Wuhan Deepin Technology Co., Ltd.
 *
 * Author:     Iceyer <me@iceyer.net>
 *
 * Maintainer: Iceyer <me@iceyer.net>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QUrl>
#include <QIcon>
#include <QDBusInterface>
#include <QDBusPendingCall>
#include <QCommandLineParser>
#include <QProcessEnvironment>

#include <DLog>
#include <DStandardPaths>
#include <DApplication>

#include <metadetector.h>

#include "view/mainframe.h"
#include "core/mediadatabase.h"
#include "core/medialibrary.h"
#include "core/metasearchservice.h"
#include "core/player.h"
#include "core/pluginmanager.h"
#include "core/musicsettings.h"
#include "core/util/threadpool.h"
#include "musicapp.h"

using namespace Dtk::Core;
using namespace Dtk::Widget;

int main(int argc, char *argv[])
{
#ifdef SNAP_APP
    DStandardPaths::setMode(DStandardPaths::Snap);
#endif

#if defined(STATIC_LIB)
    DWIDGET_INIT_RESOURCE();
    QCoreApplication::addLibraryPath(".");
#endif
    DApplication::loadDXcbPlugin();

    DApplication app(argc, argv);
    app.setAttribute(Qt::AA_UseHighDpiPixmaps);
    app.setAttribute(Qt::AA_EnableHighDpiScaling);
    app.setOrganizationName("deepin");
    app.setApplicationName("deepin-music");
    //app.setApplicationVersion(DApplication::buildVersion("3.1"));
    const QDate buildDate = QLocale( QLocale::English ).toDate( QString(__DATE__).replace("  ", " 0"), "MMM dd yyyy");
    QString t_date = buildDate.toString("MMdd");
    // Version Time
    app.setApplicationVersion(DApplication::buildVersion(t_date));
    //app.setStyle("chameleon");


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

    app.loadTranslator();

    // !!! MUST setApplicationDisplayName before DMainWindow create
    app.setApplicationDisplayName(QObject::tr("Deepin Music"));
    app.setWindowIcon(QIcon(":/common/image/deepin-music.svg"));

    if (!app.setSingleInstance("deepinmusic")) {
        qDebug() << "another deepin music has started";
        if (!toOpenFile.isEmpty()) {
            QFileInfo fi(toOpenFile);
            QUrl url = QUrl::fromLocalFile(fi.absoluteFilePath());
            QDBusInterface iface("org.mpris.MediaPlayer2.DeepinMusic",
                                 "/org/mpris/MediaPlayer2",
                                 "org.mpris.MediaPlayer2.Player",
                                 QDBusConnection::sessionBus());
            iface.asyncCall("OpenUri", url.toString());
        }

        // show deepin-music
        QDBusInterface iface("org.mpris.MediaPlayer2.DeepinMusic",
                             "/org/mpris/MediaPlayer2",
                             "org.mpris.MediaPlayer2",
                             QDBusConnection::sessionBus());
        iface.asyncCall("Raise");
        exit(0);
    }

    MusicSettings::init();

    // set theme
    qDebug() << "TRACE:" << "set theme";
    MusicSettings::setOption("base.play.theme", "light");
    auto theme = MusicSettings::value("base.play.theme").toString();

    // DMainWindow must create on main function, so it can deconstruction before QApplication
    MainFrame mainframe;
    MusicApp *music = new MusicApp(&mainframe);
    music->initUI();

    Player::instance()->init();

    music->initConnection();

    if (!toOpenFile.isEmpty()) {
        auto fi = QFileInfo(toOpenFile);
        auto url = QUrl::fromLocalFile(fi.absoluteFilePath());
        MusicSettings::setOption("base.play.to_open_uri", url.toString());
    }

    app.connect(&app, &QApplication::lastWindowClosed,
    &mainframe, [ & ]() {
        auto quit = MusicSettings::value("base.close.close_action").toInt();
        if (quit == 1) {
            music->quit();
        }
    });

    //app.setTheme("light");
    app.setQuitOnLastWindowClosed(true);
    //app.setStyle("chameleon");

    return app.exec();
}
