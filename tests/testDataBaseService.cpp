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
#include "databaseservice.h"

TEST(Application, dataBaseService)
{
    TEST_CASE_NAME("dataBaseService")

    MainFrame *w = Application::getInstance()->getMainWindow();
//    MusicBaseListView *baseListView = w->findChild<MusicBaseListView *>(AC_dataBaseListview);

    QTest::qWait(50);

    DataBaseService::getInstance()->getMusicInfoByHash("all");
    DataBaseService::getInstance()->getCustomSongList();
//    DataBaseService::getInstance()->getDatabase();
//    DataBaseService::getInstance()->getImportStatus();

    QList<MediaMeta> metaList;
    metaList.append(MediaMeta());
    DataBaseService::getInstance()->addMetaToPlaylist("all", metaList);
    DataBaseService::getInstance()->updateMetaCodec(MediaMeta());
    DataBaseService::getInstance()->setFirstSong("");
    DataBaseService::getInstance()->updatePlaylistDisplayName("新建歌单", "");
    DataBaseService::getInstance()->signalCoverUpdate(Player::getInstance()->getActiveMeta());

    QTest::qWait(50);
}



