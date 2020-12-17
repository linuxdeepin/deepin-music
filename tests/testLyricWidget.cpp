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


TEST(Application, btLyric)
{
    TEST_CASE_NAME("btLyric")

    MainFrame *w = Application::getInstance()->getMainWindow();

    QTest::qWait(50);
    QPoint pos = QPoint(20, 120);
    QTestEventList event;

    // 双击list
    pos = QPoint(20, 120);
    PlayListView *plv = w->findChild<PlayListView *>(AC_PlayListView);
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.addMousePress(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.addMouseDClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.simulate(plv->viewport());
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
    QWheelEvent wheel1(pos, 120, Qt::MouseButton::MidButton, Qt::NoModifier);
    lv->event(&wheel1);

    MusicLyricWidget *mlw = w->findChild<MusicLyricWidget *>(AC_musicLyricWidget);
    mlw->updateUI();


    // 点击歌词按钮
    QTest::qWait(500);
    QTestEventList event2;
    event2.addMouseMove(QPoint(20, 20));
    event2.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, QPoint(20, 20), 50);
    event2.simulate(bt);
    event2.clear();
    QTest::qWait(500);
}
