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
#include <DGuiApplicationHelper>
#include <DApplicationSettings>
#include <DExportedInterface>
#include <metadetector.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

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
    QString userName = QDir::homePath().section("/", -1, -1);
    std::string path = ("/home/" + userName + "/.cache/deepin/deepin-music/single").toStdString();
    int fd = open (path.c_str(), O_WRONLY | O_CREAT, 0644);
    int flock = lockf(fd, F_TLOCK, 0 );
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
//    DApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
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
//    app.setAttribute(Qt::AA_EnableHighDpiScaling);
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
//    createSpeechDbus();//创建语音dbus
    // handle open file
    QString toOpenFile;
    if (parser.positionalArguments().length() > 0) {
        toOpenFile = parser.positionalArguments().first();
    }

    app.loadTranslator();

    /*
     MUST setApplicationDisplayName before DMainWindow create
     app.setApplicationDisplayName(QObject::tr("Music"));
     app.setWindowIcon(QIcon(":/common/image/deepin-music.svg"));
    */

    QIcon icon = QIcon::fromTheme("deepin-music");
    app.setProductIcon(icon);

    if (!app.setSingleInstance("deepinmusic") || !checkOnly()) {
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
        exit(0);
    }
    MusicSettings::init();

    DApplicationSettings saveTheme;

    /*-DMainWindow must create on main function, so it can deconstruction before QApplication-*/

    MainFrame mainframe;
    MusicApp *music = new MusicApp(&mainframe);
    music->initUI();
    /*---Player instance init---*/

    int count = parser.positionalArguments().length();
    if (count > 1) {
        QStringList files = parser.positionalArguments();
        files.removeFirst();
        music->onStartImport(files);
    }
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

    app.setQuitOnLastWindowClosed(false);

    QObject::connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
                     &mainframe, &MainFrame::slotTheme);

    return app.exec();
}
