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


TEST(Application, btLyric)
{
    TEST_CASE_NAME("btLyric")

    MainFrame *w = Application::getInstance()->getMainWindow();
    MusicBaseListView *baseListView = w->findChild<MusicBaseListView *>(AC_dataBaseListview);

    QTest::qWait(50);
    QPoint pos = QPoint(20, 120);
    QTestEventList event;

    // 点击所有音乐
    QTest::qWait(100);
    pos = QPoint(130, 100);
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.simulate(baseListView->viewport());
    event.clear();

    // 双击list
    pos = QPoint(20, 80);
    PlayListView *plv = w->findChild<PlayListView *>(AC_PlayListView);
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.addMousePress(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.addMouseDClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.simulate(plv->viewport());
    event.clear();

    Waveform *wf = w->findChild<Waveform *>(AC_Waveform);
    QTest::qWait(50);
    // 点击进度条
    pos = QPoint(40, 20);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
    event.addMouseMove(pos);
    event.simulate(wf);
    event.clear();


    // 点击歌词按钮
    DIconButton *bt = w->findChild<DIconButton *>(AC_Lyric);
    QTest::qWait(50);
    pos = QPoint(20, 20);
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 50);
    event.simulate(bt);
    event.clear();

    QTest::qWait(500);

    // 滚动歌词
    AbstractWheelWidget *lv = w->findChild<AbstractWheelWidget *>(AC_lyricview);

    QTestEventList event1;
    pos = QPoint(280, 440);
    QWheelEvent wheel(pos, -120, Qt::MouseButton::MidButton, Qt::NoModifier);
    lv->event(&wheel);
    QTest::qWait(500);
    QWheelEvent wheel1(pos, 120, Qt::MouseButton::MidButton, Qt::NoModifier);
    lv->event(&wheel1);
    QTest::qWait(500);

    MusicLyricWidget *mlw = w->findChild<MusicLyricWidget *>(AC_musicLyricWidget);
    mlw->updateUI();


    // 点击歌词按钮
    QTestEventList event2;
    event2.addMouseMove(QPoint(20, 20));
    event2.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, QPoint(20, 20), 50);
    event2.simulate(bt);
    event2.clear();
    QTest::qWait(500);
}
