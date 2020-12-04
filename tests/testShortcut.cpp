#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include "application.h"

#include <QTest>
#include <QDebug>
#include <QPoint>
#include <QTimer>
#include <infodialog.h>
#include <DApplication>

#include "ac-desktop-define.h"

#include "mainframe.h"
#include "musicbaselistview.h"
#include "playlistview.h"
#include "musicsonglistview.h"


TEST(Application, shortCut)
{
    TEST_CASE_NAME("shortCut")

    QTest::qWait(500);
    MainFrame *w = Application::getInstance()->getMainWindow();
    MusicBaseListView *baseListView = w->findChild<MusicBaseListView *>(AC_dataBaseListview);
    QTest::qWait(500);
    // 点击所有音乐
    QPoint pos(130, 100);
    QTestEventList event;
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
    event.simulate(baseListView->viewport());
    event.clear();

    // 双击list
    QTest::qWait(500);
    pos = QPoint(20, 20);
    PlayListView *plv = w->findChild<PlayListView *>(AC_PlayListView);
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
    event.addMousePress(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
    event.addMouseDClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
    event.simulate(plv->viewport());
    event.clear();


    // 空格
    QTest::qWait(1000);
    event.addKeyClick(Qt::Key_Space, Qt::NoModifier, 100);
    event.simulate(w);
    event.clear();
    QTest::qWait(1000);

    // 上一首
    QTest::qWait(500);
    event.addKeyClick(Qt::Key_Left, Qt::ControlModifier, 100);
    event.simulate(w);
    event.clear();

    // 下一首
    QTest::qWait(500);
    event.addKeyClick(Qt::Key_Right, Qt::ControlModifier, 100);
    event.simulate(w);
    event.clear();

    // todo 音量增大
    QTest::qWait(500);
    event.addKeyPress(Qt::Key_Up, Qt::ControlModifier | Qt::AltModifier, 100);
    event.simulate(w);
    event.clear();

    // todo 音量减小
    QTest::qWait(500);
    event.addKeyPress(Qt::Key_Down, Qt::ControlModifier | Qt::AltModifier, 100);
    event.simulate(w);
    event.clear();

    // todo 静音
    QTest::qWait(500);
    event.addKeyClick(Qt::Key_M, Qt::NoModifier, 100);
    event.simulate(w);
    event.clear();


    // todo 收藏未实现
    QTest::qWait(500);
    event.addKeyClick(Qt::Key_Dollar, Qt::NoModifier, 100);
    event.simulate(w);
    event.clear();

    // 从歌单中删除
    QTest::qWait(500);
    event.addKeyClick(Qt::Key_Delete, Qt::NoModifier, 100);
    event.simulate(w);
    event.clear();

    // 歌曲信息
    QTest::qWait(500);
    event.addKeyClick(Qt::Key_I, Qt::ControlModifier, 100);
    event.simulate(w);
    event.clear();

    // 关闭歌曲信息
    QTimer::singleShot(1000, w, [ = ]() {
        InfoDialog *infoDialog = w->findChild<InfoDialog *>(AC_infoDialog);

        QTest::qWait(500);
        infoDialog->close();
    });

    // 点击自定义列表
    QTest::qWait(500);
    MusicSongListView *songListView = w->findChild<MusicSongListView *>(AC_customizeListview);
    QTest::qWait(500);
    pos = QPoint(130, 20);
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
    event.simulate(songListView->viewport());
    event.clear();


    // 重命名歌单
    QTest::qWait(500);
    event.addKeyClick(Qt::Key_F2, Qt::NoModifier, 100);
    event.addKeyClick(Qt::Key_W, Qt::NoModifier, 100);
    event.addKeyClick(Qt::Key_Enter, Qt::NoModifier, 100);
    event.simulate(w);
    event.clear();

    // 新建歌单
    QTest::qWait(500);
    event.addKeyClick(Qt::Key_N, Qt::ControlModifier | Qt::ShiftModifier, 100);
    event.simulate(w);
    event.clear();

//    // 帮助
//    QTest::qWait(500);
//    event.addKeyClick(Qt::Key_F1, Qt::NoModifier, 100);
//    event.simulate(w);
//    event.clear();

    // 快捷键预览
    QTest::qWait(500);
    event.addKeyClick(Qt::Key_Slash, Qt::ControlModifier | Qt::ShiftModifier, 100);
    event.simulate(w);
    event.clear();

    QTest::qWait(1000);
}



