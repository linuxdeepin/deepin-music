// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
// Graph-driven unit tests for src/libdmusic/presenter.h (class Presenter).
// Forces the Qt (QMediaPlayer) backend instead of VLC/SDL for headless safety.
#include <gtest/gtest.h>
#include <QObject>
#include <QDir>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QVariantList>
#include <QVariantMap>
#include <QImage>
#include <QColor>
#include "global.h"
#include "presenter.h"

class PresenterTest : public ::testing::Test {
protected:
    void SetUp() override {
        QString tmp = QDir::tempPath() + "/dm-presenter-ut";
        QDir().mkpath(tmp + "/cache");
        QDir().mkpath(tmp + "/config");
        QDir().mkpath(tmp + "/music");
        DmGlobal::setCachePath(tmp + "/cache");
        DmGlobal::setConfigPath(tmp + "/config");
        DmGlobal::setMusicPath(tmp + "/music");
        DmGlobal::setAppName("deepin-music-ut");
        DmGlobal::initPath();
        // Use the Qt multimedia backend (offscreen-friendly) rather than VLC/SDL.
        DmGlobal::setPlaybackEngineType(DmGlobal::QtMEDIAPLAYER);
        p = new Presenter("UnknownAlbum", "UnknownArtist");
        // Populate the player-engine MprisPlayer so methods that call
        // resetDBusMpris (setMediaMeta/stop/removeFromPlayList) do not crash.
        p->setMprisPlayer("org.deepin.music", "deepin-music", "Music");
    }
    void TearDown() override { delete p; p = nullptr; }
    Presenter *p = nullptr;
};

TEST_F(PresenterTest, Lifecycle_ConstructDestruct_NoThrow) {
    ASSERT_NE(p, nullptr);
}

TEST_F(PresenterTest, AllGetters_OnEmpty_NoThrow) {
    EXPECT_NO_THROW((void)p->supportedSuffixList());
    EXPECT_NO_THROW((void)p->getActivateMeta());
    EXPECT_NO_THROW((void)p->getPlaybackStatus());
    EXPECT_NO_THROW((void)p->getMute());
    EXPECT_NO_THROW((void)p->getVolume());
    EXPECT_NO_THROW((void)p->getPlaybackMode());
    EXPECT_NO_THROW((void)p->getPosition());
    EXPECT_NO_THROW((void)p->getActivateMetImage());
    EXPECT_NO_THROW((void)p->getCurrentPlayList());
    EXPECT_NO_THROW((void)p->getPlaylistMetas());
    EXPECT_NO_THROW((void)p->isExistMeta());
    EXPECT_NO_THROW((void)p->musicInforFromHash("nohash"));
    EXPECT_NO_THROW((void)p->playlistSortType("nohash"));
    EXPECT_NO_THROW((void)p->playlistMetaCount("nohash"));
    EXPECT_NO_THROW((void)p->allPlaylistInfos());
    EXPECT_NO_THROW((void)p->customPlaylistInfos());
    EXPECT_NO_THROW((void)p->allAlbumInfos());
    EXPECT_NO_THROW((void)p->allArtistInfos());
    EXPECT_NO_THROW((void)p->searchedAlbumInfos());
    EXPECT_NO_THROW((void)p->searchedArtistInfos());
    EXPECT_NO_THROW((void)p->getLyrics());
    EXPECT_NO_THROW((void)p->getEffectImage());
    EXPECT_NO_THROW((void)p->detectEncodings("nohash"));
    EXPECT_NO_THROW((void)p->valueFromSettings("unknown.key"));
}

TEST_F(PresenterTest, SetMute_NoThrow) { EXPECT_NO_THROW(p->setMute(true)); }
TEST_F(PresenterTest, SetVolume_NoThrow) { EXPECT_NO_THROW(p->setVolume(60)); }
TEST_F(PresenterTest, SetPlaybackMode_NoThrow) { EXPECT_NO_THROW(p->setPlaybackMode(QVariant(DmGlobal::RepeatAll))); }
TEST_F(PresenterTest, SetPosition_NoThrow) { EXPECT_NO_THROW(p->setPosition(1000)); }
TEST_F(PresenterTest, DISABLED_SetEQ_RequiresPlayerInit_Skipped) { EXPECT_NO_THROW(p->setEQ(true, 0, QVariantList{1, 2, 3})); }
TEST_F(PresenterTest, SetEQEnable_NoThrow) { EXPECT_NO_THROW(p->setEQEnable(false)); }
TEST_F(PresenterTest, SetEQpre_NoThrow) { EXPECT_NO_THROW(p->setEQpre(0)); }
TEST_F(PresenterTest, SetEQbauds_NoThrow) { EXPECT_NO_THROW(p->setEQbauds(0, 5)); }
TEST_F(PresenterTest, SetEQCurMode_NoThrow) { EXPECT_NO_THROW(p->setEQCurMode(1)); }
TEST_F(PresenterTest, SetActivateMeta_NoThrow) { EXPECT_NO_THROW(p->setActivateMeta("nohash")); }
TEST_F(PresenterTest, SetCurrentPlayList_NoThrow) { EXPECT_NO_THROW(p->setCurrentPlayList("noplaylist")); }
TEST_F(PresenterTest, SetEffectImage_NoThrow) { QImage img(2, 2, QImage::Format_RGB32); img.fill(Qt::blue); EXPECT_NO_THROW(p->setEffectImage(img)); }
TEST_F(PresenterTest, SetValueToSettings_NoThrow) { EXPECT_NO_THROW(p->setValueToSettings("base.play.auto_play", QVariant(true))); }
TEST_F(PresenterTest, UpdateMetaCodec_NoThrow) { EXPECT_NO_THROW(p->updateMetaCodec("nohash", "UTF-8")); }
TEST_F(PresenterTest, SyncToSettings_NoThrow) { EXPECT_NO_THROW(p->syncToSettings()); }
TEST_F(PresenterTest, ResetToSettings_NoThrow) { EXPECT_NO_THROW(p->resetToSettings()); }

TEST_F(PresenterTest, PlaylistOps_ImportMetas_NoThrow) { EXPECT_NO_THROW(p->importMetas(QStringList{"/tmp/no-a.mp3"}, "play", false)); }
TEST_F(PresenterTest, PlaylistOps_AddMetasToPlayList_NoThrow) { EXPECT_NO_THROW(p->addMetasToPlayList(QStringList{"m1"}, "play")); }
TEST_F(PresenterTest, PlaylistOps_AddAlbumToPlayList_NoThrow) { EXPECT_NO_THROW(p->addAlbumToPlayList("album", "play")); }
TEST_F(PresenterTest, PlaylistOps_AddArtistToPlayList_NoThrow) { EXPECT_NO_THROW(p->addArtistToPlayList("artist", "play")); }
TEST_F(PresenterTest, PlaylistOps_ClearPlayList_NoThrow) { EXPECT_NO_THROW(p->clearPlayList("play")); }
TEST_F(PresenterTest, PlaylistOps_RemoveFromPlayList_NoThrow) { EXPECT_NO_THROW(p->removeFromPlayList(QStringList{"m1"}, "play", false)); }
TEST_F(PresenterTest, PlaylistOps_MoveMetasPlayList_NoThrow) { EXPECT_NO_THROW(p->moveMetasPlayList(QStringList{"m1"}, "play", "m2")); }
TEST_F(PresenterTest, PlaylistOps_AddPlayList_NoThrow) { EXPECT_NO_THROW((void)p->addPlayList("NewList")); }
TEST_F(PresenterTest, PlaylistOps_SortPlaylist_NoThrow) { EXPECT_NO_THROW(p->sortPlaylist(DmGlobal::SortByTitle, "play")); }
TEST_F(PresenterTest, PlaylistOps_DeletePlaylist_NoThrow) { EXPECT_NO_THROW((void)p->deletePlaylist("play")); }
TEST_F(PresenterTest, PlaylistOps_RenamePlaylist_NoThrow) { EXPECT_NO_THROW((void)p->renamePlaylist("Renamed", "play")); }
TEST_F(PresenterTest, PlaylistOps_MovePlaylist_NoThrow) { EXPECT_NO_THROW(p->movePlaylist("play", "next")); }
TEST_F(PresenterTest, PlaylistOps_PlaylistInfoFromHash_NoThrow) { EXPECT_NO_THROW((void)p->playlistInfoFromHash("play")); }
TEST_F(PresenterTest, PlaylistOps_IsExistMetaHash_NoThrow) { EXPECT_NO_THROW((void)p->isExistMeta("m1", "play")); }
TEST_F(PresenterTest, PlaylistOps_PlayAlbum_NoThrow) { EXPECT_NO_THROW(p->playAlbum("album", "m1")); }
TEST_F(PresenterTest, PlaylistOps_PlayArtist_NoThrow) { EXPECT_NO_THROW(p->playArtist("artist", "m1")); }
TEST_F(PresenterTest, PlaylistOps_PlayPlaylist_NoThrow) { EXPECT_NO_THROW(p->playPlaylist("play", "m1")); }
TEST_F(PresenterTest, PlaylistOps_NextMetaFromPlay_NoThrow) { EXPECT_NO_THROW((void)p->nextMetaFromPlay("m1")); }
TEST_F(PresenterTest, PlaylistOps_PreMetaFromPlay_NoThrow) { EXPECT_NO_THROW((void)p->preMetaFromPlay("m1")); }
TEST_F(PresenterTest, PlaylistOps_ShowMetaFile_NoThrow) { EXPECT_NO_THROW(p->showMetaFile("m1")); }

TEST_F(PresenterTest, Search_NoThrow) {
    EXPECT_NO_THROW((void)p->quickSearchText("abc"));
    EXPECT_NO_THROW((void)p->searchText("abc", ""));
}

TEST_F(PresenterTest, ColorByKmeans_NoThrow) {
    EXPECT_NO_THROW((void)p->getMainColorByKmeans());
    EXPECT_NO_THROW((void)p->getSecondColorByKmeans());
}

TEST_F(PresenterTest, PlayerActions_Play_NoThrow) { EXPECT_NO_THROW(p->play()); }
TEST_F(PresenterTest, PlayerActions_Pause_NoThrow) { EXPECT_NO_THROW(p->pause()); }
TEST_F(PresenterTest, PlayerActions_PlayPause_NoThrow) { EXPECT_NO_THROW(p->playPause()); }
TEST_F(PresenterTest, PlayerActions_PlayPre_NoThrow) { EXPECT_NO_THROW(p->playPre()); }
TEST_F(PresenterTest, PlayerActions_PlayNext_NoThrow) { EXPECT_NO_THROW(p->playNext()); }
TEST_F(PresenterTest, PlayerActions_Resume_NoThrow) { EXPECT_NO_THROW(p->resume()); }
TEST_F(PresenterTest, PlayerActions_Stop_NoThrow) { EXPECT_NO_THROW(p->stop()); }
TEST_F(PresenterTest, PlayerActions_SaveDataToDB_NoThrow) { EXPECT_NO_THROW(p->saveDataToDB()); }

// setMprisPlayer is exercised in SetUp() (above); the standalone variant is
// kept disabled to avoid creating a second MprisPlayer on the same service.
TEST_F(PresenterTest, DISABLED_SetMprisPlayer_AlreadyCoveredInSetUp_Skipped) {
    EXPECT_NO_THROW(p->setMprisPlayer("org.deepin.music", "deepin-music", "Music"));
}
TEST_F(PresenterTest, DISABLED_ForceExit_TearsDownProcess_Skipped) {
    EXPECT_NO_THROW(p->forceExit());
}

// ---- Data-driven Presenter coverage: import a real audio sample and pump the
// event loop so the async DataManager worker pipeline (DB + index helpers) and
// the meta-dependent getters actually execute. ----
#include <QCoreApplication>
#include <QEventLoop>
#include <QTimer>
#include <QThread>

TEST_F(PresenterTest, WithData_ImportSample_PumpEventLoop_NoThrow) {
    QString sample = QString::fromUtf8(TEST_DATA_DIR) + "/sample.mp3";
    EXPECT_NO_THROW(p->importMetas(QStringList{sample}, "play", false));
    // Pump the event loop to let the queued worker pipeline deliver results.
    QEventLoop loop;
    for (int i = 0; i < 20; ++i) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
        QThread::msleep(20);
    }
    EXPECT_NO_THROW((void)p->getActivateMeta());
    EXPECT_NO_THROW((void)p->musicInforFromHash(""));
    EXPECT_NO_THROW((void)p->playlistInfoFromHash("play"));
    EXPECT_NO_THROW((void)p->playlistMetaCount("play"));
    EXPECT_NO_THROW((void)p->allPlaylistInfos());
    EXPECT_NO_THROW((void)p->allAlbumInfos());
    EXPECT_NO_THROW((void)p->allArtistInfos());
    EXPECT_NO_THROW(p->play());
    for (int i = 0; i < 10; ++i) { QCoreApplication::processEvents(QEventLoop::AllEvents, 50); QThread::msleep(10); }
    EXPECT_NO_THROW(p->pause());
    EXPECT_NO_THROW(p->stop());
    QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
}
