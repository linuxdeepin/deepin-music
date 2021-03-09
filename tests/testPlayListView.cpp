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
#include <DToolButton>

#include "ac-desktop-define.h"

#include "mainframe.h"
#include "musicbaselistview.h"
#include "playlistview.h"
#include "musicsonglistview.h"


#include "databaseservice.h"


// 菜单播放
TEST(Application, playListView1)
{
    TEST_CASE_NAME("playListView")

    MainFrame *w = Application::getInstance()->getMainWindow();
    MusicBaseListView *baseListView = w->findChild<MusicBaseListView *>(AC_dataBaseListview);

    QTest::qWait(500);
    QPoint pos = QPoint(20, 20);
    QTestEventList event;

    // 点击所有音乐
    QTest::qWait(50);
    QPoint pos1(130, 100);
    event.addMouseMove(pos1);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos1, 10);
    event.simulate(baseListView->viewport());
    event.clear();
    QTest::qWait(100);

    // 双击list
    pos = QPoint(20, 20);
    PlayListView *plv = w->findChild<PlayListView *>(AC_PlayListView);
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.addMousePress(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.addMouseDClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.simulate(plv->viewport());
    event.clear();

    // 点击切换回list模式
    QTest::qWait(50);
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    DToolButton *iconListBtn = w->findChild<DToolButton *>(AC_btlistMode);
    event.simulate(iconListBtn);
    event.clear();

    QTimer::singleShot(300, w, [ = ]() {
        QTestEventList event;
        DMenu *menuWidget = static_cast<DMenu *>(qApp->activePopupWidget());
        event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
        event.addKeyClick(Qt::Key_Enter, Qt::NoModifier, 50);
        event.addDelay(100);
        event.simulate(menuWidget);
        event.clear();
        QTest::qWait(100);
    });

    QContextMenuEvent menuEvent(QContextMenuEvent::Mouse, QPoint(20, 20));
    qApp->sendEvent(plv->viewport(), &menuEvent);

    QTest::qWait(100);
}

// 菜单暂停
TEST(Application, playListView2)
{
    TEST_CASE_NAME("playListView")

    MainFrame *w = Application::getInstance()->getMainWindow();

    QTest::qWait(500);
    QPoint pos = QPoint(20, 20);
    QTestEventList event;

    // 双击list
    pos = QPoint(20, 20);
    PlayListView *plv = w->findChild<PlayListView *>(AC_PlayListView);
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.addMousePress(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.addMouseDClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.simulate(plv->viewport());
    event.clear();

    QTimer::singleShot(300, w, [ = ]() {
//        QTimer::singleShot(200, w, [ = ]() {
//            QTest::qWait(1000);
//            QTestEventList event;
//            DDialog *messageBox = w->findChild<DDialog *>(AC_MessageBox);
//            event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
//            event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
//            event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
//            event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
//            event.addKeyClick(Qt::Key_Enter, Qt::NoModifier, 50);
//            event.simulate(messageBox);
//            event.clear();
//        });

        QTestEventList event;
        DMenu *menuWidget = static_cast<DMenu *>(qApp->activePopupWidget());
        event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
        event.addKeyClick(Qt::Key_Enter, Qt::NoModifier, 50);
        event.addDelay(100);
        event.simulate(menuWidget);
        event.clear();
        QTest::qWait(100);
    });

    QContextMenuEvent menuEvent(QContextMenuEvent::Mouse, QPoint(20, 20));
    qApp->sendEvent(plv->viewport(), &menuEvent);

    QTest::qWait(1000);
}

// 菜单添加到播放队列
TEST(Application, playListView3)
{
    TEST_CASE_NAME("playListView")

    MainFrame *w = Application::getInstance()->getMainWindow();

    QTest::qWait(500);
    QPoint pos = QPoint(20, 20);
    QTestEventList event;

    // 双击list
    PlayListView *plv = w->findChild<PlayListView *>(AC_PlayListView);
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.addMousePress(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.addMouseDClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.simulate(plv->viewport());
    event.clear();

    QTimer::singleShot(300, w, [ = ]() {
        QTestEventList event;
        DMenu *menuWidget = static_cast<DMenu *>(qApp->activePopupWidget());
        event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
        event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
        event.addKeyClick(Qt::Key_Enter, Qt::NoModifier, 50);
        event.addDelay(100);
        event.simulate(menuWidget);
        event.clear();
        QTest::qWait(100);

        menuWidget = static_cast<DMenu *>(qApp->activePopupWidget());
        event.addKeyClick(Qt::Key_Enter, Qt::NoModifier, 50);
        event.simulate(menuWidget);
        event.clear();
        QTest::qWait(100);
    });

    QContextMenuEvent menuEvent(QContextMenuEvent::Mouse, QPoint(20, 20));
    qApp->sendEvent(plv->viewport(), &menuEvent);

    QTest::qWait(1000);
}

// 菜单添加到我的收藏
TEST(Application, playListView4)
{
    TEST_CASE_NAME("playListView")

    MainFrame *w = Application::getInstance()->getMainWindow();

    QTest::qWait(500);
    QPoint pos = QPoint(20, 20);
    QTestEventList event;

    // 双击list
    PlayListView *plv = w->findChild<PlayListView *>(AC_PlayListView);
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.addMousePress(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.addMouseDClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.simulate(plv->viewport());
    event.clear();

    QTimer::singleShot(300, w, [ = ]() {
        QTestEventList event;
        DMenu *menuWidget = static_cast<DMenu *>(qApp->activePopupWidget());
        event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
        event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
        event.addKeyClick(Qt::Key_Enter, Qt::NoModifier, 50);
        event.addDelay(100);
        event.simulate(menuWidget);
        event.clear();
        QTest::qWait(100);

        menuWidget = static_cast<DMenu *>(qApp->activePopupWidget());
        event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
        event.addKeyClick(Qt::Key_Enter, Qt::NoModifier, 50);
        event.simulate(menuWidget);
        event.clear();
        QTest::qWait(100);
    });

    QContextMenuEvent menuEvent(QContextMenuEvent::Mouse, QPoint(20, 20));
    qApp->sendEvent(plv->viewport(), &menuEvent);

    QTest::qWait(1000);
}

// 菜单添加到新建歌单
TEST(Application, playListView5)
{
    TEST_CASE_NAME("playListView")

    MainFrame *w = Application::getInstance()->getMainWindow();
    MusicBaseListView *baseListView = w->findChild<MusicBaseListView *>(AC_dataBaseListview);

    QTest::qWait(500);
    QPoint pos = QPoint(20, 20);
    QTestEventList event;

    // 双击list
    PlayListView *plv = w->findChild<PlayListView *>(AC_PlayListView);
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.addMousePress(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.addMouseDClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.simulate(plv->viewport());
    event.clear();

    QTimer::singleShot(300, w, [ = ]() {
        QTestEventList event;
        DMenu *menuWidget = static_cast<DMenu *>(qApp->activePopupWidget());
        event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
        event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
        event.addKeyClick(Qt::Key_Enter, Qt::NoModifier, 50);
        event.addDelay(100);
        event.simulate(menuWidget);
        event.clear();
        QTest::qWait(100);

        menuWidget = static_cast<DMenu *>(qApp->activePopupWidget());
        event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
        event.addKeyClick(Qt::Key_Enter, Qt::NoModifier, 50);
        event.simulate(menuWidget);
        event.clear();
        baseListView->setFocus();
        QTest::qWait(100);
    });

    QContextMenuEvent menuEvent(QContextMenuEvent::Mouse, QPoint(20, 20));
    qApp->sendEvent(plv->viewport(), &menuEvent);

    QTest::qWait(1000);
}

// 菜单添加到其他歌单
TEST(Application, playListView6)
{
    TEST_CASE_NAME("playListView")

    MainFrame *w = Application::getInstance()->getMainWindow();
    MusicBaseListView *baseListView = w->findChild<MusicBaseListView *>(AC_dataBaseListview);

    QTest::qWait(500);
    QPoint pos = QPoint(20, 20);
    QTestEventList event;

    // 点击所有音乐
    QTest::qWait(50);
    pos.setY(100);
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.simulate(baseListView->viewport());
    event.clear();
    QTest::qWait(50);

    // 双击list
    pos = QPoint(20, 20);
    PlayListView *plv = w->findChild<PlayListView *>(AC_PlayListView);
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.addMousePress(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.addMouseDClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.simulate(plv->viewport());
    event.clear();

    QTimer::singleShot(300, w, [ = ]() {
        QTestEventList event;
        DMenu *menuWidget = static_cast<DMenu *>(qApp->activePopupWidget());
        event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
        event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
        event.addKeyClick(Qt::Key_Enter, Qt::NoModifier, 50);
        event.addDelay(100);
        event.simulate(menuWidget);
        event.clear();
        QTest::qWait(100);

        menuWidget = static_cast<DMenu *>(qApp->activePopupWidget());
        event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
        event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
        event.addKeyClick(Qt::Key_Enter, Qt::NoModifier, 50);
        event.simulate(menuWidget);
        event.clear();
        QTest::qWait(100);
    });

    QTest::qWait(50);
    QContextMenuEvent menuEvent(QContextMenuEvent::Mouse, QPoint(20, 20));
    qApp->sendEvent(plv->viewport(), &menuEvent);

    QTest::qWait(1000);
}

// 菜单从歌单中删除
TEST(Application, playListView7)
{
    TEST_CASE_NAME("playListView")

    MainFrame *w = Application::getInstance()->getMainWindow();
    MusicBaseListView *baseListView = w->findChild<MusicBaseListView *>(AC_dataBaseListview);

    QTest::qWait(500);
    QPoint pos = QPoint(20, 20);
    QTestEventList event;

    // 点击所有音乐
    QTest::qWait(50);
    pos.setY(100);
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.simulate(baseListView->viewport());
    event.clear();
    QTest::qWait(50);

    // 双击list
    PlayListView *plv = w->findChild<PlayListView *>(AC_PlayListView);
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.addMousePress(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.addMouseDClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.simulate(plv->viewport());
    event.clear();

    QTimer::singleShot(300, w, [ = ]() {
        QTimer::singleShot(800, w, [ = ]() {
            QTest::qWait(500);
            QTestEventList event;
            DDialog *messageBox = w->findChild<DDialog *>(AC_MessageBox);
            event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
            event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
            event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
            event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
            event.addKeyClick(Qt::Key_Enter, Qt::NoModifier, 50);
            event.simulate(messageBox);
            event.clear();
            QTest::qWait(50);
        });

        QTestEventList event;
        DMenu *menuWidget = static_cast<DMenu *>(qApp->activePopupWidget());
        event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
        event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
        event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
        event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
        event.addKeyClick(Qt::Key_Enter, Qt::NoModifier, 50);
        event.addDelay(100);
        event.simulate(menuWidget);
        event.clear();
        QTest::qWait(100);
    });

    QContextMenuEvent menuEvent(QContextMenuEvent::Mouse, QPoint(20, 20));
    qApp->sendEvent(plv->viewport(), &menuEvent);

    QTest::qWait(1000);
}

// 菜单从本地中删除
TEST(Application, playListView8)
{
    TEST_CASE_NAME("playListView")

    MainFrame *w = Application::getInstance()->getMainWindow();
    MusicBaseListView *baseListView = w->findChild<MusicBaseListView *>(AC_dataBaseListview);

    QTest::qWait(500);
    QPoint pos = QPoint(20, 20);
    QTestEventList event;

    // 点击所有音乐
    QTest::qWait(50);
    pos.setY(100);
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.simulate(baseListView->viewport());
    event.clear();
    QTest::qWait(50);

    // 双击list
    pos = QPoint(20, 80);
    PlayListView *plv = w->findChild<PlayListView *>(AC_PlayListView);
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.addMousePress(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.addMouseDClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.simulate(plv->viewport());
    event.clear();

    QTimer::singleShot(300, w, [ = ]() {
        QTimer::singleShot(800, w, [ = ]() {
            QTest::qWait(500);
            QTestEventList event;
            DDialog *messageBox = w->findChild<DDialog *>(AC_MessageBox);
            event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
            event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
            event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
            event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
            event.addKeyClick(Qt::Key_Enter, Qt::NoModifier, 50);
            event.simulate(messageBox);
            event.clear();
            QTest::qWait(50);
        });

        QTestEventList event;
        DMenu *menuWidget = static_cast<DMenu *>(qApp->activePopupWidget());
        event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
        event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
        event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
        event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
        event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
        event.addKeyClick(Qt::Key_Enter, Qt::NoModifier, 50);
        event.addDelay(100);
        event.simulate(menuWidget);
        event.clear();
        QTest::qWait(100);
    });

    QContextMenuEvent menuEvent(QContextMenuEvent::Mouse, QPoint(20, 20));
    qApp->sendEvent(plv->viewport(), &menuEvent);

    QTest::qWait(1000);
}

// 菜单编码方式
TEST(Application, playListView9)
{
    TEST_CASE_NAME("playListView")

    MainFrame *w = Application::getInstance()->getMainWindow();

    QTest::qWait(500);
    QPoint pos = QPoint(20, 20);
    QTestEventList event;

    // 双击list
    PlayListView *plv = w->findChild<PlayListView *>(AC_PlayListView);
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.addMousePress(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.addMouseDClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.simulate(plv->viewport());
    event.clear();

    QTimer::singleShot(300, w, [ = ]() {
        QTestEventList event;
        DMenu *menuWidget = static_cast<DMenu *>(qApp->activePopupWidget());
        event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
        event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
        event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
        event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
        event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
        event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
        event.addKeyClick(Qt::Key_Enter, Qt::NoModifier, 50);
        event.addDelay(100);
        event.simulate(menuWidget);
        event.clear();
        QTest::qWait(100);

        menuWidget = static_cast<DMenu *>(qApp->activePopupWidget());
        event.addKeyClick(Qt::Key_Enter, Qt::NoModifier, 50);
        event.simulate(menuWidget);
        event.clear();
        QTest::qWait(100);
    });

    QContextMenuEvent menuEvent(QContextMenuEvent::Mouse, QPoint(20, 20));
    qApp->sendEvent(plv->viewport(), &menuEvent);

    QTest::qWait(1000);
}

// 菜单歌曲信息
TEST(Application, playListView10)
{
    TEST_CASE_NAME("playListView")

    MainFrame *w = Application::getInstance()->getMainWindow();

    QTest::qWait(500);
    QPoint pos = QPoint(20, 20);
    QTestEventList event;

    // 双击list
    PlayListView *plv = w->findChild<PlayListView *>(AC_PlayListView);
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.addMousePress(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.addMouseDClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.simulate(plv->viewport());
    event.clear();

    QTimer::singleShot(300, w, [ = ]() {
        QTestEventList event;
        DMenu *menuWidget = static_cast<DMenu *>(qApp->activePopupWidget());
        event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
        event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
        event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
        event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
        event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
        event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
        event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
        event.addKeyClick(Qt::Key_Enter, Qt::NoModifier, 50);
        event.addDelay(100);
        event.simulate(menuWidget);
        event.clear();
        QTest::qWait(100);

        InfoDialog *info = w->findChild<InfoDialog *>(AC_infoDialog);
        info->close();
    });

    QContextMenuEvent menuEvent(QContextMenuEvent::Mouse, QPoint(20, 20));
    qApp->sendEvent(plv->viewport(), &menuEvent);

    QTest::qWait(1000);
}

TEST(Application, playListViewDrag)
{
    TEST_CASE_NAME("playListViewDrag")

    MainFrame *w = Application::getInstance()->getMainWindow();
    PlayListView *plv = w->findChild<PlayListView *>(AC_PlayListView);

    // 双击list
    QPoint pos = QPoint(20, 20);
    QTestEventList event;
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.addMousePress(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.addMouseDClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.simulate(plv->viewport());
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

    pos = QPoint(130, 130);

    QDragEnterEvent eEnter(pos, Qt::IgnoreAction, &mimedata, Qt::LeftButton, Qt::NoModifier);
    qApp->sendEvent(plv->viewport(), &eEnter);

    QDragMoveEvent eMove(pos, Qt::IgnoreAction, &mimedata, Qt::LeftButton, Qt::NoModifier);
    qApp->sendEvent(plv->viewport(), &eMove);

    QDropEvent e(pos, Qt::IgnoreAction, &mimedata, Qt::LeftButton, Qt::NoModifier);
    qApp->sendEvent(plv->viewport(), &e);

    QTest::qWait(100);
}
