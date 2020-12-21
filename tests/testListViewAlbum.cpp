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

    pos = QPoint(130, 130);

    QDragEnterEvent eEnter(pos, Qt::IgnoreAction, &mimedata, Qt::LeftButton, Qt::NoModifier);
    qApp->sendEvent(alv->viewport(), &eEnter);

    QDragMoveEvent eMove(pos, Qt::IgnoreAction, &mimedata, Qt::LeftButton, Qt::NoModifier);
    qApp->sendEvent(alv->viewport(), &eMove);

    QDropEvent e(pos, Qt::IgnoreAction, &mimedata, Qt::LeftButton, Qt::NoModifier);
    qApp->sendEvent(alv->viewport(), &e);

    QTest::qWait(100);
}


