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
#include <albumlistview.h>
#include <QStandardPaths>
#include <infodialog.h>
#include <DApplication>
#include <QDBusInterface>
#include <DToolButton>
#include <QDBusPendingCall>

#include "ac-desktop-define.h"

#include "mainframe.h"
#include "musicbaselistview.h"
#include "playlistview.h"
#include "musicsonglistview.h"


TEST(Application, albumListView)
{
    TEST_CASE_NAME("albumListView")

    MainFrame *w = Application::getInstance()->getMainWindow();
    MusicBaseListView *baseListView = w->findChild<MusicBaseListView *>(AC_dataBaseListview);

    // 点击专辑
    QPoint pos = QPoint(130, 30);
    QTestEventList event;
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.simulate(baseListView->viewport());
    event.clear();

    QTest::qWait(100);
    AlbumListView *alv = w->findChild<AlbumListView *>(AC_albumListView);

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
    qApp->sendEvent(alv->viewport(), &eEnter);

    QDragMoveEvent eMove(pos, Qt::IgnoreAction, &mimedata, Qt::LeftButton, Qt::NoModifier);
    qApp->sendEvent(alv->viewport(), &eMove);

    QDropEvent e(pos, Qt::IgnoreAction, &mimedata, Qt::LeftButton, Qt::NoModifier);
    qApp->sendEvent(alv->viewport(), &e);

    QTest::qWait(100);
}

TEST(Application, albumDataDelegate)
{
    TEST_CASE_NAME("albumListViewDelegate")

    MainFrame *w = Application::getInstance()->getMainWindow();
    MusicBaseListView *baseListView = w->findChild<MusicBaseListView *>(AC_dataBaseListview);
    DToolButton *iconModeBtn = w->findChild<DToolButton *>(AC_btIconMode);

    // 点击专辑
    QPoint pos = QPoint(130, 30);
    QTestEventList event;
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.simulate(baseListView->viewport());
    event.clear();

    QTest::qWait(100);
    AlbumListView *alv = w->findChild<AlbumListView *>(AC_albumListView);


    QTest::qWait(50);
    pos = QPoint(20, 20);
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.simulate(iconModeBtn);
    event.clear();

//    QEvent event1(QEvent::MouseButtonPress);
//    QStyleOptionViewItem option;
//    option.init(alv);

//    QAbstractItemDelegate *itemDelegate = alv->itemDelegate(alv->model()->index(0, 0));
//    if (itemDelegate) {
//        if (alv->model()) {
//            itemDelegate->editorEvent(&event1, alv->model(), option, alv->model()->index(0, 0));
//        }
//    }

    QTest::qWait(100);
}

// 平板相关
TEST(Application, setIsHScreen)
{
    TEST_CASE_NAME("setIsHScreen")
    CommonService::getInstance()->setIsHScreen(false);
    QCOMPARE(false, CommonService::getInstance()->isHScreen());
    CommonService::getInstance()->setIsHScreen(true);
    QCOMPARE(true, CommonService::getInstance()->isHScreen());
}

TEST(Application, setCurrentWidgetPosY)
{
    TEST_CASE_NAME("setCurrentWidgetPosY")
    CommonService::getInstance()->setCurrentWidgetPosY(50);
    QCOMPARE(50, CommonService::getInstance()->getCurrentWidgetPosY());
}

TEST(Application, setSelectModel)
{
    TEST_CASE_NAME("setSelectModel")
    CommonService::getInstance()->setSelectModel(CommonService::SingleSelect);
    QCOMPARE(CommonService::SingleSelect, CommonService::getInstance()->getSelectModel());
}

TEST(Application, setIsTabletEnvironment)
{
    TEST_CASE_NAME("setIsTabletEnvironment")
    CommonService::getInstance()->setIsTabletEnvironment(false);
    QCOMPARE(false, CommonService::getInstance()->isTabletEnvironment());
}
// 平板模式下专辑页面Icon模式显示
TEST(Application, tabletAlbum)
{
    TEST_CASE_NAME("tabletAlbum")
    CommonService::getInstance()->setIsTabletEnvironment(true);

    MainFrame *w = Application::getInstance()->getMainWindow();
    MusicBaseListView *baseListView = w->findChild<MusicBaseListView *>(AC_dataBaseListview);
    QTest::qWait(50);
    // 点击专辑
    QPoint pos(130, 20);
    QTestEventList event;
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.simulate(baseListView->viewport());
    event.clear();

    w->initPadMenu();
    w->initTabletSelectBtn();

    QAction *selectAction = w->findChild<QAction *>(AC_tablet_title_select);
    if (selectAction) {
        w->slotMenuTriggered(selectAction);
    }

    QTest::qWait(200);
    CommonService::getInstance()->setSelectModel(CommonService::SingleSelect);

    QTest::qWait(200);
    CommonService::getInstance()->setIsTabletEnvironment(false);
}

// 平板模式下专辑页面Icon模式下的单击操作
TEST(Application, tabletAlbumInconClick)
{
    TEST_CASE_NAME("tabletAlbumInconClick")
    CommonService::getInstance()->setIsTabletEnvironment(true);

    MainFrame *w = Application::getInstance()->getMainWindow();
    MusicBaseListView *baseListView = w->findChild<MusicBaseListView *>(AC_dataBaseListview);
    QTest::qWait(50);
    // 点击专辑
    QPoint pos(130, 20);
    QTestEventList event;
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.simulate(baseListView->viewport());
    event.clear();

    DToolButton *iconModeBtn = w->findChild<DToolButton *>(AC_btIconMode);
    if (iconModeBtn != nullptr) {
        QTest::mousePress(iconModeBtn, Qt::LeftButton, Qt::NoModifier, QPoint(20, 20), 10);
        QTest::mouseRelease(iconModeBtn, Qt::LeftButton, Qt::NoModifier, QPoint(20, 20), 10);
    }
    QTest::qWait(100);

    AlbumListView *albumListView = w->findChild<AlbumListView *>(AC_albumListView);
    QPoint posClick(87, 78);
    QTest::mouseMove(albumListView->viewport(), posClick, 10);
    QTest::mouseClick(albumListView->viewport(), Qt::LeftButton, Qt::NoModifier, posClick, 10);
    QTest::qWait(1000);
    CommonService::getInstance()->setIsTabletEnvironment(false);
}

// 平板模式下专辑页面Icon模式下的双击操作
TEST(Application, tabletAlbumInconDoubleClick)
{
    TEST_CASE_NAME("tabletAlbumInconDoubleClick")
    CommonService::getInstance()->setIsTabletEnvironment(true);

    MainFrame *w = Application::getInstance()->getMainWindow();
    MusicBaseListView *baseListView = w->findChild<MusicBaseListView *>(AC_dataBaseListview);
    QTest::qWait(50);
    // 点击专辑
    QPoint pos(130, 20);
    QTestEventList event;
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.simulate(baseListView->viewport());
    event.clear();
    DToolButton *iconModeBtn = w->findChild<DToolButton *>(AC_btIconMode);
    if (iconModeBtn != nullptr) {
        QTest::mousePress(iconModeBtn, Qt::LeftButton, Qt::NoModifier, QPoint(20, 20), 10);
        QTest::mouseRelease(iconModeBtn, Qt::LeftButton, Qt::NoModifier, QPoint(20, 20), 10);
    }
    QTest::qWait(100);

    AlbumListView *albumListView = w->findChild<AlbumListView *>(AC_albumListView);
    QPoint posDclick(87, 79);
    QTest::mouseMove(albumListView->viewport(), posDclick, 10);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, posDclick, 10);
    event.addMousePress(Qt::MouseButton::LeftButton, Qt::NoModifier, posDclick, 10);
    event.addMouseDClick(Qt::MouseButton::LeftButton, Qt::NoModifier, posDclick, 10);
    event.simulate(albumListView->viewport());
    event.clear();
    QTest::qWait(1000);
    CommonService::getInstance()->setIsTabletEnvironment(false);
}

TEST(Application, tabletAlbumInconListDoubleClick)
{
    TEST_CASE_NAME("tabletAlbumInconDoubleClick")
    CommonService::getInstance()->setIsTabletEnvironment(true);

    MainFrame *w = Application::getInstance()->getMainWindow();
    MusicBaseListView *baseListView = w->findChild<MusicBaseListView *>(AC_dataBaseListview);
    QTest::qWait(50);
    // 点击专辑
    QPoint pos(130, 20);
    QTestEventList event;
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.simulate(baseListView->viewport());
    event.clear();
    DToolButton *listModeBtn = w->findChild<DToolButton *>(AC_btlistMode);
    if (listModeBtn != nullptr) {
        QTest::mousePress(listModeBtn, Qt::LeftButton, Qt::NoModifier, QPoint(20, 20), 10);
        QTest::mouseRelease(listModeBtn, Qt::LeftButton, Qt::NoModifier, QPoint(20, 20), 10);
    }
    QTest::qWait(100);

    AlbumListView *albumListView = w->findChild<AlbumListView *>(AC_albumListView);
    QTest::mouseMove(albumListView->viewport(), pos, 10);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.addMousePress(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.addMouseDClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.simulate(albumListView->viewport());
    event.clear();
    QTest::qWait(1000);
    CommonService::getInstance()->setIsTabletEnvironment(false);
}
