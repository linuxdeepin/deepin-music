




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


TEST(Application, dataBaseService)
{
    TEST_CASE_NAME("dataBaseService")

//    MainFrame *w = Application::getInstance()->getMainWindow();
//    MusicBaseListView *baseListView = w->findChild<MusicBaseListView *>(AC_dataBaseListview);

//    QTest::qWait(500);
////    // todo
////    DataBaseService::getInstance()->customSongList();
////    DataBaseService::getInstance()->allMusicInfosCount();
////    DataBaseService::getInstance()->getDatabase();
////    DataBaseService::getInstance()->slotGetAllMediaMetaFromThread((QList<MediaMeta>()));

//    emit DataBaseService::getInstance()->sigGetAllMediaMeta();
//    emit DataBaseService::getInstance()->sigImportMedias(QStringList());
//    emit DataBaseService::getInstance()->sigImportFinished("all");
//    QTest::qWait(500);
}



