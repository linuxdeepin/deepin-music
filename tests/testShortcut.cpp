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

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include "application.h"

#include <QTest>
#include <QDebug>
#include <QPoint>
#include <QTimer>
#include <infodialog.h>
#include <DApplication>
#include <QDBusInterface>
#include <QDBusPendingCall>

#include "ac-desktop-define.h"

#include "mainframe.h"
#include "musicbaselistview.h"
#include "playlistview.h"
#include "musicsonglistview.h"
#include "speechCenter.h"
#include "shortcut.h"


TEST(Application, shortCut)
{
    TEST_CASE_NAME("shortCut")

    QTest::qWait(50);
    MainFrame *w = Application::getInstance()->getMainWindow();
    MusicBaseListView *baseListView = w->findChild<MusicBaseListView *>(AC_dataBaseListview);
    QTest::qWait(50);
    // 点击所有音乐
    QPoint pos(130, 100);
    QTestEventList event;
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.simulate(baseListView->viewport());
    event.clear();

    // 双击list
    QTest::qWait(50);
    pos = QPoint(20, 20);
    PlayListView *plv = w->findChild<PlayListView *>(AC_PlayListView);
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.addMousePress(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.addMouseDClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.simulate(plv->viewport());
    event.clear();


    // 空格
    QTest::qWait(100);
    event.addKeyClick(Qt::Key_Space, Qt::NoModifier, 10);
    event.simulate(w);
    event.clear();
    QTest::qWait(100);

    // 上一首
    QTest::qWait(50);
    event.addKeyClick(Qt::Key_Left, Qt::ControlModifier, 10);
    event.simulate(w);
    event.clear();

    // 下一首
    QTest::qWait(50);
    event.addKeyClick(Qt::Key_Right, Qt::ControlModifier, 10);
    event.simulate(w);
    event.clear();

    // 音量增大
    QTest::qWait(50);
    event.addKeyPress(Qt::Key_Up, Qt::ControlModifier | Qt::AltModifier, 10);
    event.simulate(w);
    event.clear();

    // 音量减小
    QTest::qWait(50);
    event.addKeyPress(Qt::Key_Down, Qt::ControlModifier | Qt::AltModifier, 10);
    event.simulate(w);
    event.clear();

    // 静音
    QTest::qWait(50);
    event.addKeyClick(Qt::Key_M, Qt::NoModifier, 10);
    event.simulate(w);
    event.clear();

    // 解除静音
    QTest::qWait(50);
    event.addKeyClick(Qt::Key_M, Qt::NoModifier, 10);
    event.simulate(w);
    event.clear();


    // todo 收藏未实现
    QTest::qWait(50);
    event.addKeyClick(Qt::Key_Dollar, Qt::NoModifier, 10);
    event.simulate(w);
    event.clear();



    // 删除MessageBox
    QTest::qWait(50);
    QTimer::singleShot(200, w, [ = ]() {
        // 清空ListView
        QTest::qWait(50);
        DDialog *messageBox = w->findChild<DDialog *>("MessageBox");
        if (messageBox) {
            QPoint pos = QPoint(130, 150);
            QTestEventList event;
            event.addMouseMove(pos);
            event.addKeyClick(Qt::Key::Key_Tab, Qt::NoModifier, 10);
            event.addKeyClick(Qt::Key::Key_Tab, Qt::NoModifier, 10);
            event.addKeyClick(Qt::Key::Key_Tab, Qt::NoModifier, 10);
            event.addKeyClick(Qt::Key::Key_Tab, Qt::NoModifier, 10);
            event.addKeyClick(Qt::Key::Key_Enter, Qt::NoModifier, 50);
            event.simulate(messageBox);
            event.clear();
        }
    });

    // 从歌单中删除
    QTest::qWait(50);
    event.addKeyClick(Qt::Key_Delete, Qt::NoModifier, 10);
    event.simulate(w);
    event.clear();



    // 关闭歌曲信息
    QTest::qWait(200);
    QTimer::singleShot(200, w, [ = ]() {
        QTest::qWait(50);
        InfoDialog *infoDialog = w->findChild<InfoDialog *>("infoDialog");
        if (infoDialog) {
            infoDialog->close();
        }
    });

    // 歌曲信息
    QTest::qWait(50);
    event.addKeyClick(Qt::Key_I, Qt::ControlModifier, 10);
    event.simulate(w);
    event.clear();

    // 点击自定义列表
    QTest::qWait(50);
    MusicSongListView *songListView = w->findChild<MusicSongListView *>(AC_customizeListview);
    QTest::qWait(50);
    pos = QPoint(130, 20);
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.simulate(songListView->viewport());
    event.clear();


    // 重命名歌单
    QTest::qWait(50);
    event.addKeyClick(Qt::Key_F2, Qt::NoModifier, 10);
    event.addKeyClick(Qt::Key_W, Qt::NoModifier, 10);
    event.addKeyClick(Qt::Key_Enter, Qt::NoModifier, 10);
    event.simulate(w);
    event.clear();

    // 新建歌单
    QTest::qWait(50);
    event.addKeyClick(Qt::Key_N, Qt::ControlModifier | Qt::ShiftModifier, 10);
    event.simulate(w);
    event.clear();

//    // 帮助
//    QTest::qWait(500);
//    event.addKeyClick(Qt::Key_F1, Qt::NoModifier, 100);
//    event.simulate(w);
//    event.clear();

    // 快捷键预览
    QTest::qWait(50);
    event.addKeyClick(Qt::Key_Slash, Qt::ControlModifier | Qt::ShiftModifier, 10);
    event.simulate(w);
    event.clear();

    Shortcut sc;
    sc.toStr();

    QTest::qWait(100);
}

TEST(Application, shortCut1)
{
    TEST_CASE_NAME("shortCut1")

    QTest::qWait(50);
    MainFrame *w = Application::getInstance()->getMainWindow();
    MusicBaseListView *baseListView = w->findChild<MusicBaseListView *>(AC_dataBaseListview);
    QTest::qWait(50);
    // 点击所有音乐
    QPoint pos(130, 100);
    QTestEventList event;
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.simulate(baseListView->viewport());
    event.clear();

    // 双击list
    QTest::qWait(50);
    pos = QPoint(20, 20);
    PlayListView *plv = w->findChild<PlayListView *>(AC_PlayListView);
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.addMousePress(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.addMouseDClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.simulate(plv->viewport());
    event.clear();


    // Ctrl+F
    QTest::qWait(10);
    event.addKeyClick(Qt::Key_F, Qt::ControlModifier, 10);
    event.simulate(w);
    event.clear();
    QTest::qWait(10);

    // Ctrl+Alt+F
    QTest::qWait(10);
    event.addKeyClick(Qt::Key_F, Qt::ControlModifier | Qt::AltModifier, 10);
    event.simulate(w);
    event.clear();
    QTest::qWait(10);

    // Ctrl+Shift+/
    QTest::qWait(10);
    event.addKeyClick(Qt::Key_Slash, Qt::ControlModifier | Qt::ShiftModifier, 10);
    event.simulate(w);
    event.clear();
    QTest::qWait(10);

    QTest::qWait(100);
}

TEST(Application, other)
{
    TEST_CASE_NAME("other")
    MainFrame *w = Application::getInstance()->getMainWindow();
    MusicBaseListView *baseListView = w->findChild<MusicBaseListView *>(AC_dataBaseListview);

    QPoint pos = QPoint(130, 30);
    QTestEventList event;
    // 点击所有音乐
    QTest::qWait(100);
    pos = QPoint(130, 100);
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.simulate(baseListView->viewport());
    event.clear();

    // 双击list
    pos = QPoint(20, 120);
    PlayListView *plv = w->findChild<PlayListView *>(AC_PlayListView);
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.addMousePress(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.addMouseDClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.simulate(plv->viewport());
    event.clear();

    QTest::qWait(500);
    emit Player::getInstance()->getMpris()->playRequested();

    QTest::qWait(500);
    emit Player::getInstance()->getMpris()->pauseRequested();

    QTest::qWait(500);
    emit Player::getInstance()->getMpris()->playRequested();

    QTest::qWait(500);
    emit Player::getInstance()->getMpris()->previousRequested();

    Player::getInstance()->setMode(Player::Shuffle);
    QTest::qWait(500);
    emit Player::getInstance()->getMpris()->nextRequested();

    Player::getInstance()->setMode(Player::RepeatNull);
    QTest::qWait(500);
    emit Player::getInstance()->getMpris()->nextRequested();

    QTest::qWait(500);
    //Player::getInstance()->loadMediaProgress(Player::getInstance()->getActiveMeta().localPath);

    MediaMeta meta = Player::getInstance()->getActiveMeta();
    Player::getInstance()->stop();
    Player::getInstance()->setActiveMeta(meta);
    Player::getInstance()->resume();

    QTest::qWait(500);
    emit Player::getInstance()->getMpris()->raiseRequested();

    w->autoStartToPlay();
//    mainfram里的autoStartToPlay直接调

    QTest::qWait(100);
}

TEST(Application, otherImportLinkText1)
{
    // 扫描歌曲
    TEST_CASE_NAME("otherImportLinkText1")

    QTest::qWait(100);
    MainFrame *w = Application::getInstance()->getMainWindow();
    QLabel *ilt = w->findChild<QLabel *>(AC_importLinkText);
    ilt->linkActivated("");
    QTest::qWait(500);
}

TEST(Application, other1)
{
    TEST_CASE_NAME("other1")
    MainFrame *w = Application::getInstance()->getMainWindow();
    MusicBaseListView *baseListView = w->findChild<MusicBaseListView *>(AC_dataBaseListview);
    QPoint pos = QPoint(130, 30);
    QTestEventList event;
    // 点击所有音乐
    QTest::qWait(100);
    pos = QPoint(130, 100);
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.simulate(baseListView->viewport());
    event.clear();

    // 双击list
    pos = QPoint(20, 120);
    PlayListView *plv = w->findChild<PlayListView *>(AC_PlayListView);
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.addMousePress(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.addMouseDClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.simulate(plv->viewport());
    event.clear();


    QTest::qWait(100);
    qDebug() << __FUNCTION__ << "playMusic浮夸";
    SpeechCenter::getInstance()->playMusic("浮夸");
    qDebug() << __FUNCTION__ << "playMusic浮夸1";
    SpeechCenter::getInstance()->playMusic("浮夸1");//不存在
    SpeechCenter::getInstance()->playMusic("");
    SpeechCenter::getInstance()->playArtist("华晨宇");
    qDebug() << __FUNCTION__ << "playArtist华晨";
    SpeechCenter::getInstance()->playArtist("华晨");//模糊匹配
    qDebug() << __FUNCTION__ << "playArtist华晨1";
    SpeechCenter::getInstance()->playArtist("华晨1");//不存在
    SpeechCenter::getInstance()->playArtistMusic("华晨宇:浮夸");
    SpeechCenter::getInstance()->playArtistMusic("华晨宇");
    SpeechCenter::getInstance()->playArtistMusic("");
    SpeechCenter::getInstance()->playAlbum("Unknown album");
    qDebug() << __FUNCTION__ << "未知专辑";
    SpeechCenter::getInstance()->playAlbum("未知专辑");
    SpeechCenter::getInstance()->playFaverite("fav");
    SpeechCenter::getInstance()->playSonglist("New playlist");
    SpeechCenter::getInstance()->playSonglist("空");
    qDebug() << __FUNCTION__ << "playMusic浮夸";
    SpeechCenter::getInstance()->playMusic("浮夸");
    qDebug() << __FUNCTION__ << "pause";
    SpeechCenter::getInstance()->pause("");
    SpeechCenter::getInstance()->resume("");
    SpeechCenter::getInstance()->next("");
    SpeechCenter::getInstance()->pre("");
    SpeechCenter::getInstance()->stop("");
    SpeechCenter::getInstance()->playIndex("100");
    SpeechCenter::getInstance()->playIndex("1");
    SpeechCenter::getInstance()->addFaverite("");
    SpeechCenter::getInstance()->removeFaverite("");
    SpeechCenter::getInstance()->setMode("0");
    SpeechCenter::getInstance()->setMode("1");
    SpeechCenter::getInstance()->setMode("2");
    qDebug() << __FUNCTION__ << "OpenUris";
    QStringList list;
    SpeechCenter::getInstance()->OpenUris(list);
    qDebug() << __FUNCTION__ << "OpenUris2";
    QTest::qWait(500);
}

TEST(Application, otherImportLinkText2)
{
    // 扫描歌曲
    TEST_CASE_NAME("otherImportLinkText2")

    QTest::qWait(100);
    MainFrame *w = Application::getInstance()->getMainWindow();
    QLabel *ilt = w->findChild<QLabel *>(AC_importLinkText);
    ilt->linkActivated("");
    QTest::qWait(500);
}

