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

#include <QtTest>
#include <QCoreApplication>
#include <QTest>

#include <QObject>
#include <QScopedPointer>
#include <util/singleton.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include <AbstractAppender.h>
#include <DApplication>

#include "core/musicsettings.h"
#include "core/util/global.h"
#include "application.h"
#include "mainframe.h"
#include "player.h"
#include "core/vlc/vlcdynamicinstance.h"
#ifndef SYSTEM_MIPS
#include <sanitizer/asan_interface.h>
#endif
#include "speechexportbus.h"
#include "speechCenter.h"

using namespace Dtk::Core;
using namespace Dtk::Widget;

static bool checkOnly();

void createSpeechDbus()
{
    SpeechExportBus mSpeech(SpeechCenter::getInstance());
    // 'playMusic','红颜' 显示搜索界面
    // 'playMusic',''       显示所有音乐界面，随机播放
    mSpeech.registerAction("playMusic", "play Music",
                           std::bind(&SpeechCenter::playMusic, SpeechCenter::getInstance(), std::placeholders::_1));
    // 'playArtist','xxx'
    mSpeech.registerAction("playArtist", "play Artist",
                           std::bind(&SpeechCenter::playArtist, SpeechCenter::getInstance(), std::placeholders::_1));
    // 'playArtistMusic','xxx:xx'
    mSpeech.registerAction("playArtistMusic", "play Artist Music",
                           std::bind(&SpeechCenter::playArtistMusic, SpeechCenter::getInstance(), std::placeholders::_1));
    // 'playAlbum','xxx'
    mSpeech.registerAction("playAlbum", "play Album",
                           std::bind(&SpeechCenter::playAlbum, SpeechCenter::getInstance(), std::placeholders::_1));
    // 'playFaverite','fav'
    mSpeech.registerAction("playFaverite", "play Faverite",
                           std::bind(&SpeechCenter::playFaverite, SpeechCenter::getInstance(), std::placeholders::_1));
    // 'playSonglist','123'  歌单名称
    mSpeech.registerAction("playSonglist", "play Songlist",
                           std::bind(&SpeechCenter::playSonglist, SpeechCenter::getInstance(), std::placeholders::_1));
    // 'pause',''
    mSpeech.registerAction("pause", "pause",
                           std::bind(&SpeechCenter::pause, SpeechCenter::getInstance(), std::placeholders::_1));
    // 'resume',''
    mSpeech.registerAction("resume", "resume",
                           std::bind(&SpeechCenter::resume, SpeechCenter::getInstance(), std::placeholders::_1));
    // 'stop',''
    mSpeech.registerAction("stop", "stop",
                           std::bind(&SpeechCenter::stop, SpeechCenter::getInstance(), std::placeholders::_1));
    // 'pre',''
    mSpeech.registerAction("pre", "pre",
                           std::bind(&SpeechCenter::pre, SpeechCenter::getInstance(), std::placeholders::_1));
    // 'next',''
    mSpeech.registerAction("next", "next",
                           std::bind(&SpeechCenter::next, SpeechCenter::getInstance(), std::placeholders::_1));
    // 'playIndex',''    指定播放第几首
    mSpeech.registerAction("playIndex", "play Index",
                           std::bind(&SpeechCenter::playIndex, SpeechCenter::getInstance(), std::placeholders::_1));
    // 'addFaverite',''
    mSpeech.registerAction("addFaverite", "add Faverite",
                           std::bind(&SpeechCenter::addFaverite, SpeechCenter::getInstance(), std::placeholders::_1));
    // 'removeFaverite',''
    mSpeech.registerAction("removeFaverite", "remove Faverite",
                           std::bind(&SpeechCenter::removeFaverite, SpeechCenter::getInstance(), std::placeholders::_1));
    // 'setMode','0' 列表循环  'setMode','1' 单曲循环  'setMode','2' 随机
    mSpeech.registerAction("setMode", "set Mode",
                           std::bind(&SpeechCenter::setMode, SpeechCenter::getInstance(), std::placeholders::_1));
    // dbus导入音乐文件
    mSpeech.registerQStringListAction("OpenUris", "OpenUris",
                                      std::bind(&SpeechCenter::OpenUris, SpeechCenter::getInstance(), std::placeholders::_1));
}

#define QMYTEST_MAIN(TestObject) \
    QT_BEGIN_NAMESPACE \
    QTEST_ADD_GPU_BLACKLIST_SUPPORT_DEFS \
    QT_END_NAMESPACE \
    int main(int argc, char *argv[]) \
    { \
        DApplication app(argc, argv); \
        app.setAttribute(Qt::AA_UseHighDpiPixmaps); \
        app.setOrganizationName("deepin"); \
        app.setApplicationName("deepin-music"); \
        app.loadTranslator(); \
        checkOnly(); \
        MusicSettings::init(); \
        Global::initPlaybackEngineType(); \
        Global::checkBoardVendorType(); \
        Global::setBoardVendorType(true); \
        MainFrame mainframe; \
        auto showflag = MusicSettings::value("base.play.showFlag").toBool(); \
        mainframe.initUI(showflag); \
        mainframe.show(); \
        mainframe.autoStartToPlay(); \
        createSpeechDbus(); \
        Application::getInstance()->setMainWindow(&mainframe); \
        app.setSingleInstance("deepinmusic"); \
        QTEST_DISABLE_KEYPAD_NAVIGATION \
        QTEST_ADD_GPU_BLACKLIST_SUPPORT \
        TestObject tc; \
        QTEST_SET_MAIN_SOURCE_PATH \
        int status = QTest::qExec(&tc, argc, argv); \
        Player::getInstance()->releasePlayer(); \
        return status;\
    } \

class QTestMain : public QObject
{
    Q_OBJECT

public:
    QTestMain();
    ~QTestMain();

private slots:
    void testGTest();

    void testQString_data();
    void testQString();

    void testFloat_data();
    void testFloat();

    void testGui_data();
};

QTestMain::QTestMain()
{

}

QTestMain::~QTestMain()
{

}

void QTestMain::testGTest()
{
    testing::GTEST_FLAG(output) = "xml:./report/report_deepin-music.xml";
    int argc = 1;
    const auto arg0 = "dummy";
    char *argv0 = const_cast<char *>(arg0);
    char **argv = &argv0;
    testing::InitGoogleTest(&argc, argv);
    int ret = RUN_ALL_TESTS();
    // 内存分析屏蔽mips
#ifndef SYSTEM_MIPS
    __sanitizer_set_report_path("./asan_deepin-music.log");//内存检测输出
#endif
    Q_UNUSED(ret)
}

void QTestMain::testQString_data()
{
    QTest::addColumn<QString>("string");
    QTest::addColumn<QString>("result");
    QTest::newRow("lower") << "hello" << "HELLO";
    QTest::newRow("mix") << "heLLo" << "HELLO";
    QTest::newRow("upper") << "HELLO" << "HELLO";
}

void QTestMain::testQString()
{
    QFETCH(QString, string);
    QFETCH(QString, result);
    QCOMPARE(string.toUpper(), result);
    QBENCHMARK{
        Q_UNUSED(string.toUpper())
    }

}

void QTestMain::testFloat_data()
{
    QTest::addColumn<QString>("name");
    QTest::addColumn<double>("score");
    QTest::newRow("zhangsan") << "zhangsan" << 60.0;
    QTest::newRow("lisi") << "lisi" << 56.0;
    QTest::newRow("wanger") << "wanger" << 48.0;
}

void QTestMain::testFloat()
{
    QFETCH(QString, name);
    QFETCH(double, score);
    QVERIFY2(score >= 30.0, name.toLocal8Bit() + " score: " + QString::number(score).toLocal8Bit());
}

void QTestMain::testGui_data()
{
    QTest::addColumn<QTestEventList>("event");
    QTest::addColumn<QString>("result");

    QTestEventList list1;
    list1.addKeyClicks("hello world");
    QTest::newRow("item 0 ") << list1 << QString("hello world");

    QTestEventList list2;
    list2.addKeyClicks("abs0");
    list2.addKeyClick(Qt::Key_Backspace);
    QTest::newRow("item 1") << list2 << QString("abs");
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

QMYTEST_MAIN(QTestMain)

#include "testMain.moc"
