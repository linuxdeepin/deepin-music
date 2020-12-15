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
#include <DApplicationSettings>
#include <DExportedInterface>
#include <metadetector.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "config.h"

#include "core/player.h"
#include "core/musicsettings.h"
#include "core/util/threadpool.h"
#include "core/util/global.h"
#include "speech/exportedinterface.h"
#include "databaseservice.h"
#include "acobjectlist.h"

#include "mainframe.h"

using namespace Dtk::Core;
using namespace Dtk::Widget;

void createSpeechDbus();

bool checkOnly();

int main(int argc, char *argv[])
{
    qDebug() << "zy------main " << QTime::currentTime().toString("hh:mm:ss.zzz");
    setenv("PULSE_PROP_media.role", "music", 1);

    DApplication app(argc, argv);

#ifdef SNAP_APP
    DStandardPaths::setMode(DStandardPaths::Snap);
#endif

#if defined(STATIC_LIB)
    DWIDGET_INIT_RESOURCE();
    QCoreApplication::addLibraryPath(".");
#endif

    app.setAttribute(Qt::AA_UseHighDpiPixmaps);
    QAccessible::installFactory(accessibleFactory);
    app.setOrganizationName("deepin");
    app.setApplicationName("deepin-music");
    // Version Time
    app.setApplicationVersion(DApplication::buildVersion(VERSION));

    DLogManager::registerConsoleAppender();
    DLogManager::registerFileAppender();


    QCommandLineParser parser;
    parser.setApplicationDescription("Deepin music player.");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("file", "Music file path");
    parser.process(app);

    QIcon icon = QIcon::fromTheme("deepin-music");
    app.setProductIcon(icon);

    // handle open file
    QStringList OpenFilePaths;
    if (parser.positionalArguments().length() > 0) {
        OpenFilePaths = parser.positionalArguments();
    }

    MusicSettings::init();
    if (!OpenFilePaths.isEmpty()) {
        QStringList strList;
        for (QString str : OpenFilePaths) {
            QUrl url = QUrl(str);
            if (url.toLocalFile().isEmpty()) {
                strList.append(str);
            } else {
                strList.append(url.toLocalFile());
            }
        }
        if (strList.size() > 0) {
            DataBaseService::getInstance()->setFirstSong(strList.at(0));
            DataBaseService::getInstance()->importMedias("all", strList); //导入数据库
        }
    }
    app.loadTranslator();

    if (!app.setSingleInstance("deepinmusic") || !checkOnly()) {
        qDebug() << "another deepin music has started";
        for (auto curStr : parser.positionalArguments()) {
            if (!curStr.isEmpty()) {
                QUrl url = QUrl(curStr);//::fromLocalFile(fi.absoluteFilePath());
                while (true) {
                    QDBusInterface iface("org.mpris.MediaPlayer2.DeepinMusic",
                                         "/org/mpris/MediaPlayer2",
                                         "org.mpris.MediaPlayer2.Player",
                                         QDBusConnection::sessionBus());
                    if (iface.isValid()) {
                        if (url.toLocalFile().isEmpty()) {
                            iface.asyncCall("OpenUri", curStr);
                        } else {
                            iface.asyncCall("OpenUri", url.toLocalFile());
                        }
                        break;
                    }
                }
            }
        }

        /*-----show deepin-music----*/
        QDBusInterface iface("org.mpris.MediaPlayer2.DeepinMusic",
                             "/org/mpris/MediaPlayer2",
                             "org.mpris.MediaPlayer2",
                             QDBusConnection::sessionBus());
        if (iface.isValid()) {
            iface.asyncCall("Raise");
        }
        return 0;
    }

    DApplicationSettings saveTheme;
    /*---Player instance init---*/
    MainFrame mainframe;
    int musicCount = DataBaseService::getInstance()->allMusicInfosCount();
    mainframe.initUI(musicCount > 0 ? true : false);
    mainframe.show();
    mainframe.autoStartToPlay();
    createSpeechDbus();

//    app.connect(&app, &QApplication::lastWindowClosed,
//    &mainframe, [ & ]() {
//        auto quit = MusicSettings::value("base.close.is_close").toBool();
//        if (quit) {
//            music->quit();
//        }
//    });

    app.setQuitOnLastWindowClosed(false);
    return app.exec();
}

bool checkOnly()
{
    //single
    QString userName = QDir::homePath().section("/", -1, -1);
    std::string path = ("/home/" + userName + "/.cache/deepin/deepin-music/").toStdString();
    QDir tdir(path.c_str());
    if (!tdir.exists()) {
        bool ret =  tdir.mkpath(path.c_str());
        qDebug() << ret ;
    }

    path += "single";
    int fd = open(path.c_str(), O_WRONLY | O_CREAT, 0644);
    int flock = lockf(fd, F_TLOCK, 0);

    if (fd == -1) {
        perror("open lockfile/n");
        return false;
    }
    if (flock == -1) {
        perror("lock file error/n");
        return false;
    }
    return true;
}

void createSpeechDbus()
{
    QDBusConnection::sessionBus().registerService("com.deepin.musicSpeech");
    ExportedInterface *mSpeech = new ExportedInterface(nullptr);
    mSpeech->registerAction("1", "playmusic");
    mSpeech->registerAction("2", "play artist");
    mSpeech->registerAction("3", "play artist song");
    mSpeech->registerAction("4", "play faverite");
    mSpeech->registerAction("5", "play custom ");
    mSpeech->registerAction("6", "play radom");
    mSpeech->registerAction("11", "pause");
    mSpeech->registerAction("12", "stop");
    mSpeech->registerAction("13", "resume");
    mSpeech->registerAction("14", "previous");
    mSpeech->registerAction("15", "next");
    mSpeech->registerAction("21", "faverite");
    mSpeech->registerAction("22", "unfaverite");
    mSpeech->registerAction("23", "set play mode");
}
