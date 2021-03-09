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

//#include <gtest/gtest.h>
//#include <gmock/gmock-matchers.h>
//#include "application.h"

//#include <QTest>
//#include <QDebug>
//#include <QPoint>
//#include <QTimer>
//#include <infodialog.h>
//#include <DApplication>
//#include <DIconButton>
//#include <QStandardPaths>
//#include <DPushButton>
//#define protected public
//#include <DFileDialog>

//#include "ac-desktop-define.h"

//#include "mainframe.h"
//#include "playlistview.h"
//#include "musicsonglistview.h"
//#include "musiclyricwidget.h"
//#include "musicbaselistview.h"
//#include "lrc/lyriclabel.h"
//#include "importwidget.h"
//#include "musicsettings.h"


//// 删除所有音乐
//TEST(Application, importWidget)
//{
//    TEST_CASE_NAME("importWidget")

//    MainFrame *w = Application::getInstance()->getMainWindow();
//    MusicBaseListView *baseListView = w->findChild<MusicBaseListView *>(AC_dataBaseListview);

//    QPoint pos = QPoint(130, 30);
//    QTestEventList event;

//    // 点击专辑
//    pos = QPoint(130, 30);
//    event.addMouseMove(pos);
//    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
//    event.simulate(baseListView->viewport());
//    event.clear();

//    // 点击所有音乐
//    QTest::qWait(100);
//    pos = QPoint(130, 100);
//    event.addMouseMove(pos);
//    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
//    event.simulate(baseListView->viewport());
//    event.clear();


//    // dialog list 点击
//    QTest::qWait(50);
//    QTimer::singleShot(200, w, [ = ]() {
//        // 清空ListView
//        QTest::qWait(50);
//        DDialog *messageBox = w->findChild<DDialog *>("MessageBox");
//        if (messageBox) {
//            QPoint pos = QPoint(130, 150);
//            QTestEventList event;
//            event.addMouseMove(pos);
//            event.addKeyClick(Qt::Key::Key_Tab, Qt::NoModifier, 10);
//            event.addKeyClick(Qt::Key::Key_Tab, Qt::NoModifier, 10);
//            event.addKeyClick(Qt::Key::Key_Tab, Qt::NoModifier, 10);
//            event.addKeyClick(Qt::Key::Key_Tab, Qt::NoModifier, 10);
//            event.addKeyClick(Qt::Key::Key_Enter, Qt::NoModifier, 50);
//            event.simulate(messageBox);
//            event.clear();
//        }
//    });

//    // 全选
//    pos = QPoint(20, 20);
//    PlayListView *plv = w->findChild<PlayListView *>(AC_PlayListView);
//    event.addMouseMove(pos);
//    event.addKeyClick(Qt::Key_A, Qt::ControlModifier, 10);
//    event.simulate(plv->viewport());
//    event.clear();
//    plv->slotRmvFromSongList();

//    QTest::qWait(500);
//}

//TEST(Application, importWidget3)
//{
//    TEST_CASE_NAME("importWidget")

//    MainFrame *w = Application::getInstance()->getMainWindow();
////    ImportWidget *importWidget = w->findChild<ImportWidget *>(AC_ImportWidget);

//    // fileDialog
//    QTimer::singleShot(300, w, [ = ]() {
//        DFileDialog *fileDialog = w->findChild<DFileDialog *>("fileDialogImport");
//        if (fileDialog) {
//            QTest::qWait(500);

//            QTestEventList event;
//            QPoint pos(280, 250);

////            event.addMouseMove(pos);
////            event.addKeyClick(Qt::Key_Enter, Qt::NoModifier);
////            event.addKeyPress(Qt::Key_Enter, Qt::NoModifier);
////            event.addKeyRelease(Qt::Key_Enter, Qt::NoModifier);
//////            event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
////            event.simulate(fileDialog);
////            event.clear();

////            QCloseEvent closeEvent;
////            qApp->sendEvent(fileDialog, &closeEvent);

////            fileDialog->done(0);
////            fileDialog->~DFileDialog();
//        }
//        qDebug() << "\n--------------- " << "进入" << " -----------------"
//                 << " Func:" << __FUNCTION__  << " Line:" << __LINE__ ;
//    });

//    DPushButton *importPath = w->findChild<DPushButton *>(AC_importButton);
//    QTestEventList event;
//    QPoint pos(20, 20);
//    event.addMouseMove(pos);
//    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
//    event.simulate(importPath);
//    event.clear();

//}

//TEST(Application, importWidget4)
//{
//    TEST_CASE_NAME("importWidget")
//    MainFrame *w = Application::getInstance()->getMainWindow();

//    // fileDialog
//    QTimer::singleShot(500, w, [ = ]() {
//        QTest::qWait(100);
//        DFileDialog *fileDialog = w->findChild<DFileDialog *>("fileDialogAdd");
//        if (fileDialog) {
//            QTest::qWait(300);
//            fileDialog->close();
//        }
//    });

//    DPushButton *importPath = w->findChild<DPushButton *>(AC_addMusicButton);
//    QTestEventList event;
//    QPoint pos(20, 20);
//    event.addMouseMove(pos);
//    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
//    event.simulate(importPath);
//    event.clear();

//    QTest::qWait(500);

//}

//TEST(Application, importWidget2)
//{
//    // 扫描歌曲
//    TEST_CASE_NAME("importWidget")

//    QTest::qWait(100);
//    MainFrame *w = Application::getInstance()->getMainWindow();
//    QLabel *ilt = w->findChild<QLabel *>(AC_importLinkText);
//    ilt->linkActivated("");
//    QTest::qWait(500);
//}
