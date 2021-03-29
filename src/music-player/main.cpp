/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     ZouYa <zouya@uniontech.com>
 *
 * Maintainer: WangYu <wangyu@uniontech.com>
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
#include <QDBusReply>
#include <metadetector.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "config.h"

#include "core/player.h"
#include "core/musicsettings.h"
#include "core/util/global.h"
#include "databaseservice.h"
#include "acobjectlist.h"
#include "speechCenter.h"
#include <functional>
#include "mainframe.h"
#include "speechexportbus.h"

using namespace Dtk::Core;
using namespace Dtk::Widget;

void createSpeechDbus();

bool checkOnly();

int main(int argc, char *argv[])
{
    setenv("PULSE_PROP_media.role", "music", 1);

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
    QAccessible::installFactory(accessibleFactory);
    app->setOrganizationName("deepin");
    app->setApplicationName("deepin-music");
    // Version Time
    app->setApplicationVersion(DApplication::buildVersion(VERSION));

    DLogManager::registerConsoleAppender();
    DLogManager::registerFileAppender();


    QCommandLineParser parser;
    parser.setApplicationDescription("Deepin music player.");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("file", "Music file path");
    parser.process(*app);

    QIcon icon = QIcon::fromTheme("deepin-music");
    app->setProductIcon(icon);

    // handle open file
    QStringList OpenFilePaths;
    if (parser.positionalArguments().length() > 0) {
        OpenFilePaths = parser.positionalArguments();
    }

    app->loadTranslator();
    MusicSettings::init();
    Player::getInstance();
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
        // 添加应用唯一性判断
        if (strList.size() > 0 && checkOnly()) {
            DataBaseService::getInstance()->setFirstSong(strList.at(0));
            DataBaseService::getInstance()->importMedias("all", strList); //导入数据库
        }
    }

    if (!app->setSingleInstance("deepinmusic") || !checkOnly()) {
        qDebug() << "another deepin music has started";
        QDBusInterface speechbus("org.mpris.MediaPlayer2.DeepinMusic",
                                 "/org/mpris/speech",
                                 "com.deepin.speech",
                                 QDBusConnection::sessionBus());
        if (speechbus.isValid()) {
            QVariant mediaMeta;
            mediaMeta.setValue(parser.positionalArguments());
            QDBusReply<QVariant> msg  = speechbus.call(QString("invokeStrlist"), "OpenUris", mediaMeta); //0 function  ,1 params
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

    app->setQuitOnLastWindowClosed(false);
    return app->exec();
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
    SpeechExportBus *mSpeech = new SpeechExportBus(SpeechCenter::getInstance());
    // 'playMusic','红颜' 显示搜索界面
    // 'playMusic',''       显示所有音乐界面，随机播放
    mSpeech->registerAction("playMusic", "play Music",
                            std::bind(&SpeechCenter::playMusic, SpeechCenter::getInstance(), std::placeholders::_1));
    // 'playArtist','华晨宇'
    mSpeech->registerAction("playArtist", "play Artist",
                            std::bind(&SpeechCenter::playArtist, SpeechCenter::getInstance(), std::placeholders::_1));
    // 'playArtistMusic','华晨宇:齐天'
    mSpeech->registerAction("playArtistMusic", "play Artist Music",
                            std::bind(&SpeechCenter::playArtistMusic, SpeechCenter::getInstance(), std::placeholders::_1));
    // 'playAlbum','历久尝新'
    mSpeech->registerAction("playAlbum", "play Album",
                            std::bind(&SpeechCenter::playAlbum, SpeechCenter::getInstance(), std::placeholders::_1));
    // 'playFaverite','fav'
    mSpeech->registerAction("playFaverite", "play Faverite",
                            std::bind(&SpeechCenter::playFaverite, SpeechCenter::getInstance(), std::placeholders::_1));
    // 'playSonglist','123'  歌单名称
    mSpeech->registerAction("playSonglist", "play Songlist",
                            std::bind(&SpeechCenter::playSonglist, SpeechCenter::getInstance(), std::placeholders::_1));
    // 'pause',''
    mSpeech->registerAction("pause", "pause",
                            std::bind(&SpeechCenter::pause, SpeechCenter::getInstance(), std::placeholders::_1));
    // 'resume',''
    mSpeech->registerAction("resume", "resume",
                            std::bind(&SpeechCenter::resume, SpeechCenter::getInstance(), std::placeholders::_1));
    // 'stop',''
    mSpeech->registerAction("stop", "stop",
                            std::bind(&SpeechCenter::stop, SpeechCenter::getInstance(), std::placeholders::_1));
    // 'pre',''
    mSpeech->registerAction("pre", "pre",
                            std::bind(&SpeechCenter::pre, SpeechCenter::getInstance(), std::placeholders::_1));
    // 'next',''
    mSpeech->registerAction("next", "next",
                            std::bind(&SpeechCenter::next, SpeechCenter::getInstance(), std::placeholders::_1));
    // 'playIndex',''    指定播放第几首
    mSpeech->registerAction("playIndex", "play Index",
                            std::bind(&SpeechCenter::playIndex, SpeechCenter::getInstance(), std::placeholders::_1));
    // 'addFaverite',''
    mSpeech->registerAction("addFaverite", "add Faverite",
                            std::bind(&SpeechCenter::addFaverite, SpeechCenter::getInstance(), std::placeholders::_1));
    // 'removeFaverite',''
    mSpeech->registerAction("removeFaverite", "remove Faverite",
                            std::bind(&SpeechCenter::removeFaverite, SpeechCenter::getInstance(), std::placeholders::_1));
    // 'setMode','0' 列表循环  'setMode','1' 单曲循环  'setMode','2' 随机
    mSpeech->registerAction("setMode", "set Mode",
                            std::bind(&SpeechCenter::setMode, SpeechCenter::getInstance(), std::placeholders::_1));
    // dbus导入音乐文件
    mSpeech->registerQStringListAction("OpenUris", "OpenUris",
                                       std::bind(&SpeechCenter::OpenUris, SpeechCenter::getInstance(), std::placeholders::_1));
}
