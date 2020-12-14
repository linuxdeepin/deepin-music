
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
#include "albumlistview.h"
#include "musiclistinfoview.h"
#include "musiclistdialog.h"


TEST(Application, musicListDialg)
{
    TEST_CASE_NAME("musicListDialg")

    MainFrame *w = Application::getInstance()->getMainWindow();
    MusicBaseListView *baseListView = w->findChild<MusicBaseListView *>(AC_dataBaseListview);
    QTest::qWait(500);
    // 点击专辑
    QPoint pos(130, 20);
    QTestEventList event;
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
    event.simulate(baseListView->viewport());
    event.clear();



//    // dialog list 点击
//    QTest::qWait(500);
//    QTimer::singleShot(1500, w, [ = ]() {
//        MusicListDialog *mld = w->findChild<MusicListDialog *>(AC_musicListDialogAlbum);
//        MusicListInfoView *mliv = w->findChild<MusicListInfoView *>(AC_musicListInfoView);

////        QPoint pos(130, 20);
//        QTestEventList event;
//        event.addMouseMove(pos);
//        event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
//        event.addMousePress(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
//        event.addMouseDClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
//        event.simulate(mliv->viewport());
//        event.clear();

////        QContextMenuEvent menuEvent(QContextMenuEvent::Reason::Other, pos);
////        qApp->sendEvent(mld, &menuEvent);


//        QTest::qWait(1000);
//        mld->close();
//    });

    // 双击list
    pos = QPoint(20, 20);
    AlbumListView *alv = w->findChild<AlbumListView *>(AC_albumListView);
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
    event.addMousePress(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
    event.addMouseDClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
    event.simulate(alv->viewport());
    event.clear();

    QTest::qWait(1000);
}



