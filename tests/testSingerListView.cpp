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

#include "ac-desktop-define.h"

#include "mainframe.h"
#include "musicbaselistview.h"
#include "playlistview.h"
#include "musicsonglistview.h"
#include "singerlistview.h"
#include "musiclistdatawidget.h"
#include "subsonglistwidget.h"


TEST(Application, singerListViewDrag)
{
    TEST_CASE_NAME("singerListViewDrag")

    MainFrame *w = Application::getInstance()->getMainWindow();
    MusicBaseListView *baseListView = w->findChild<MusicBaseListView *>(AC_dataBaseListview);

    // 点击歌手
    QPoint pos = QPoint(20, 50);
    QTestEventList event;
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.addMousePress(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.addMouseDClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.simulate(baseListView->viewport());
    event.clear();

    QTest::qWait(100);
    SingerListView *slv = w->findChild<SingerListView *>(AC_singerListView);

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
    qApp->sendEvent(slv->viewport(), &eEnter);

    QDragMoveEvent eMove(pos, Qt::IgnoreAction, &mimedata, Qt::LeftButton, Qt::NoModifier);
    qApp->sendEvent(slv->viewport(), &eMove);

    QDropEvent e(pos, Qt::IgnoreAction, &mimedata, Qt::LeftButton, Qt::NoModifier);
    qApp->sendEvent(slv->viewport(), &e);

    QTest::qWait(100);
}

TEST(Application, singerDataDelegateEdit)
{
    TEST_CASE_NAME("singerDataDelegateEdit")
    MainFrame *w = Application::getInstance()->getMainWindow();
    MusicBaseListView *baseListView = w->findChild<MusicBaseListView *>(AC_dataBaseListview);

    // 点击歌手
    QTestEventList event;
    event.addMouseMove(QPoint(20, 50));
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, QPoint(20, 50), 10);
    event.addMousePress(Qt::MouseButton::LeftButton, Qt::NoModifier, QPoint(20, 50), 10);
    event.addMouseDClick(Qt::MouseButton::LeftButton, Qt::NoModifier, QPoint(20, 50), 10);
    event.simulate(baseListView->viewport());
    event.clear();
    QTest::qWait(100);

    DToolButton *iconModeBtn = w->findChild<DToolButton *>(AC_btIconMode);
    if (iconModeBtn != nullptr) {
        QTest::mousePress(iconModeBtn, Qt::LeftButton, Qt::NoModifier, QPoint(20, 20), 10);
        QTest::mouseRelease(iconModeBtn, Qt::LeftButton, Qt::NoModifier, QPoint(20, 20), 10);
    }
    QTest::qWait(100);

    SingerListView *singerListView = w->findChild<SingerListView *>(AC_singerListView);
    QPoint pos(87, 78);
    QTest::mouseMove(singerListView->viewport(), pos, 10);
    QTest::mouseClick(singerListView->viewport(), Qt::LeftButton, Qt::NoModifier, pos, 10);
    QTest::qWait(1000);
}

TEST(Application, singerDataDelegateEdit1)
{
    TEST_CASE_NAME("singerDataDelegateEdit")
    MainFrame *w = Application::getInstance()->getMainWindow();
    MusicBaseListView *baseListView = w->findChild<MusicBaseListView *>(AC_dataBaseListview);

    // 点击歌手
    QTestEventList event;
    event.addMouseMove(QPoint(20, 50));
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, QPoint(20, 50), 10);
    event.addMousePress(Qt::MouseButton::LeftButton, Qt::NoModifier, QPoint(20, 50), 10);
    event.addMouseDClick(Qt::MouseButton::LeftButton, Qt::NoModifier, QPoint(20, 50), 10);
    event.simulate(baseListView->viewport());
    event.clear();
    QTest::qWait(100);

    DToolButton *iconModeBtn = w->findChild<DToolButton *>(AC_btIconMode);
    if (iconModeBtn != nullptr) {
        QTest::mousePress(iconModeBtn, Qt::LeftButton, Qt::NoModifier, QPoint(20, 20), 10);
        QTest::mouseRelease(iconModeBtn, Qt::LeftButton, Qt::NoModifier, QPoint(20, 20), 10);
    }
    QTest::qWait(100);

    SingerListView *singerListView = w->findChild<SingerListView *>(AC_singerListView);
    QPoint pos(87, 79);
    QTest::mouseMove(singerListView->viewport(), pos, 10);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.addMousePress(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.addMouseDClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.simulate(singerListView->viewport());
    event.clear();
    QTest::qWait(1000);
}

// 平板模式下歌手页面Icon模式显示
TEST(Application, tabletSinger)
{
    TEST_CASE_NAME("tabletSinger")
    CommonService::getInstance()->setIsTabletEnvironment(true);

    MainFrame *w = Application::getInstance()->getMainWindow();
    MusicBaseListView *baseListView = w->findChild<MusicBaseListView *>(AC_dataBaseListview);
    QTest::qWait(50);
    // 点击歌手
    QPoint pos(130, 60);
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

// 平板模式下歌手页面Icon模式下的单击操作
TEST(Application, tabletSingerInconClick)
{
    TEST_CASE_NAME("tabletSingerInconClick")
    CommonService::getInstance()->setIsTabletEnvironment(true);

    MainFrame *w = Application::getInstance()->getMainWindow();
    MusicBaseListView *baseListView = w->findChild<MusicBaseListView *>(AC_dataBaseListview);
    QTest::qWait(50);
    // 点击歌手
    QPoint pos(130, 60);
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

    SingerListView *singerListView = w->findChild<SingerListView *>(AC_singerListView);
    QPoint posClick(87, 78);
    QTest::mouseMove(singerListView->viewport(), posClick, 10);
    QTest::mouseClick(singerListView->viewport(), Qt::LeftButton, Qt::NoModifier, posClick, 10);
    QTest::qWait(1000);
    CommonService::getInstance()->setIsTabletEnvironment(false);
}

// 平板模式下歌手页面Icon模式下的双击操作
TEST(Application, tabletSingerInconDoubleClick)
{
    TEST_CASE_NAME("tabletSingerInconDoubleClick")
    CommonService::getInstance()->setIsTabletEnvironment(true);

    MainFrame *w = Application::getInstance()->getMainWindow();
    MusicBaseListView *baseListView = w->findChild<MusicBaseListView *>(AC_dataBaseListview);
    QTest::qWait(50);
    // 点击歌手
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

    SingerListView *singerListView = w->findChild<SingerListView *>(AC_singerListView);
    QPoint posDclick(87, 79);
    QTest::mouseMove(singerListView->viewport(), posDclick, 10);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, posDclick, 10);
    event.addMousePress(Qt::MouseButton::LeftButton, Qt::NoModifier, posDclick, 10);
    event.addMouseDClick(Qt::MouseButton::LeftButton, Qt::NoModifier, posDclick, 10);
    event.simulate(singerListView->viewport());
    event.clear();
    QTest::qWait(1000);
    CommonService::getInstance()->setIsTabletEnvironment(false);
}

TEST(Application, tabletSingerInconListDoubleClick)
{
    TEST_CASE_NAME("tabletSingerInconListDoubleClick")
    CommonService::getInstance()->setIsTabletEnvironment(true);

    MainFrame *w = Application::getInstance()->getMainWindow();
    MusicBaseListView *baseListView = w->findChild<MusicBaseListView *>(AC_dataBaseListview);
    QTest::qWait(50);
    // 点击歌手
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

    SingerListView *singerListView = w->findChild<SingerListView *>(AC_singerListView);
    QTest::mouseMove(singerListView->viewport(), pos, 10);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.addMousePress(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.addMouseDClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.simulate(singerListView->viewport());
    event.clear();
    QTest::qWait(1000);
    CommonService::getInstance()->setIsTabletEnvironment(false);
}
