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

    // 点击自定义列表
    QTest::qWait(100);
    QPoint pos(20, 20);
    QTestEventList event;
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

    pos = QPoint(20, 20);
    QDragEnterEvent eEnter(pos, Qt::IgnoreAction, &mimedata, Qt::LeftButton, Qt::NoModifier);
    qApp->sendEvent(songListView->viewport(), &eEnter);

    QDragMoveEvent eMove(pos, Qt::IgnoreAction, &mimedata, Qt::LeftButton, Qt::NoModifier);
    qApp->sendEvent(songListView->viewport(), &eMove);

    QDropEvent e(pos, Qt::IgnoreAction, &mimedata, Qt::LeftButton, Qt::NoModifier);
    qApp->sendEvent(songListView->viewport(), &e);

    QTest::qWait(100);
}


