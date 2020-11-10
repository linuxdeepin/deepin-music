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
//#include <DApplication>
#include <DGuiApplicationHelper>
#include <DApplicationSettings>
#include <DExportedInterface>
#include <metadetector.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

//#include "config.h"

#include "view/mainframe.h"
#include "core/mediadatabase.h"
#include "core/medialibrary.h"
#include "core/metasearchservice.h"
#include "core/player.h"
#include "core/pluginmanager.h"
#include "core/musicsettings.h"
#include "core/util/threadpool.h"
#include "core/util/global.h"
#include "musicapp.h"
#include "speech/exportedinterface.h"

#include <DVtableHook>
#define protected public
#include <DApplication>
#undef protected

using namespace Dtk::Core;
using namespace Dtk::Widget;

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

bool checkOnly()
{
    //single
    QString userName = QDir::homePath().section("/", -1, -1);
    std::string path = ("/home/" + userName + "/.cache/deepin/deepin-music/").toStdString();
    QDir tdir(path.c_str());
    if (!tdir.exists()) {
        bool ret =  tdir.mkpath(path.c_str());
        MusicSettings::setOption("base.play.showFlag", 0);
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

int main(int argc, char *argv[])
{
    setenv("PULSE_PROP_media.role", "music", 1);

//    DApplication::loadDXcbPlugin();
#if (DTK_VERSION < DTK_VERSION_CHECK(5, 4, 0, 0))
    DApplication *app = new DApplication(argc, argv);
#else
    DApplication *app = DApplication::globalApplication(argc, argv);
#endif

#ifdef SNAP_APP
    DStandardPaths::setMode(DStandardPaths::Snap);
#endif

#if defined(STATIC_LIB)
    DWIDGET_INIT_RESOURCE();
    QCoreApplication::addLibraryPath(".");
#endif

    app->setAttribute(Qt::AA_UseHighDpiPixmaps);
    app->setOrganizationName("deepin");
    app->setApplicationName("deepin-music");
    // Version Time
#ifdef CVERSION
    QString verstr(CVERSION);
//    app->setApplicationVersion(DApplication::buildVersion(verstr));
    app->setApplicationVersion(verstr);
#endif
    DLogManager::registerConsoleAppender();
    DLogManager::registerFileAppender();

    QCommandLineParser parser;
    parser.setApplicationDescription("Deepin music player.");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("file", "Music file path");
    parser.process(*app);
    // handle open file
    QString toOpenFile;
    if (parser.positionalArguments().length() > 0) {
        toOpenFile = parser.positionalArguments().first();
    }

    app->loadTranslator();

    QIcon icon = QIcon::fromTheme("deepin-music");
    app->setProductIcon(icon);

    if (!app->setSingleInstance("deepinmusic") || !checkOnly()) {
        qDebug() << "another deepin music has started";
        for (auto curStr : parser.positionalArguments()) {
            if (!curStr.isEmpty()) {
                QFileInfo fi(curStr);
                QUrl url = QUrl::fromLocalFile(fi.absoluteFilePath());
                while (true) {
                    QDBusInterface iface("org.mpris.MediaPlayer2.DeepinMusic",
                                         "/org/mpris/MediaPlayer2",
                                         "org.mpris.MediaPlayer2.Player",
                                         QDBusConnection::sessionBus());
                    if (iface.isValid()) {
                        iface.asyncCall("OpenUri", url.toString());
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

    MusicSettings::init();
    DApplicationSettings saveTheme;

    /*---Player instance init---*/
    MainFrame mainframe;
    MusicApp *music = new MusicApp(&mainframe);

    auto showflag = MusicSettings::value("base.play.showFlag").toBool();
    music->initUI(showflag);

    QTimer::singleShot(20, nullptr, [ = ]() {
        music->initConnection(showflag);
        /*----创建语音dbus-----*/
        createSpeechDbus();
    });

    int count = parser.positionalArguments().length();
    if (count > 1) {
        QStringList files = parser.positionalArguments();
        files.removeFirst();
        music->onStartImport(files);
    }

    if (!toOpenFile.isEmpty()) {
        auto fi = QFileInfo(toOpenFile);
        auto url = QUrl::fromLocalFile(fi.absoluteFilePath());
        MusicSettings::setOption("base.play.to_open_uri", url.toString());
    }

    app->connect(app, &QApplication::lastWindowClosed,
    &mainframe, [ & ]() {
        auto quit = MusicSettings::value("base.close.is_close").toBool();
        if (quit) {
            music->quit();
        }
    });

    app->setQuitOnLastWindowClosed(false);

    QObject::connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
                     &mainframe, &MainFrame::slotTheme);
    Dtk::Core::DVtableHook::overrideVfptrFun(app, &DApplication::handleQuitAction,
                                             &mainframe, &MainFrame::closeFromMenu);

    return app->exec();
}
