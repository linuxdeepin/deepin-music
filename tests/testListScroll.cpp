#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include "application.h"

#include <QTest>
#include <QDebug>
#include <QPoint>
#include <QTimer>
#include <infodialog.h>
#include <DApplication>
#include <DIconButton>

#include "ac-desktop-define.h"

#include "mainframe.h"
#include "playlistview.h"
#include "musicsonglistview.h"
#include "musiclyricwidget.h"
#include "lrc/lyriclabel.h"
#include "musicbaselistview.h"
#include "waveform.h"
#include "musiclistscrollarea.h"


TEST(Application, testListScroll)
{
    TEST_CASE_NAME("testListScroll")

    MainFrame *w = Application::getInstance()->getMainWindow();
    MusicBaseListView *baseListView = w->findChild<MusicBaseListView *>(AC_dataBaseListview);
    MusicSongListView *songListView = w->findChild<MusicSongListView *>(AC_customizeListview);

    QTest::qWait(100);
    QTestEventList event;
    QPoint pos = QPoint(20, 100);
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.simulate(baseListView->viewport());
    event.clear();

    // 上方菜单
    QTimer::singleShot(200, w, [ = ]() {
        QTestEventList event;
        QTest::qWait(100);
        DMenu *menuWidget = static_cast<DMenu *>(qApp->activePopupWidget());
        event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
        event.addKeyClick(Qt::Key_Enter, Qt::NoModifier, 50);
        event.simulate(menuWidget);
        event.clear();
    });

    // 点击所有音乐
    QTest::qWait(50);
    pos = QPoint(20, 20);
    baseListView->setFocus();
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos);
    event.addKeyClick(Qt::Key_M, Qt::ControlModifier, 10);
    event.simulate(baseListView);
    event.clear();
    QTest::qWait(250);
}

TEST(Application, testListScroll1)
{
    TEST_CASE_NAME("testListScroll1")

//    MainFrame *w = Application::getInstance()->getMainWindow();
//    MusicBaseListView *baseListView = w->findChild<MusicBaseListView *>(AC_dataBaseListview);
//    MusicSongListView *songListView = w->findChild<MusicSongListView *>(AC_customizeListview);

//    QTestEventList event;
//    QPoint pos = QPoint(20, 100);
//    // 下方菜单
//    // 新建歌单
//    QTest::qWait(50);
//    event.addKeyClick(Qt::Key_N, Qt::ControlModifier | Qt::ShiftModifier, 10);
//    event.simulate(w);
//    event.clear();

//    // 重命名
//    QTimer::singleShot(300, w, [ = ]() {
//        QTestEventList event;
//        QTest::qWait(100);
//        DMenu *menuWidget = static_cast<DMenu *>(qApp->activePopupWidget());
//        event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
//        event.addKeyClick(Qt::Key_Enter, Qt::NoModifier, 50);
//        event.addDelay(100);
//        event.simulate(menuWidget);
//        event.clear();
//        QTest::qWait(100);
//        baseListView->setFocus();
//    });
//    QTest::qWait(50);
//    songListView->setFocus();
//    event.addMouseMove(pos);
//    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos);
//    event.addKeyClick(Qt::Key_M, Qt::ControlModifier, 10);
//    event.simulate(songListView);
//    event.clear();
//    QTest::qWait(550);
}


TEST(Application, testListScroll2)
{
    TEST_CASE_NAME("testListScrol2")

    MainFrame *w = Application::getInstance()->getMainWindow();
    MusicSongListView *songListView = w->findChild<MusicSongListView *>(AC_customizeListview);

    QTestEventList event;
    QPoint pos = QPoint(20, 100);
    // 下方菜单
    // 新建歌单
    QTest::qWait(50);
    event.addKeyClick(Qt::Key_N, Qt::ControlModifier | Qt::ShiftModifier, 10);
    event.simulate(w);
    event.clear();

    // 删除
    QTimer::singleShot(300, w, [ = ]() {
        QTimer::singleShot(500, w, [ = ]() {
            QTest::qWait(50);
            QTestEventList event;
            DDialog *messageBox = w->findChild<DDialog *>(AC_MessageBox);
            if (messageBox) {
                event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
                event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
                event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
                event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
                event.addKeyClick(Qt::Key_Enter, Qt::NoModifier, 50);
                event.simulate(messageBox);
                event.clear();
            }
        });

        QTestEventList event;
        DMenu *menuWidget = static_cast<DMenu *>(qApp->activePopupWidget());
        if (menuWidget) {
            event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
            event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
            event.addKeyClick(Qt::Key_Enter, Qt::NoModifier, 50);
            event.simulate(menuWidget);
            event.clear();
            QTest::qWait(100);
        }
    });

    if (songListView) {
        QTest::qWait(50);
//        songListView->setFocus();
//        event.addMouseMove(pos);
//        event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos);
//        event.addKeyClick(Qt::Key_M, Qt::ControlModifier, 10);
//        event.simulate(songListView);
//        event.clear();

        songListView->rmvSongList();
    }

    QTest::qWait(500);
}

