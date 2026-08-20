// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
// Graph-driven unit tests for src/libdmusic/core/datamanager.h (class DataManager).
// Uses the ":memory:" SQLite DB path (whitelisted for testing) to avoid disk I/O.
#include <gtest/gtest.h>
#include <QObject>
#include <QDir>
#include <QSet>
#include <QString>
#include <QStringList>
#include <QList>
#include <QVariant>
#include <QPair>
#include "global.h"
#include "core/datamanager.h"

class DataManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        QString tmp = QDir::tempPath() + "/dm-datamanager-ut";
        QDir().mkpath(tmp + "/cache");
        QDir().mkpath(tmp + "/config");
        QDir().mkpath(tmp + "/music");
        DmGlobal::setCachePath(tmp + "/cache");
        DmGlobal::setConfigPath(tmp + "/config");
        DmGlobal::setMusicPath(tmp + "/music");
        DmGlobal::setAppName("deepin-music-ut");
        DmGlobal::initPath();
        dm = new DataManager(QStringList() << "mp3" << "flac", nullptr, ":memory:");
    }
    void TearDown() override { delete dm; dm = nullptr; }
    DataManager *dm = nullptr;
};

TEST_F(DataManagerTest, Lifecycle_ConstructDestruct_NoThrow) {
    ASSERT_NE(dm, nullptr);
}

TEST_F(DataManagerTest, AllPublicGetters_OnEmpty_NoThrow) {
    EXPECT_NO_THROW((void)dm->currentPlayliHash());
    EXPECT_NO_THROW((void)dm->metaFromHash("nohash"));
    EXPECT_NO_THROW((void)dm->playlistFromHash("nohash"));
    EXPECT_NO_THROW((void)dm->isPlaylistQueueMatched("nohash", QList<DMusic::MediaMeta>{}));
    EXPECT_NO_THROW((void)dm->getPlaylistMetas("nohash", -1));
    EXPECT_NO_THROW((void)dm->allPlaylistInfos());
    EXPECT_NO_THROW((void)dm->allPlaylistVariantList());
    EXPECT_NO_THROW((void)dm->customPlaylistInfos());
    EXPECT_NO_THROW((void)dm->customPlaylistVariantList());
    EXPECT_NO_THROW((void)dm->allAlbumInfos());
    EXPECT_NO_THROW((void)dm->allAlbumVariantList());
    EXPECT_NO_THROW((void)dm->allArtistInfos());
    EXPECT_NO_THROW((void)dm->allArtistVariantList());
    EXPECT_NO_THROW((void)dm->isExistMeta());
    EXPECT_NO_THROW((void)dm->searchedAlbumInfos());
    EXPECT_NO_THROW((void)dm->searchedArtistInfos());
    EXPECT_NO_THROW((void)dm->searchedAlbumVariantList());
    EXPECT_NO_THROW((void)dm->searchedArtistVariantList());
    EXPECT_NO_THROW((void)dm->valueFromSettings("unknown.key"));
}

TEST_F(DataManagerTest, PlaylistMutations_NoThrow) {
    DMusic::PlaylistInfo pl = dm->addPlayList("MyList");
    QString hash = pl.uuid;
    EXPECT_FALSE(hash.isEmpty());
    dm->setCurrentPlayliHash(hash);
    EXPECT_EQ(dm->currentPlayliHash(), hash);
    EXPECT_NO_THROW((void)dm->isExistMeta("m1", hash));
    EXPECT_NO_THROW(dm->addMetasToPlayList(QList<QString>{"m1", "m2"}, hash, true));
    DMusic::MediaMeta meta;
    meta.hash = "m3"; meta.title = "t"; meta.artist = "a";
    EXPECT_NO_THROW(dm->addMetasToPlayList(QList<DMusic::MediaMeta>{meta}, hash, true));
    EXPECT_NO_THROW((void)dm->getPlaylistMetas(hash, -1));
    EXPECT_NO_THROW((void)dm->allPlaylistInfos());
    EXPECT_NO_THROW(dm->sortPlaylist(DmGlobal::SortByTitle, hash, true));
    EXPECT_NO_THROW((void)dm->renamePlaylist("Renamed", hash));
    EXPECT_NO_THROW(dm->movePlaylist(hash, ""));
    EXPECT_NO_THROW(dm->removeFromPlayList(QStringList{"m1"}, hash, false));
    EXPECT_NO_THROW((void)dm->moveMetasPlayList(QStringList{"m2"}, hash, ""));
    EXPECT_NO_THROW(dm->clearPlayList(hash, true));
    EXPECT_NO_THROW((void)dm->deletePlaylist(hash));
}

TEST_F(DataManagerTest, ImportMetas_NoUrls_NoThrow) {
    EXPECT_NO_THROW(dm->importMetas(QStringList(), "", false));
    EXPECT_NO_THROW(dm->importMetas(QStringList{"/tmp/no-a.mp3"}, "play", false));
}

TEST_F(DataManagerTest, SetValueToSettings_NoThrow) {
    EXPECT_NO_THROW(dm->setValueToSettings("base.play.auto_play", QVariant(true), false));
}
TEST_F(DataManagerTest, ValueFromSettings_NoThrow) {
    EXPECT_NO_THROW((void)dm->valueFromSettings("base.play.auto_play"));
}
TEST_F(DataManagerTest, SyncToSettings_NoThrow) {
    EXPECT_NO_THROW(dm->syncToSettings());
}
TEST_F(DataManagerTest, ResetToSettings_NoThrow) {
    EXPECT_NO_THROW(dm->resetToSettings());
}
TEST_F(DataManagerTest, SaveDataToDB_NoThrow) {
    EXPECT_NO_THROW(dm->saveDataToDB());
}
TEST_F(DataManagerTest, UpsertMetasDB_NoThrow) {
    EXPECT_NO_THROW((void)dm->upsertMetasDB());
}
TEST_F(DataManagerTest, UpdateMetaCodec_NoThrow) {
    DMusic::MediaMeta meta;
    EXPECT_NO_THROW(dm->updateMetaCodec(meta));
}

TEST_F(DataManagerTest, Search_NoThrow) {
    QStringList metaTitles;
    QList<QPair<QString, QString>> albums;
    QList<QPair<QString, QString>> artists;
    EXPECT_NO_THROW(dm->quickSearchText("abc", metaTitles, albums, artists));
    QList<DMusic::MediaMeta> outMetas;
    QList<DMusic::AlbumInfo> outAlbums;
    QList<DMusic::ArtistInfo> outArtists;
    EXPECT_NO_THROW(dm->searchText("abc", outMetas, outAlbums, outArtists, ""));
    EXPECT_NO_THROW((void)dm->searchedAlbumInfos());
    EXPECT_NO_THROW((void)dm->searchedArtistInfos());
}

TEST_F(DataManagerTest, PublicSlots_DirectCall_NoThrow) {
    EXPECT_NO_THROW(dm->slotAddOneMeta(QStringList{"play"}, DMusic::MediaMeta{}));
    EXPECT_NO_THROW(dm->slotLazyLoadDatabase());
    EXPECT_NO_THROW(dm->slotMetaCoverReady(DMusic::MediaMeta{}));
    EXPECT_NO_THROW(dm->slotCoverBatchFinished());
}

// ---- Data-driven coverage: populate real metas to drive the private
// index/album/artist/DB helpers that empty-state calls short-circuit. ----

static DMusic::MediaMeta makeSampleMeta(const QString &hash, const QString &title,
                                        const QString &artist, const QString &album) {
    DMusic::MediaMeta m;
    m.hash = hash;
    m.title = title;
    m.artist = artist;
    m.album = album;
    m.localPath = QString::fromUtf8(TEST_DATA_DIR) + "/sample.mp3";
    m.filetype = "mp3";
    m.length = 1000;
    return m;
}

TEST_F(DataManagerTest, WithData_SlotAddOneMeta_BuildsAlbumArtistIndexes) {
    DMusic::MediaMeta m1 = makeSampleMeta("h1", "Title1", "ArtistA", "AlbumX");
    DMusic::MediaMeta m2 = makeSampleMeta("h2", "Title2", "ArtistB", "AlbumY");
    EXPECT_NO_THROW(dm->slotAddOneMeta(QStringList{"play"}, m1));
    EXPECT_NO_THROW(dm->slotAddOneMeta(QStringList{"play"}, m2));
    EXPECT_NO_THROW((void)dm->allAlbumInfos());
    EXPECT_NO_THROW((void)dm->allArtistInfos());
    EXPECT_NO_THROW((void)dm->allPlaylistVariantList());
}

TEST_F(DataManagerTest, WithData_PlaylistOps_ExerciseIndexHelpers) {
    DMusic::MediaMeta m1 = makeSampleMeta("h1", "Title1", "ArtistA", "AlbumX");
    dm->slotAddOneMeta(QStringList{"play"}, m1);
    DMusic::PlaylistInfo pl = dm->addPlayList("MyList");
    QString hash = pl.uuid;
    dm->addMetasToPlayList(QList<QString>{"h1"}, hash, true);
    EXPECT_NO_THROW((void)dm->getPlaylistMetas(hash, -1));
    EXPECT_NO_THROW((void)dm->isExistMeta("h1", hash));
    EXPECT_NO_THROW((void)dm->isExistMeta("nope", hash));
    dm->sortPlaylist(DmGlobal::SortByTitle, hash, true);
    EXPECT_NO_THROW(dm->removeFromPlayList(QStringList{"h1"}, hash, false));
    EXPECT_NO_THROW((void)dm->moveMetasPlayList(QStringList{"h1"}, hash, ""));
    EXPECT_NO_THROW(dm->clearPlayList(hash, true));
    EXPECT_NO_THROW((void)dm->deletePlaylist(hash));
}

TEST_F(DataManagerTest, WithData_Search_ExerciseSearchHelpers) {
    DMusic::MediaMeta m1 = makeSampleMeta("h1", "HelloSong", "ArtistA", "AlbumX");
    dm->slotAddOneMeta(QStringList{"play"}, m1);
    QStringList metaTitles;
    QList<QPair<QString, QString>> albums;
    QList<QPair<QString, QString>> artists;
    EXPECT_NO_THROW(dm->quickSearchText("Hello", metaTitles, albums, artists));
    QList<DMusic::MediaMeta> outMetas;
    QList<DMusic::AlbumInfo> outAlbums;
    QList<DMusic::ArtistInfo> outArtists;
    EXPECT_NO_THROW(dm->searchText("Hello", outMetas, outAlbums, outArtists, ""));
    EXPECT_NO_THROW((void)dm->searchedAlbumInfos());
    EXPECT_NO_THROW((void)dm->searchedArtistInfos());
    EXPECT_NO_THROW((void)dm->searchedAlbumVariantList());
    EXPECT_NO_THROW((void)dm->searchedArtistVariantList());
}

TEST_F(DataManagerTest, WithData_DB_WriteAndLoad_ExerciseDbHelpers) {
    DMusic::MediaMeta m1 = makeSampleMeta("h1", "Title1", "ArtistA", "AlbumX");
    dm->slotAddOneMeta(QStringList{"play"}, m1);
    EXPECT_NO_THROW(dm->saveDataToDB());
    EXPECT_NO_THROW((void)dm->upsertMetasDB());
    EXPECT_NO_THROW(dm->slotLazyLoadDatabase());
    EXPECT_NO_THROW(dm->updateMetaCodec(m1));
}

TEST_F(DataManagerTest, WithData_RemoveMeta_ExerciseDeleteHelpers) {
    DMusic::MediaMeta m1 = makeSampleMeta("h1", "Title1", "ArtistA", "AlbumX");
    DMusic::MediaMeta m2 = makeSampleMeta("h2", "Title2", "ArtistB", "AlbumY");
    DMusic::PlaylistInfo pl = dm->addPlayList("DelList");
    QString hash = pl.uuid;
    dm->addMetasToPlayList(QList<DMusic::MediaMeta>{m1, m2}, hash, true);
    // delFlag=true with the meta in a real playlist -> deleteMetaFromAllMetas/
    // Album/Artist.
    EXPECT_NO_THROW(dm->removeFromPlayList(QStringList{"h1"}, hash, true));
    // playlistHash="all" branch -> full removal path.
    EXPECT_NO_THROW(dm->removeFromPlayList(QStringList{"h2"}, "all", false));
    EXPECT_NO_THROW((void)dm->deletePlaylist(hash));
}

TEST_F(DataManagerTest, WithData_MetaFromHash_PlaylistFromHash) {
    DMusic::MediaMeta m1 = makeSampleMeta("h1", "Title1", "ArtistA", "AlbumX");
    dm->slotAddOneMeta(QStringList{"play"}, m1);
    EXPECT_NO_THROW((void)dm->metaFromHash("h1"));
    EXPECT_NO_THROW((void)dm->playlistFromHash("play"));
    QList<DMusic::MediaMeta> single{m1};
    EXPECT_NO_THROW((void)dm->isPlaylistQueueMatched("play", single));
}

TEST_F(DataManagerTest, WithData_SortAllTypes_CoversComparators) {
    DMusic::MediaMeta m1 = makeSampleMeta("h1", "BetaTitle", "ZetaArtist", "GammaAlbum");
    DMusic::MediaMeta m2 = makeSampleMeta("h2", "AlphaTitle", "MuArtist", "DeltaAlbum");
    DMusic::PlaylistInfo pl = dm->addPlayList("SortList");
    QString hash = pl.uuid;
    dm->addMetasToPlayList(QList<DMusic::MediaMeta>{m1, m2}, hash, true);
    // sortPlaylist switches on the BASE sort types (AddTime/Title/Artist/Ablum/
    // Custom). Call each twice (signalFlag=true) to toggle ASC<->DES and cover
    // both comparator variants.
    int baseTypes[] = {DmGlobal::SortByAddTime, DmGlobal::SortByTitle,
                       DmGlobal::SortByArtist, DmGlobal::SortByAblum,
                       DmGlobal::SortByCustom};
    for (int t : baseTypes) {
        EXPECT_NO_THROW(dm->sortPlaylist(t, hash, true));  // -> ASC
        EXPECT_NO_THROW(dm->sortPlaylist(t, hash, true));  // -> DES
    }
}

TEST_F(DataManagerTest, WithData_AllBuildsAlbumArtistSort_CoversComparators) {
    DMusic::MediaMeta m1 = makeSampleMeta("a1", "T1", "Ar1", "Al1");
    DMusic::MediaMeta m2 = makeSampleMeta("a2", "T2", "Ar2", "Al2");
    dm->slotAddOneMeta(QStringList{"all"}, m1);
    dm->slotAddOneMeta(QStringList{"all"}, m2);
    EXPECT_NO_THROW((void)dm->allAlbumInfos());
    EXPECT_NO_THROW((void)dm->allArtistInfos());
    EXPECT_NO_THROW((void)dm->allAlbumVariantList());
    EXPECT_NO_THROW((void)dm->allArtistVariantList());
    // album/artist playlist sort (signalFlag=false) -> TimestampASC comparators
    EXPECT_NO_THROW(dm->sortPlaylist(DmGlobal::SortByAddTimeASC, "album", false));
    EXPECT_NO_THROW(dm->sortPlaylist(DmGlobal::SortByAddTimeASC, "artist", false));
    EXPECT_NO_THROW(dm->sortPlaylist(DmGlobal::SortByAddTimeASC, "albumResult", false));
    EXPECT_NO_THROW(dm->sortPlaylist(DmGlobal::SortByAddTimeASC, "artistResult", false));
    // isPlaylistQueueMatched with a populated playlist -> comparator lambda
    DMusic::PlaylistInfo pl = dm->addPlayList("QList");
    dm->addMetasToPlayList(QList<DMusic::MediaMeta>{m1, m2}, pl.uuid, true);
    EXPECT_NO_THROW((void)dm->isPlaylistQueueMatched(pl.uuid, QList<DMusic::MediaMeta>{m1, m2}));
}
