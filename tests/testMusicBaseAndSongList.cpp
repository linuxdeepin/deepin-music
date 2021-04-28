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
#include <QMimeData>
#include <QStandardPaths>
#include <infodialog.h>
#include <DApplication>
#include <QDBusInterface>
#include <QDBusPendingCall>
#include <QShortcut>

#include <DPushButton>

#include "ac-desktop-define.h"
#include "mainframe.h"
#include "musicbaselistview.h"
#include "playlistview.h"
#include "musicsonglistview.h"


TEST(Application, musicBaseSong)
{
    TEST_CASE_NAME("musicBaseSong")
    MainFrame *w = Application::getInstance()->getMainWindow();
    MusicBaseListView *baseListView = w->findChild<MusicBaseListView *>(AC_dataBaseListview);

    QMimeData mimedata;
    QList<QUrl> li;
    QString lastImportPath =  QStandardPaths::standardLocations(QStandardPaths::MusicLocation).first();
    lastImportPath += "/歌曲/004.mp3";
    li.append(QUrl(lastImportPath));
    mimedata.setUrls(li);

    // 关闭导入失败窗口
    QTimer::singleShot(500, [ = ]() {
        DDialog *warnDlg = w->findChild<DDialog *>("uniquewarndailog");
        if (warnDlg) {
            warnDlg->close();
        }
    });

    QPoint pos = QPoint(130, 130);

    QDragEnterEvent eEnter(pos, Qt::IgnoreAction, &mimedata, Qt::LeftButton, Qt::NoModifier);
    qApp->sendEvent(baseListView->viewport(), &eEnter);

    QDragMoveEvent eMove(pos, Qt::IgnoreAction, &mimedata, Qt::LeftButton, Qt::NoModifier);
    qApp->sendEvent(baseListView->viewport(), &eMove);

    QDropEvent e(pos, Qt::IgnoreAction, &mimedata, Qt::LeftButton, Qt::NoModifier);
    qApp->sendEvent(baseListView->viewport(), &e);

    QTest::qWait(100);
}

TEST(Application, musicBaseSong1)
{
    TEST_CASE_NAME("musicBaseSong")

    MainFrame *w = Application::getInstance()->getMainWindow();
    MusicSongListView *songListView = w->findChild<MusicSongListView *>(AC_customizeListview);

    // 新建歌单
    QTestEventList event;
    songListView->addNewSongList();

    // 点击自定义列表
    QTest::qWait(100);
    QPoint pos(20, 20);
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
    event.simulate(songListView->viewport());
    event.clear();


    QMimeData mimedata;
    QList<QUrl> li;
    QString lastImportPath =  QStandardPaths::standardLocations(QStandardPaths::MusicLocation).first();
    lastImportPath += "/歌曲/004.mp3";
    li.append(QUrl(lastImportPath));
    mimedata.setUrls(li);

    // 关闭导入失败窗口
    QTimer::singleShot(500, [ = ]() {
        DDialog *warnDlg = w->findChild<DDialog *>("uniquewarndailog");
        if (warnDlg) {
            warnDlg->close();
        }
    });

    pos = QPoint(20, 20);
    QDragEnterEvent eEnter(pos, Qt::IgnoreAction, &mimedata, Qt::LeftButton, Qt::NoModifier);
    qApp->sendEvent(songListView->viewport(), &eEnter);

    QDragMoveEvent eMove(pos, Qt::IgnoreAction, &mimedata, Qt::LeftButton, Qt::NoModifier);
    qApp->sendEvent(songListView->viewport(), &eMove);

    QDropEvent e(pos, Qt::IgnoreAction, &mimedata, Qt::LeftButton, Qt::NoModifier);
    qApp->sendEvent(songListView->viewport(), &e);

    QTest::qWait(100);
}

//esc按键,重命名取消
TEST(Application, musicSongListViewEsc)
{
    TEST_CASE_NAME("musicSongListView");
    MainFrame *w = Application::getInstance()->getMainWindow();
    MusicSongListView *songListView = w->findChild<MusicSongListView *>(AC_customizeListview);
    songListView->addNewSongList();

    //按esc键
    QTimer::singleShot(400, songListView, [ = ]() {
        QShortcut *escShortcut = songListView->findChild<QShortcut *>(AC_Shortcut_Escape);
        if (escShortcut != nullptr)
            escShortcut->activated();
        QTest::qWait(600);
    });
    // 双击list
    QPoint pos = QPoint(73, 21);
    QTestEventList eventList;
    eventList.addMouseMove(pos);
    eventList.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    eventList.addMousePress(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    eventList.addMouseDClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    eventList.simulate(songListView->viewport());
    eventList.clear();
    QTest::qWait(1000);
}

TEST(Application, songListViewPlayorPause)
{
    TEST_CASE_NAME("musicSongListView");
    MainFrame *w = Application::getInstance()->getMainWindow();
    MusicSongListView *songListView = w->findChild<MusicSongListView *>(AC_customizeListview);

    QTestEventList eventList;
    eventList.addKeyClick(Qt::Key_Space, Qt::NoModifier, 10);
    eventList.simulate(songListView->viewport());
    eventList.clear();
    QTest::qWait(200);

    eventList.addKeyClick(Qt::Key_Space, Qt::NoModifier, 10);
    eventList.simulate(songListView->viewport());
    eventList.clear();
    QTest::qWait(50);
}

//MusicSongListView右键菜单测试1
TEST(Application, musicSongListViewRename)
{
    TEST_CASE_NAME("musicSongListView");
    MainFrame *w = Application::getInstance()->getMainWindow();
    MusicSongListView *songListView = w->findChild<MusicSongListView *>(AC_customizeListview);

    QPoint pos(73, 21);
    QTestEventList eventList;
    eventList.addMouseMove(pos);
    eventList.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    eventList.simulate(songListView->viewport());
    eventList.clear();
    QTest::qWait(100);
    //菜单触发重命名
    QTimer::singleShot(300, songListView, [ = ]() {
        QTimer::singleShot(200, songListView, [ = ]() {
            QTestEventList event;
            event.addKeyClick(Qt::Key_Enter, Qt::NoModifier, 50);
            event.simulate(songListView->viewport());
            event.clear();
            QTest::qWait(50);
        });
        QTestEventList event;
        DMenu *menuWidget = static_cast<DMenu *>(qApp->activePopupWidget());
        if (menuWidget != nullptr) {
            event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
            event.addKeyClick(Qt::Key_Enter, Qt::NoModifier, 50);
            event.simulate(menuWidget);
            event.clear();
            QTest::qWait(50);
        }
        QTest::qWait(950);
    });

    QContextMenuEvent menuEvent(QContextMenuEvent::Mouse, pos);
    qApp->sendEvent(songListView->viewport(), &menuEvent);
    QTest::qWait(2000);
}

//MusicSongListView右键菜单测试2
TEST(Application, musicSongListViewMenuDelete)
{
    TEST_CASE_NAME("musicSongListView");
    MainFrame *w = Application::getInstance()->getMainWindow();
    MusicSongListView *songListView = w->findChild<MusicSongListView *>(AC_customizeListview);

    //菜单触发删除
    QPoint pos(73, 21);
    QTestEventList eventList;
    eventList.addMouseMove(pos);
    eventList.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    eventList.simulate(songListView->viewport());
    eventList.clear();
    QTest::qWait(100);

    QTimer::singleShot(300, songListView, [ = ]() {
        QTimer::singleShot(300, songListView, [ = ]() {
            QTest::qWait(50);
            QTestEventList event;
            DDialog *messageBox = songListView->findChild<DDialog *>(AC_MessageBox);
            if (messageBox != nullptr) {
                event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
                event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
                event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
                event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
                event.addKeyClick(Qt::Key_Enter, Qt::NoModifier, 50);
                event.simulate(messageBox);
                event.clear();
                QTest::qWait(50);
            }
        });

        QTestEventList event;
        DMenu *menuWidget = static_cast<DMenu *>(qApp->activePopupWidget());
        if (menuWidget != nullptr) {
            event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
            event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
            event.addKeyClick(Qt::Key_Enter, Qt::NoModifier, 50);
            event.addDelay(100);
            event.simulate(menuWidget);
            event.clear();
            QTest::qWait(50);
        }
        QTest::qWait(950);
    });

    QContextMenuEvent menuEvent(QContextMenuEvent::Mouse, pos);
    qApp->sendEvent(songListView->viewport(), &menuEvent);
    QTest::qWait(2000);
    songListView->setThemeType(1);
}

TEST(Application, songlistSearch)
{
    // fix bug77449
    MainFrame *w = Application::getInstance()->getMainWindow();
    MusicSongListView *songListView = w->findChild<MusicSongListView *>(AC_customizeListview);
    //菜单触发删除
    QPoint pos(73, 61);
    QTestEventList eventList;
    eventList.addMouseMove(pos);
    eventList.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    eventList.simulate(songListView->viewport());
    eventList.clear();
    QTest::qWait(100);

    // 歌曲搜索
    QLineEdit *se = w->findChild<QLineEdit *>(AC_Search);
    // 搜索a
    QTest::qWait(50);
    pos = QPoint(100, 20);
    QTestEventList event1;
    event1.addMouseMove(pos);
    event1.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 50);
    event1.addKeyPress(Qt::Key::Key_A, Qt::NoModifier, 100);
    event1.addKeyPress(Qt::Key::Key_Enter, Qt::NoModifier, 100);
    event1.simulate(se);
    event1.clear();

    DPushButton *playAllBtn = w->findChild<DPushButton *>(AC_musicListDataPlayAll);
    bool isEnable = false;
    if (playAllBtn) {
        isEnable = playAllBtn->isEnabled();
    }
    QCOMPARE(isEnable, true);
}
