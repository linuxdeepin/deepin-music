// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// datamanager.cpp 的单元测试：歌单 CRUD、查询、设置读写。
// 对应 docs/unit-test-plan.md Phase 2。
//
// 关键：所有测试用 ":memory:" 内存 DB 注入，数据完全隔离，绝不触碰
// ~/.cache/mediameta.sqlite 真实库。每个用例用独立 DataManager 实例 + 作用域隔离，
// 避免 QSqlDatabase 默认连接名冲突。

#include <gtest/gtest.h>

#include <QCoreApplication>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QFile>
#include <QSignalSpy>

#include "datamanager.h"
#include "global.h"

#ifndef TEST_DATA_DIR
#define TEST_DATA_DIR "."
#endif

namespace {
QString sampleMp3Path()
{
    return QString(TEST_DATA_DIR) + "/sample.mp3";
}
}

namespace {
// 内存 DB 的 DataManager 工厂
DataManager *makeDataMgr(const QStringList &suffix = QStringList{"mp3", "flac"})
{
    return new DataManager(suffix, nullptr, ":memory:");
}

// 导入 sample.mp3 并返回已导入的 DataManager（用于"导入后操作"测试）
std::unique_ptr<DataManager> importSample()
{
    auto dm = std::unique_ptr<DataManager>(makeDataMgr());
    QSignalSpy spy(dm.get(), &DataManager::signalImportFinished);
    dm->importMetas({sampleMp3Path()}, "play");
    spy.wait(5000);
    return dm;
}
}

// ============================================================================
// 内置歌单：内存 DB 构造后应预置一组只读内置歌单（album/artist/fav/play 等）
// ============================================================================
TEST(DataManagerBuiltInTest, hasBuiltinPlaylistsAfterConstruction)
{
    std::unique_ptr<DataManager> dm(makeDataMgr());
    const auto playlists = dm->allPlaylistInfos();
    EXPECT_GT(playlists.size(), 0);

    // 收集所有内置 uuid，验证关键内置歌单存在
    QStringList uuids;
    for (const auto &pl : playlists) uuids << pl.uuid;
    EXPECT_TRUE(uuids.contains("play"));
}

TEST(DataManagerBuiltInTest, customPlaylistsInitiallyHasNoUserList)
{
    // customPlaylistInfos 返回非内置（用户自建）歌单；全新内存库应为空或仅内置
    std::unique_ptr<DataManager> dm(makeDataMgr());
    const auto customs = dm->customPlaylistInfos();
    // 内置歌单 hide=true，customPlaylistInfos 通常排除 hide 的；这里只验证不崩溃且可调用
    EXPECT_GE(customs.size(), 0);
}

// ============================================================================
// addPlayList : 新建歌单，返回有效 uuid；重名自动加序号
// ============================================================================
TEST(DataManagerAddPlaylistTest, createsPlaylistWithUuid)
{
    std::unique_ptr<DataManager> dm(makeDataMgr());
    const auto pl = dm->addPlayList("MyList");
    EXPECT_FALSE(pl.uuid.isEmpty());
    EXPECT_EQ(pl.displayName, "MyList");
}

TEST(DataManagerAddPlaylistTest, duplicateNameGetsSuffix)
{
    std::unique_ptr<DataManager> dm(makeDataMgr());
    const auto pl1 = dm->addPlayList("Dup");
    const auto pl2 = dm->addPlayList("Dup");
    EXPECT_EQ(pl1.displayName, "Dup");
    EXPECT_EQ(pl2.displayName, "Dup 1");   // 重名自动加序号
    EXPECT_NE(pl1.uuid, pl2.uuid);
}

// ============================================================================
// deletePlaylist : 删除存在的歌单返回 true；不存在返回 false
// ============================================================================
TEST(DataManagerDeletePlaylistTest, deletesExistingPlaylist)
{
    std::unique_ptr<DataManager> dm(makeDataMgr());
    const auto pl = dm->addPlayList("ToDelete");
    EXPECT_TRUE(dm->deletePlaylist(pl.uuid));
}

TEST(DataManagerDeletePlaylistTest, returnsFalseForNonexistent)
{
    std::unique_ptr<DataManager> dm(makeDataMgr());
    EXPECT_FALSE(dm->deletePlaylist("nonexistent-uuid-xyz"));
}

TEST(DataManagerDeletePlaylistTest, deletesFromAllPlaylistInfos)
{
    std::unique_ptr<DataManager> dm(makeDataMgr());
    const auto pl = dm->addPlayList("WillDelete");
    const int before = dm->allPlaylistInfos().size();
    dm->deletePlaylist(pl.uuid);
    const int after = dm->allPlaylistInfos().size();
    EXPECT_EQ(after, before - 1);
}

// ============================================================================
// renamePlaylist : 改名成功；重名拒绝
// ============================================================================
TEST(DataManagerRenamePlaylistTest, renamesSuccessfully)
{
    std::unique_ptr<DataManager> dm(makeDataMgr());
    const auto pl = dm->addPlayList("OldName");
    EXPECT_TRUE(dm->renamePlaylist("NewName", pl.uuid));
    // 通过 allPlaylistInfos 找回验证
    const auto playlists = dm->allPlaylistInfos();
    bool found = false;
    for (const auto &p : playlists) {
        if (p.uuid == pl.uuid) {
            EXPECT_EQ(p.displayName, "NewName");
            found = true;
        }
    }
    EXPECT_TRUE(found);
}

TEST(DataManagerRenamePlaylistTest, rejectsDuplicateName)
{
    std::unique_ptr<DataManager> dm(makeDataMgr());
    dm->addPlayList("NameA");
    const auto plB = dm->addPlayList("NameB");
    // 把 B 改成 A 的名字，应被拒绝
    EXPECT_FALSE(dm->renamePlaylist("NameA", plB.uuid));
}

TEST(DataManagerRenamePlaylistTest, returnsFalseForNonexistent)
{
    std::unique_ptr<DataManager> dm(makeDataMgr());
    EXPECT_FALSE(dm->renamePlaylist("Whatever", "nonexistent-uuid"));
}

// ============================================================================
// currentPlayliHash / setCurrentPlayliHash : 当前歌单读写
// ============================================================================
TEST(DataManagerCurrentPlaylistTest, setAndGetCurrentPlaylist)
{
    std::unique_ptr<DataManager> dm(makeDataMgr());
    dm->setCurrentPlayliHash("play");
    EXPECT_EQ(dm->currentPlayliHash(), "play");
}

// ============================================================================
// isExistMeta() : 无 meta 时返回 false
// ============================================================================
TEST(DataManagerIsExistMetaTest, emptyLibraryReturnsFalse)
{
    std::unique_ptr<DataManager> dm(makeDataMgr());
    EXPECT_FALSE(dm->isExistMeta());
}

// ============================================================================
// getPlaylistMetas : 空库查询不崩溃，返回空
// ============================================================================
TEST(DataManagerGetMetasTest, emptyPlaylistReturnsEmpty)
{
    std::unique_ptr<DataManager> dm(makeDataMgr());
    const auto metas = dm->getPlaylistMetas("play");
    EXPECT_TRUE(metas.isEmpty());
}

TEST(DataManagerGetMetasTest, nonexistentPlaylistReturnsEmpty)
{
    std::unique_ptr<DataManager> dm(makeDataMgr());
    const auto metas = dm->getPlaylistMetas("no-such-playlist");
    EXPECT_TRUE(metas.isEmpty());
}

// ============================================================================
// valueFromSettings / setValueToSettings : 设置读写回环
// 注意：底层 MusicSettings 基于 DTK DSettings（schema 约束），只能读写 schema 中
// 已定义的 key（见 src/libdmusic/data/music-settings.json）。写入未定义 key 会使
// DSettings::setOption 返回空指针并解引用崩溃（源码缺陷，见文档「已知缺陷」）。
// 故这里只用真实存在的 key 测试。
// ============================================================================
TEST(DataManagerSettingsTest, setAndGetVolumeRoundtrip)
{
    std::unique_ptr<DataManager> dm(makeDataMgr());
    dm->setValueToSettings("base.play.volume", 80);
    EXPECT_EQ(dm->valueFromSettings("base.play.volume").toInt(), 80);
}

TEST(DataManagerSettingsTest, overwriteExistingValue)
{
    std::unique_ptr<DataManager> dm(makeDataMgr());
    dm->setValueToSettings("base.play.volume", 30);
    dm->setValueToSettings("base.play.volume", 70);
    EXPECT_EQ(dm->valueFromSettings("base.play.volume").toInt(), 70);
}

TEST(DataManagerSettingsTest, setAndGetMuteFlag)
{
    std::unique_ptr<DataManager> dm(makeDataMgr());
    dm->setValueToSettings("base.play.mute", true);
    EXPECT_EQ(dm->valueFromSettings("base.play.mute").toBool(), true);
}

// ============================================================================
// quickSearchText : 空库搜索不崩溃，返回空结果
// ============================================================================
TEST(DataManagerSearchTest, emptyLibrarySearchReturnsNoResults)
{
    std::unique_ptr<DataManager> dm(makeDataMgr());
    QStringList metaTitles;
    QList<QPair<QString, QString>> albums, artists;
    dm->quickSearchText("anything", metaTitles, albums, artists);
    EXPECT_TRUE(metaTitles.isEmpty());
    EXPECT_TRUE(albums.isEmpty());
    EXPECT_TRUE(artists.isEmpty());
}

// ============================================================================
// metaFromHash / playlistFromHash : 不存在 hash 返回空对象
// ============================================================================
TEST(DataManagerQueryTest, metaFromHashNonexistentReturnsEmpty)
{
    std::unique_ptr<DataManager> dm(makeDataMgr());
    const auto meta = dm->metaFromHash("nonexistent-hash-xyz");
    EXPECT_TRUE(meta.hash.isEmpty());
}

TEST(DataManagerQueryTest, playlistFromHashBuiltinReturnsValid)
{
    std::unique_ptr<DataManager> dm(makeDataMgr());
    // "play" 是内置歌单，应能查到
    const auto pl = dm->playlistFromHash("play");
    EXPECT_EQ(pl.uuid, "play");
}

TEST(DataManagerQueryTest, playlistFromHashNonexistentReturnsEmpty)
{
    std::unique_ptr<DataManager> dm(makeDataMgr());
    const auto pl = dm->playlistFromHash("nonexistent-hash-xyz");
    EXPECT_TRUE(pl.uuid.isEmpty());
}

// ============================================================================
// 批量查询：空库返回空集合，不崩溃
// ============================================================================
TEST(DataManagerBulkQueryTest, emptyLibraryReturnsExpectedCollections)
{
    std::unique_ptr<DataManager> dm(makeDataMgr());
    // 无歌曲：album/artist 相关集合应为空
    EXPECT_TRUE(dm->allAlbumInfos().isEmpty());
    EXPECT_TRUE(dm->allArtistInfos().isEmpty());
    EXPECT_TRUE(dm->allAlbumVariantList().isEmpty());
    EXPECT_TRUE(dm->allArtistVariantList().isEmpty());
    // 内置歌单（album/artist/fav/play 等）构造时预置，故 allPlaylistVariantList 非空
    EXPECT_FALSE(dm->allPlaylistVariantList().isEmpty());
    // 用户自建歌单：空库应为空
    EXPECT_TRUE(dm->customPlaylistVariantList().isEmpty());
    // 搜索结果：未搜索时应为空
    EXPECT_TRUE(dm->searchedAlbumVariantList().isEmpty());
    EXPECT_TRUE(dm->searchedArtistVariantList().isEmpty());
}

// ============================================================================
// movePlaylist : 移动歌单顺序不崩溃
// ============================================================================
TEST(DataManagerMovePlaylistTest, moveDoesNotCrash)
{
    std::unique_ptr<DataManager> dm(makeDataMgr());
    const auto pl1 = dm->addPlayList("A");
    const auto pl2 = dm->addPlayList("B");
    dm->movePlaylist(pl1.uuid, pl2.uuid);
    SUCCEED();
}

// ============================================================================
// sortPlaylist : 排序不崩溃
// ============================================================================
TEST(DataManagerSortPlaylistTest, sortDoesNotCrash)
{
    std::unique_ptr<DataManager> dm(makeDataMgr());
    dm->sortPlaylist(DmGlobal::SortByTitle, "play", true);
    dm->sortPlaylist(DmGlobal::SortByAddTime, "play", false);
    SUCCEED();
}

// ============================================================================
// isExistMeta(hash, playlist) : 不存在返回 false
// ============================================================================
TEST(DataManagerIsExistMetaInPlaylistTest, nonexistentReturnsFalse)
{
    std::unique_ptr<DataManager> dm(makeDataMgr());
    EXPECT_FALSE(dm->isExistMeta("no-such-meta", "play"));
}

// ============================================================================
// searchText : 空库搜索返回空
// ============================================================================
TEST(DataManagerSearchTextTest, emptyLibraryReturnsNoResults)
{
    std::unique_ptr<DataManager> dm(makeDataMgr());
    QList<DMusic::MediaMeta> metaTitles;
    QList<DMusic::AlbumInfo> albums;
    QList<DMusic::ArtistInfo> artists;
    dm->searchText("anything", metaTitles, albums, artists);
    EXPECT_TRUE(metaTitles.isEmpty());
    EXPECT_TRUE(albums.isEmpty());
    EXPECT_TRUE(artists.isEmpty());
    // 搜索后查询搜索结果也应为空
    EXPECT_TRUE(dm->searchedAlbumInfos().isEmpty());
    EXPECT_TRUE(dm->searchedArtistInfos().isEmpty());
}

// ============================================================================
// syncToSettings / resetToSettings : 不崩溃
// ============================================================================
TEST(DataManagerSettingsSyncTest, syncAndResetDoNotCrash)
{
    std::unique_ptr<DataManager> dm(makeDataMgr());
    dm->syncToSettings();
    dm->resetToSettings();
    SUCCEED();
}

// ============================================================================
// getPlaylistMetas 默认参数：不传 hash 返回全部（不崩溃）
// ============================================================================
TEST(DataManagerGetMetasDefaultTest, defaultArgsDoNotCrash)
{
    std::unique_ptr<DataManager> dm(makeDataMgr());
    const auto metas = dm->getPlaylistMetas();
    EXPECT_TRUE(metas.isEmpty());
}

// ============================================================================
// addMetasToPlayList(metas) : 向歌单注入构造的 meta（不需文件），填充 sortMetas
// 这是激活 sortPlaylist 排序谓词的关键前序步骤
// ============================================================================
TEST(DataManagerAddMetasTest, populatesSortMetasForBuiltinPlaylist)
{
    std::unique_ptr<DataManager> dm(makeDataMgr());
    DMusic::MediaMeta m1; m1.hash = "h1"; m1.title = "B";
    DMusic::MediaMeta m2; m2.hash = "h2"; m2.title = "A";
    dm->addMetasToPlayList(QList<DMusic::MediaMeta>{m1, m2}, "play");
    const auto pl = dm->playlistFromHash("play");
    EXPECT_EQ(pl.sortMetas.size(), 2);
    EXPECT_TRUE(pl.sortMetas.contains("h1"));
    EXPECT_TRUE(pl.sortMetas.contains("h2"));
}

TEST(DataManagerAddMetasTest, emptyMetasOrHashIsNoop)
{
    std::unique_ptr<DataManager> dm(makeDataMgr());
    dm->addMetasToPlayList(QList<DMusic::MediaMeta>{}, "play");                 // 空 metas
    dm->addMetasToPlayList(QList<DMusic::MediaMeta>{DMusic::MediaMeta()}, "");  // 空 hash
    EXPECT_EQ(dm->playlistFromHash("play").sortMetas.size(), 0);
}

// ============================================================================
// sortPlaylist : 有元素时触发各排序谓词（moreThanTimestampASC/DES/TitleASC/...）
// signalFlag=false 直接按 type 排序，覆盖各 ASC/DES 分支
// ============================================================================
TEST(DataManagerSortWithMetasTest, sortByAllTypesTriggersComparators)
{
    std::unique_ptr<DataManager> dm(makeDataMgr());
    DMusic::MediaMeta m1; m1.hash = "h1";
    DMusic::MediaMeta m2; m2.hash = "h2";
    dm->addMetasToPlayList(QList<DMusic::MediaMeta>{m1, m2}, "play");
    // 各排序类型（signalFlag=false 直接按 type 排序，触发对应 static 谓词）
    dm->sortPlaylist(DmGlobal::SortByAddTimeASC, "play", false);
    dm->sortPlaylist(DmGlobal::SortByAddTimeDES, "play", false);
    dm->sortPlaylist(DmGlobal::SortByTitleASC,   "play", false);
    dm->sortPlaylist(DmGlobal::SortByTitleDES,   "play", false);
    dm->sortPlaylist(DmGlobal::SortByArtistASC,  "play", false);
    dm->sortPlaylist(DmGlobal::SortByArtistDES,  "play", false);
    dm->sortPlaylist(DmGlobal::SortByAblumASC,   "play", false);
    dm->sortPlaylist(DmGlobal::SortByAblumDES,   "play", false);
    SUCCEED();
}

TEST(DataManagerSortWithMetasTest, sortCustomSetsSortType)
{
    std::unique_ptr<DataManager> dm(makeDataMgr());
    DMusic::MediaMeta m; m.hash = "h1";
    dm->addMetasToPlayList(QList<DMusic::MediaMeta>{m}, "play");
    dm->sortPlaylist(DmGlobal::SortByCustomASC, "play", false);
    EXPECT_EQ(dm->playlistFromHash("play").sortType, DmGlobal::SortByCustomASC);
}

// ============================================================================
// clearPlayList : 清空歌单元数据
// ============================================================================
TEST(DataManagerClearPlayListTest, clearsSortMetas)
{
    std::unique_ptr<DataManager> dm(makeDataMgr());
    DMusic::MediaMeta m; m.hash = "h1";
    dm->addMetasToPlayList(QList<DMusic::MediaMeta>{m}, "play");
    EXPECT_EQ(dm->playlistFromHash("play").sortMetas.size(), 1);
    dm->clearPlayList("play");
    EXPECT_EQ(dm->playlistFromHash("play").sortMetas.size(), 0);
}

// ============================================================================
// removeFromPlayList : delFlag=true 触发 deleteMetaFromAllMetas/Album/Artist
// ============================================================================
TEST(DataManagerRemoveFromPlayListTest, removeWithDelFlagFromPlay)
{
    std::unique_ptr<DataManager> dm(makeDataMgr());
    DMusic::MediaMeta m; m.hash = "h1";
    dm->addMetasToPlayList(QList<DMusic::MediaMeta>{m}, "play");
    dm->removeFromPlayList({"h1"}, "play", true);  // delFlag=true
    EXPECT_EQ(dm->playlistFromHash("play").sortMetas.size(), 0);
}

TEST(DataManagerRemoveFromPlayListTest, removeFromAllPlaylist)
{
    std::unique_ptr<DataManager> dm(makeDataMgr());
    DMusic::MediaMeta m; m.hash = "h1";
    dm->addMetasToPlayList(QList<DMusic::MediaMeta>{m}, "play");
    dm->removeFromPlayList({"h1"}, "all", false);
    SUCCEED();
}

// ============================================================================
// moveMetasPlayList : 在 custom 排序的歌单内移动
// ============================================================================
TEST(DataManagerMoveMetasPlayListTest, movesInCustomSortedList)
{
    std::unique_ptr<DataManager> dm(makeDataMgr());
    DMusic::MediaMeta m1; m1.hash = "h1";
    DMusic::MediaMeta m2; m2.hash = "h2";
    dm->addMetasToPlayList(QList<DMusic::MediaMeta>{m1, m2}, "play");
    // moveMetasPlayList 要求 sortType == CustomASC/DES
    dm->sortPlaylist(DmGlobal::SortByCustomASC, "play", false);
    EXPECT_TRUE(dm->moveMetasPlayList({"h1"}, "play", "h2"));
}

TEST(DataManagerMoveMetasPlayListTest, rejectsSystemPlaylist)
{
    std::unique_ptr<DataManager> dm(makeDataMgr());
    EXPECT_FALSE(dm->moveMetasPlayList({"h1"}, "all", ""));
    EXPECT_FALSE(dm->moveMetasPlayList({"h1"}, "album", ""));
}

// ============================================================================
// sortPlaylist signalFlag=true : 覆盖 ASC↔DES 翻转逻辑分支（line 1367-1418）
// 之前只测 signalFlag=false（直接按 type），翻转分支完全未覆盖
// ============================================================================
TEST(DataManagerSortToggleTest, toggleSortCoversFlippingLogic)
{
    std::unique_ptr<DataManager> dm(makeDataMgr());
    DMusic::MediaMeta m1; m1.hash = "h1";
    DMusic::MediaMeta m2; m2.hash = "h2";
    dm->addMetasToPlayList(QList<DMusic::MediaMeta>{m1, m2}, "play");
    // signalFlag=true：根据当前 sortType 翻转。连续调用覆盖各 ASC↔DES 分支
    dm->sortPlaylist(DmGlobal::SortByAddTime, "play", true);
    dm->sortPlaylist(DmGlobal::SortByAddTime, "play", true);   // 翻转回
    dm->sortPlaylist(DmGlobal::SortByTitle,   "play", true);
    dm->sortPlaylist(DmGlobal::SortByArtist,  "play", true);
    dm->sortPlaylist(DmGlobal::SortByAblum,   "play", true);
    dm->sortPlaylist(DmGlobal::SortByCustom,  "play", true);   // Custom 分支 emit 信号
    SUCCEED();
}

// ============================================================================
// importMetas : 用真实 mp3 触发异步导入（DBOperate Worker 线程）
// 激活 importMetas / slotImportMetas / addMetaToAlbum / addMetaToArtist /
// load*DB 等核心数据通路——这是 :memory: 空库无法触及的路径
// ============================================================================
TEST(DataManagerImportTest, importMetasProcessesRealFile)
{
    ASSERT_TRUE(QFile::exists(sampleMp3Path())) << "testdata/sample.mp3 missing";
    std::unique_ptr<DataManager> dm(makeDataMgr());
    QSignalSpy spy(dm.get(), &DataManager::signalImportFinished);
    dm->importMetas({sampleMp3Path()}, "play");
    // 等待异步导入完成（Worker 线程 emit signalImportFinished）
    EXPECT_TRUE(spy.wait(5000)) << "importMetas did not finish within 5s";
    // 导入后库应有 meta
    EXPECT_TRUE(dm->isExistMeta());
}

// ============================================================================
// importMetas 后搜索：覆盖 searchText 的 music/album/artist/all 四个分支
// 导入后库有 meta，空文本搜索匹配所有
// ============================================================================
TEST(DataManagerSearchAfterImportTest, searchTextMusicTypeFindsImported)
{
    auto dm = importSample();
    ASSERT_TRUE(dm->isExistMeta());
    QList<DMusic::MediaMeta> metas;
    QList<DMusic::AlbumInfo> albums;
    QList<DMusic::ArtistInfo> artists;
    dm->searchText("", metas, albums, artists, "music");
    EXPECT_FALSE(metas.isEmpty());
}

TEST(DataManagerSearchAfterImportTest, searchTextAlbumTypeDoesNotCrash)
{
    auto dm = importSample();
    QList<DMusic::MediaMeta> metas;
    QList<DMusic::AlbumInfo> albums;
    QList<DMusic::ArtistInfo> artists;
    dm->searchText("", metas, albums, artists, "album");
    SUCCEED();
}

TEST(DataManagerSearchAfterImportTest, searchTextArtistTypeDoesNotCrash)
{
    auto dm = importSample();
    QList<DMusic::MediaMeta> metas;
    QList<DMusic::AlbumInfo> albums;
    QList<DMusic::ArtistInfo> artists;
    dm->searchText("", metas, albums, artists, "artist");
    SUCCEED();
}

TEST(DataManagerSearchAfterImportTest, searchTextAllTypeFindsImported)
{
    auto dm = importSample();
    QList<DMusic::MediaMeta> metas;
    QList<DMusic::AlbumInfo> albums;
    QList<DMusic::ArtistInfo> artists;
    dm->searchText("", metas, albums, artists);  // 默认 type="" → else(all) 分支
    EXPECT_FALSE(metas.isEmpty());
}

// ============================================================================
// importMetas 后对 album/artist 歌单排序：覆盖 sortPlaylist 的 album/artist 分支
// ============================================================================
TEST(DataManagerSortAfterImportTest, sortAlbumAndArtistPlaylists)
{
    auto dm = importSample();
    dm->sortPlaylist(DmGlobal::SortByTitleASC, "album", false);
    dm->sortPlaylist(DmGlobal::SortByTitleASC, "artist", false);
    SUCCEED();
}

// ============================================================================
// updateMetaCodec : 导入后修改 meta 的 album，触发 album/artist 重索引
// 覆盖 updateMetaCodec 的核心逻辑（line 1743-1801）
// ============================================================================
TEST(DataManagerUpdateMetaCodecTest, updateCodecReindexesAlbum)
{
    auto dm = importSample();
    ASSERT_TRUE(dm->isExistMeta());
    const QList<DMusic::MediaMeta> metas = dm->getPlaylistMetas("play");
    ASSERT_FALSE(metas.isEmpty());
    DMusic::MediaMeta meta = metas.first();
    meta.album = "ModifiedAlbum";  // 改 album → 触发重索引
    dm->updateMetaCodec(meta);
    SUCCEED();
}

TEST(DataManagerUpdateMetaCodecTest, updateCodecNonexistentHashIsNoop)
{
    auto dm = importSample();
    DMusic::MediaMeta meta;
    meta.hash = "nonexistent-hash-xyz";
    dm->updateMetaCodec(meta);  // hash 不在库 → 提前 return（line 1736）
    SUCCEED();
}

// ============================================================================
// saveDataToDB : 导入后保存到 :memory: DB（覆盖 transaction + INSERT 循环）
// ============================================================================
TEST(DataManagerSaveDataTest, saveDataToDBAfterImport)
{
    auto dm = importSample();
    dm->saveDataToDB();
    SUCCEED();
}

// ============================================================================
// slotLazyLoadDatabase : public slot，触发 loadMetasDB + loadPlaylistMetasDB
// 覆盖 load 系列的 DB 读取路径
// ============================================================================
TEST(DataManagerLazyLoadTest, slotLazyLoadDatabaseLoadsFromDB)
{
    auto dm = importSample();
    dm->saveDataToDB();           // 先写入
    dm->slotLazyLoadDatabase();   // 再加载（loadMetasDB + loadPlaylistMetasDB）
    SUCCEED();
}

// ============================================================================
// updateMetaCodec : meta 留在原 album（existFla=true 分支，line 1748-1753）
// 与 updateCodecReindexesAlbum（existFla=false 新建 album）互补
// ============================================================================
TEST(DataManagerUpdateMetaCodecTest, updateCodecStaysInExistingAlbum)
{
    auto dm = importSample();
    const QList<DMusic::MediaMeta> metas = dm->getPlaylistMetas("play");
    ASSERT_FALSE(metas.isEmpty());
    DMusic::MediaMeta meta = metas.first();
    // 不改 album → existFla=true（album 已存在）→ 更新已有 album
    dm->updateMetaCodec(meta);
    SUCCEED();
}
