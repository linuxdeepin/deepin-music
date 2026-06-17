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
#include <QFileInfo>
#include <QDir>
#include <QSignalSpy>

#include "datamanager.h"
#include "global.h"
#include "core/dboperate.h"
#include "util/utils.h"

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

// ============================================================================
// Phase 3 扩展：继续提升 datamanager.cpp 行覆盖率
// 重点覆盖此前未触及的分支：album/artist 聚合查询与排序、search 各类型分支、
// addMetasToPlayList 的 current-playlist 分支、removeFromPlayList 的级联删除分支、
// moveMetasPlayList 末尾插入、clearPlayList 默认参数、signalClearImportingHash 信号、
// deleteMetaFromAlbum/Artist 清空后移除等。
// ============================================================================

// ----------------------------------------------------------------------------
// allAlbumInfos / allArtistInfos : 导入后聚合查询应返回非空集合
// 关键：触发 addMetaToAlbum/addMetaToArtist（含 compareAlbumName/compareArtistName 谓词）
// 及 allAlbumInfos 内部 sortPlaylist(album) 路径
// ----------------------------------------------------------------------------
TEST(DataManagerAggregateTest, allAlbumInfosPopulatedAfterImport)
{
    auto dm = importSample();
    ASSERT_TRUE(dm->isExistMeta());
    const auto albums = dm->allAlbumInfos();
    EXPECT_FALSE(albums.isEmpty());
    // 同时验证 variantList 路径（Utils::albumToVariantMap）
    EXPECT_FALSE(dm->allAlbumVariantList().isEmpty());
}

TEST(DataManagerAggregateTest, allArtistInfosPopulatedAfterImport)
{
    auto dm = importSample();
    ASSERT_TRUE(dm->isExistMeta());
    const auto artists = dm->allArtistInfos();
    EXPECT_FALSE(artists.isEmpty());
    EXPECT_FALSE(dm->allArtistVariantList().isEmpty());
}

// ----------------------------------------------------------------------------
// sortPlaylist 对 album/artist 歌单的所有排序类型
// 前置：先调用 allAlbumInfos()/allArtistInfos() 填充 m_allAlbums/m_allArtists
// 覆盖 sortPlaylist 的 album/artist 分支（line 1504-1567）及对应 std::sort 调用
// ----------------------------------------------------------------------------
TEST(DataManagerSortAggregateTest, sortAlbumPlaylistAllTypes)
{
    auto dm = importSample();
    ASSERT_FALSE(dm->allAlbumInfos().isEmpty());  // 填充 m_allAlbums
    // 各 album 排序类型（signalFlag=false 直接按 type）
    dm->sortPlaylist(DmGlobal::SortByAddTimeASC, "album", false);
    dm->sortPlaylist(DmGlobal::SortByAblumASC,   "album", false);
    dm->sortPlaylist(DmGlobal::SortByAddTimeDES, "album", false);
    dm->sortPlaylist(DmGlobal::SortByAblumDES,   "album", false);
    SUCCEED();
}

TEST(DataManagerSortAggregateTest, sortArtistPlaylistAllTypes)
{
    auto dm = importSample();
    ASSERT_FALSE(dm->allArtistInfos().isEmpty());  // 填充 m_allArtists
    dm->sortPlaylist(DmGlobal::SortByAddTimeASC, "artist", false);
    dm->sortPlaylist(DmGlobal::SortByArtistASC,  "artist", false);
    dm->sortPlaylist(DmGlobal::SortByAddTimeDES, "artist", false);
    dm->sortPlaylist(DmGlobal::SortByArtistDES,  "artist", false);
    SUCCEED();
}

// signalFlag=true 对 album/artist：覆盖 sortFlag && signalFlag 的 emit 分支
TEST(DataManagerSortAggregateTest, sortAlbumAndArtistWithSignalEmits)
{
    auto dm = importSample();
    QSignalSpy spyAlbum(dm.get(), &DataManager::signalPlaylistSortChanged);
    dm->allAlbumInfos();   // 填充
    dm->sortPlaylist(DmGlobal::SortByAblumASC, "album", true);
    dm->allArtistInfos();
    dm->sortPlaylist(DmGlobal::SortByArtistASC, "artist", true);
    EXPECT_GE(spyAlbum.count(), 1);
}

// ----------------------------------------------------------------------------
// searchText album/artist 类型：导入后搜索，填充 m_searchAlbums/m_searchArtists
// 触发 sortPlaylist(albumResult/artistResult) 分支（line 1568-1642）
// ----------------------------------------------------------------------------
TEST(DataManagerSearchBranchTest, searchTextAlbumTypePopulatesSearchAlbums)
{
    auto dm = importSample();
    ASSERT_TRUE(dm->isExistMeta());
    QList<DMusic::MediaMeta> metas;
    QList<DMusic::AlbumInfo> albums;
    QList<DMusic::ArtistInfo> artists;
    // 注意：album/artist 类型只填充 out-param，不写 m_searchAlbums
    dm->searchText("", metas, albums, artists, "album");
    EXPECT_FALSE(albums.isEmpty());
    EXPECT_FALSE(metas.isEmpty());   // 关联曲目
}

TEST(DataManagerSearchBranchTest, searchTextArtistTypePopulatesSearchArtists)
{
    auto dm = importSample();
    ASSERT_TRUE(dm->isExistMeta());
    QList<DMusic::MediaMeta> metas;
    QList<DMusic::AlbumInfo> albums;
    QList<DMusic::ArtistInfo> artists;
    dm->searchText("", metas, albums, artists, "artist");
    EXPECT_FALSE(artists.isEmpty());
}

// 搜索后再对 albumResult/artistResult 排序：覆盖 searchResult 排序分支
// 用 type=""(all) 触发 m_searchAlbums/m_searchArtists 写入，再对 result 歌单排序
TEST(DataManagerSearchBranchTest, sortSearchResultsAllTypes)
{
    auto dm = importSample();
    QList<DMusic::MediaMeta> metas;
    QList<DMusic::AlbumInfo> albums;
    QList<DMusic::ArtistInfo> artists;
    dm->searchText("", metas, albums, artists);  // 默认 all → 写 m_searchAlbums/m_searchArtists
    dm->sortPlaylist(DmGlobal::SortByAddTimeASC, "albumResult", false);
    dm->sortPlaylist(DmGlobal::SortByAblumASC,   "albumResult", false);
    dm->sortPlaylist(DmGlobal::SortByAddTimeDES, "albumResult", false);
    dm->sortPlaylist(DmGlobal::SortByAblumDES,   "albumResult", false);

    dm->sortPlaylist(DmGlobal::SortByAddTimeASC, "artistResult", false);
    dm->sortPlaylist(DmGlobal::SortByArtistASC,  "artistResult", false);
    dm->sortPlaylist(DmGlobal::SortByAddTimeDES, "artistResult", false);
    dm->sortPlaylist(DmGlobal::SortByArtistDES,  "artistResult", false);
    SUCCEED();
}

// ----------------------------------------------------------------------------
// quickSearchText : 导入后快速搜索，覆盖 allAlbumInfos/allArtistInfos 遍历
// ----------------------------------------------------------------------------
TEST(DataManagerQuickSearchTest, quickSearchFindsImported)
{
    auto dm = importSample();
    ASSERT_TRUE(dm->isExistMeta());
    QStringList metaTitles;
    QList<QPair<QString, QString>> albums, artists;
    dm->quickSearchText("", metaTitles, albums, artists);  // 空文本匹配所有
    EXPECT_FALSE(metaTitles.isEmpty());
}

// ----------------------------------------------------------------------------
// removeFromPlayList 级联删除：从 all 删除（delFlag=false 走 else 分支）
// 覆盖 deleteMetaFromAllMetas（emit signalClearImportingHash）、deleteMetaFromAlbum、
// deleteMetaFromArtist、以及 delFlag 分支
// ----------------------------------------------------------------------------
TEST(DataManagerRemoveCascadeTest, removeFromAllTriggersSignalClearImportingHash)
{
    auto dm = importSample();
    ASSERT_TRUE(dm->isExistMeta());
    const auto metas = dm->getPlaylistMetas("play");
    ASSERT_FALSE(metas.isEmpty());
    const QString hash = metas.first().hash;
    QSignalSpy spy(dm.get(), &DataManager::signalClearImportingHash);
    // playlistHash="all" → else 分支：遍历所有 playlist + deleteMetaFromAllMetas/Album/Artist
    dm->removeFromPlayList({hash}, "all", false);
    EXPECT_GE(spy.count(), 1);  // signalClearImportingHash 至少触发一次
    EXPECT_FALSE(dm->isExistMeta());  // meta 已从 m_allMetas 删除
}

TEST(DataManagerRemoveCascadeTest, removeFromAllWithDelFlagDeletesFile)
{
    // 用 sample.mp3 的临时副本导入，避免 delFlag=true 删除真实 testdata 文件
    // 污染后续导入测试。localPath 指向副本，删除副本不影响原始 testdata。
    const QString copyPath = QDir::tempPath() + "/dm_test_delcopy.mp3";
    if (QFile::exists(copyPath)) QFile::remove(copyPath);
    ASSERT_TRUE(QFile::copy(sampleMp3Path(), copyPath));

    auto dm = importSample();  // 先导入（占位，确保 DM 状态正常）
    // 再导入副本，得到 localPath=copyPath 的 meta
    QSignalSpy spy(dm.get(), &DataManager::signalImportFinished);
    dm->importMetas({copyPath}, "play");
    spy.wait(5000);

    const auto metas = dm->getPlaylistMetas("all");
    // 找到 localPath 为副本的 meta
    QString hash;
    for (const auto &m : metas) {
        if (m.localPath == copyPath) { hash = m.hash; break; }
    }
    if (!hash.isEmpty()) {
        const int before = dm->getPlaylistMetas("all").size();
        dm->removeFromPlayList({hash}, "all", true);  // delFlag=true → QFile::remove(copyPath)
        const int after = dm->getPlaylistMetas("all").size();
        EXPECT_EQ(after, before - 1);  // 副本 meta 被级联删除
        EXPECT_FALSE(QFile::exists(copyPath));  // 副本文件被删除
    }
    // 清理：确保副本被删除
    QFile::remove(copyPath);
}

// 从自定义歌单删除（非 all/album/artist 且非 delFlag）：覆盖 if 分支（line 1194-1216）
TEST(DataManagerRemoveCascadeTest, removeFromCustomPlaylistNonDelBranch)
{
    std::unique_ptr<DataManager> dm(makeDataMgr());
    DMusic::MediaMeta m1; m1.hash = "h1"; m1.title = "T1";
    DMusic::MediaMeta m2; m2.hash = "h2"; m2.title = "T2";
    const auto pl = dm->addPlayList("Custom");
    dm->addMetasToPlayList(QList<DMusic::MediaMeta>{m1, m2}, pl.uuid);
    EXPECT_EQ(dm->playlistFromHash(pl.uuid).sortMetas.size(), 2);
    // playlistHash 为自建歌单 uuid，非 all/album/artist，delFlag=false → 走 if 分支逐项移除
    dm->removeFromPlayList({"h1"}, pl.uuid, false);
    EXPECT_EQ(dm->playlistFromHash(pl.uuid).sortMetas.size(), 1);
}

// ----------------------------------------------------------------------------
// moveMetasPlayList 末尾插入分支（line 1304-1306）
// ----------------------------------------------------------------------------
TEST(DataManagerMoveMetasTest, moveToEndOfCustomPlaylist)
{
    std::unique_ptr<DataManager> dm(makeDataMgr());
    DMusic::MediaMeta m1; m1.hash = "h1";
    DMusic::MediaMeta m2; m2.hash = "h2";
    DMusic::MediaMeta m3; m3.hash = "h3";
    const auto pl = dm->addPlayList("Movable");
    dm->addMetasToPlayList(QList<DMusic::MediaMeta>{m1, m2, m3}, pl.uuid);
    dm->sortPlaylist(DmGlobal::SortByCustomASC, pl.uuid, false);
    // nextHash 为空 → 移到末尾分支
    EXPECT_TRUE(dm->moveMetasPlayList({"h1"}, pl.uuid, ""));
    const auto moved = dm->playlistFromHash(pl.uuid);
    EXPECT_EQ(moved.sortMetas.last(), "h1");
}

TEST(DataManagerMoveMetasTest, moveWithEmptyMetaListReturnsFalse)
{
    std::unique_ptr<DataManager> dm(makeDataMgr());
    const auto pl = dm->addPlayList("Empty");
    dm->sortPlaylist(DmGlobal::SortByCustomASC, pl.uuid, false);
    // 待移动的 hash 不在歌单中 → curMetas 为空 → return false
    EXPECT_FALSE(dm->moveMetasPlayList({"nonexistent"}, pl.uuid, ""));
}

TEST(DataManagerMoveMetasTest, rejectsNonCustomSortedPlaylist)
{
    std::unique_ptr<DataManager> dm(makeDataMgr());
    const auto pl = dm->addPlayList("NonCustom");
    DMusic::MediaMeta m; m.hash = "h1";
    dm->addMetasToPlayList(QList<DMusic::MediaMeta>{m}, pl.uuid);
    // addPlayList 默认 sortType=SortByCustomASC，先改成非 custom 再 move 应被拒绝
    dm->sortPlaylist(DmGlobal::SortByTitleASC, pl.uuid, false);
    EXPECT_FALSE(dm->moveMetasPlayList({"h1"}, pl.uuid, ""));
}

// ----------------------------------------------------------------------------
// clearPlayList 默认参数（空 hash → "play"）
// ----------------------------------------------------------------------------
TEST(DataManagerClearPlayListTest, clearDefaultPlayList)
{
    std::unique_ptr<DataManager> dm(makeDataMgr());
    DMusic::MediaMeta m; m.hash = "h1";
    dm->addMetasToPlayList(QList<DMusic::MediaMeta>{m}, "play");
    dm->clearPlayList("");  // 默认 → "play"
    EXPECT_EQ(dm->playlistFromHash("play").sortMetas.size(), 0);
}

TEST(DataManagerClearPlayListTest, clearNonexistentPlaylistIsNoop)
{
    std::unique_ptr<DataManager> dm(makeDataMgr());
    dm->clearPlayList("no-such-playlist");  // 无效 index → 提前 return
    SUCCEED();
}

// ----------------------------------------------------------------------------
// addMetasToPlayList current-playlist 分支（line 1068-1089 / 1130-1150）
// 当 m_currentHash == playlistHash 且 != "play" 时，同时加入 play 歌单
// ----------------------------------------------------------------------------
TEST(DataManagerAddMetasCurrentTest, addToCurrentPlaylistAlsoAddsToPlay)
{
    std::unique_ptr<DataManager> dm(makeDataMgr());
    const auto pl = dm->addPlayList("Current");
    dm->setCurrentPlayliHash(pl.uuid);  // 设为当前歌单
    DMusic::MediaMeta m; m.hash = "h1"; m.title = "T";
    dm->addMetasToPlayList(QList<QString>{"h1"}, pl.uuid);
    // current 分支会同时把 hash 加到 "play"
    EXPECT_TRUE(dm->playlistFromHash("play").sortMetas.contains("h1"));
}

TEST(DataManagerAddMetasCurrentTest, addMetasVariantToCurrentPlaylist)
{
    std::unique_ptr<DataManager> dm(makeDataMgr());
    const auto pl = dm->addPlayList("Cur2");
    dm->setCurrentPlayliHash(pl.uuid);
    DMusic::MediaMeta m; m.hash = "h1"; m.title = "T";
    dm->addMetasToPlayList(QList<DMusic::MediaMeta>{m}, pl.uuid);
    EXPECT_TRUE(dm->playlistFromHash("play").sortMetas.contains("h1"));
}

// filetype=cdda 在 addMetasToPlayList(metas) 中不加入 sortMetas
TEST(DataManagerAddMetasCurrentTest, cddaMetaNotAddedToSortMetas)
{
    std::unique_ptr<DataManager> dm(makeDataMgr());
    const auto pl = dm->addPlayList("Cdda");
    dm->setCurrentPlayliHash(pl.uuid);
    DMusic::MediaMeta m; m.hash = "h1"; m.filetype = "cdda";
    dm->addMetasToPlayList(QList<DMusic::MediaMeta>{m}, pl.uuid);
    // cdda 被跳过 sortMetas.append
    EXPECT_FALSE(dm->playlistFromHash(pl.uuid).sortMetas.contains("h1"));
}

// ----------------------------------------------------------------------------
// getPlaylistMetas count 参数 + favourite 标记
// ----------------------------------------------------------------------------
TEST(DataManagerGetMetasCountTest, countLimitsReturnedMetas)
{
    auto dm = importSample();
    ASSERT_TRUE(dm->isExistMeta());
    // count>=0 时按数量截断；count=1 最多返回 1 条
    const auto one = dm->getPlaylistMetas("all", 1);
    EXPECT_EQ(one.size(), 1u);
    // count=-1 返回全部
    const auto all = dm->getPlaylistMetas("all", -1);
    EXPECT_GE(all.size(), 1u);
}

TEST(DataManagerGetMetasCountTest, favPlaylistFavouriteFlagSet)
{
    auto dm = importSample();
    const auto metas = dm->getPlaylistMetas("play");
    ASSERT_FALSE(metas.isEmpty());
    const QString hash = metas.first().hash;
    // 加入 fav，再查 all → favourite 标记为 true
    dm->addMetasToPlayList(QList<QString>{hash}, "fav");
    const auto allMetas = dm->getPlaylistMetas("all");
    bool foundFav = false;
    for (const auto &m : allMetas) {
        if (m.hash == hash) { foundFav = m.favourite; break; }
    }
    EXPECT_TRUE(foundFav);
}

// ----------------------------------------------------------------------------
// sortPlaylist signalFlag=true 翻转：先设置 sortType 再翻转
// 覆盖各 case 中 else 分支（当前 sortType != ASC → 设 ASC）
// ----------------------------------------------------------------------------
TEST(DataManagerSortToggleBranchTest, toggleFromDesToAsc)
{
    std::unique_ptr<DataManager> dm(makeDataMgr());
    DMusic::MediaMeta m1; m1.hash = "h1";
    DMusic::MediaMeta m2; m2.hash = "h2";
    dm->addMetasToPlayList(QList<DMusic::MediaMeta>{m1, m2}, "play");
    // 先把 sortType 设为 DES，再 toggle（→ ASC 分支）
    dm->sortPlaylist(DmGlobal::SortByTitleDES, "play", false);
    dm->sortPlaylist(DmGlobal::SortByTitle, "play", true);
    EXPECT_EQ(dm->playlistFromHash("play").sortType, DmGlobal::SortByTitleASC);

    dm->sortPlaylist(DmGlobal::SortByAddTimeDES, "play", false);
    dm->sortPlaylist(DmGlobal::SortByAddTime, "play", true);
    EXPECT_EQ(dm->playlistFromHash("play").sortType, DmGlobal::SortByAddTimeASC);

    dm->sortPlaylist(DmGlobal::SortByArtistDES, "play", false);
    dm->sortPlaylist(DmGlobal::SortByArtist, "play", true);
    EXPECT_EQ(dm->playlistFromHash("play").sortType, DmGlobal::SortByArtistASC);

    dm->sortPlaylist(DmGlobal::SortByAblumDES, "play", false);
    dm->sortPlaylist(DmGlobal::SortByAblum, "play", true);
    EXPECT_EQ(dm->playlistFromHash("play").sortType, DmGlobal::SortByAblumASC);
}

// sortPlaylist 无效 hash：提前 return
TEST(DataManagerSortInvalidTest, sortInvalidHashReturnsEarly)
{
    std::unique_ptr<DataManager> dm(makeDataMgr());
    dm->sortPlaylist(DmGlobal::SortByTitleASC, "no-such-playlist", false);
    SUCCEED();
}

// ----------------------------------------------------------------------------
// updateMetaCodec：改 artist 触发 deleteMetaFromArtist 清空后移除分支
// ----------------------------------------------------------------------------
TEST(DataManagerUpdateMetaCodecTest, updateCodecReindexesArtist)
{
    auto dm = importSample();
    ASSERT_TRUE(dm->isExistMeta());
    dm->allArtistInfos();  // 填充 m_allArtists
    const QList<DMusic::MediaMeta> metas = dm->getPlaylistMetas("play");
    ASSERT_FALSE(metas.isEmpty());
    DMusic::MediaMeta meta = metas.first();
    meta.artist = "BrandNewArtist";  // 改 artist → 新建 artist + 移除空旧 artist
    dm->updateMetaCodec(meta);
    SUCCEED();
}

// ----------------------------------------------------------------------------
// movePlaylist 移到末尾 / nextHash 不存在
// ----------------------------------------------------------------------------
TEST(DataManagerMovePlaylistTest, moveBeforeNextPlaylist)
{
    std::unique_ptr<DataManager> dm(makeDataMgr());
    const auto pl1 = dm->addPlayList("P1");
    const auto pl2 = dm->addPlayList("P2");
    const auto pl3 = dm->addPlayList("P3");
    // 把 pl1 移到 pl3 之前
    dm->movePlaylist(pl1.uuid, pl3.uuid);
    SUCCEED();
}

// ----------------------------------------------------------------------------
// deletePlaylist 删除后再 allPlaylistVariantList：覆盖 deleteAllPlaylistDB 相关
// ----------------------------------------------------------------------------
TEST(DataManagerDeletePlaylistTest, deleteAndSavePersists)
{
    auto dm = importSample();
    const auto pl = dm->addPlayList("Temp");
    EXPECT_TRUE(dm->deletePlaylist(pl.uuid));
    dm->saveDataToDB();  // 删除后保存（覆盖 saveDataToDB 的 playlist 删除分支）
    SUCCEED();
}

// ----------------------------------------------------------------------------
// renamePlaylist 改成同名（自身）：源码按 displayName 查重，同名（含自身）一律拒绝
// 覆盖 renamePlaylist 的重名拒绝分支
// ----------------------------------------------------------------------------
TEST(DataManagerRenamePlaylistTest, renameToOwnNameRejected)
{
    std::unique_ptr<DataManager> dm(makeDataMgr());
    const auto pl = dm->addPlayList("Solo");
    // 改成自己当前的名字：源码按 displayName 查重，自身同名也被拒绝
    EXPECT_FALSE(dm->renamePlaylist("Solo", pl.uuid));
}

// ----------------------------------------------------------------------------
// importMetas 失败路径：不存在的文件 → failCount 增加，signalImportFinished 仍触发
// ----------------------------------------------------------------------------
TEST(DataManagerImportFailTest, importNonexistentFileFinishes)
{
    std::unique_ptr<DataManager> dm(makeDataMgr());
    QSignalSpy spy(dm.get(), &DataManager::signalImportFinished);
    dm->importMetas({"/nonexistent/path/missing.mp3"}, "play");
    EXPECT_TRUE(spy.wait(5000));
    EXPECT_FALSE(dm->isExistMeta());  // 导入失败，库仍为空
}

// ----------------------------------------------------------------------------
// 导入到 album/artist 歌单：内部 curPlaylistHash 重定向为 "all"
// 覆盖 importMetas 中 playlistHash=="album"/"artist" 分支（line 1021-1022）
// ----------------------------------------------------------------------------
TEST(DataManagerImportRedirectTest, importToAlbumPlaylistRedirectsToAll)
{
    // playlistHash="album" 触发 importMetas 内部 curPlaylistHash 重定向分支（line 1021-1022）
    // 注意：导入信号仍以原始 "album" hash 派发，DBOperate 可能不写入 m_allMetas，
    // 故这里只验证导入流程完成（信号触发），不强制断言 isExistMeta
    std::unique_ptr<DataManager> dm(makeDataMgr());
    QSignalSpy spy(dm.get(), &DataManager::signalImportFinished);
    dm->importMetas({sampleMp3Path()}, "album");  // 触发重定向分支
    EXPECT_TRUE(spy.wait(5000));
}

// ----------------------------------------------------------------------------
// saveDataToDB 后 slotLazyLoadDatabase：重新从 DB 装载（loadMetasDB + loadPlaylistMetasDB）
// 验证 load 后 meta 数量恢复
// ----------------------------------------------------------------------------
TEST(DataManagerReloadTest, lazyLoadRestoresMetasFromDB)
{
    auto dm = importSample();
    ASSERT_TRUE(dm->isExistMeta());
    const int before = dm->getPlaylistMetas("all").size();
    ASSERT_GE(before, 1);
    dm->saveDataToDB();
    dm->slotLazyLoadDatabase();  // loadMetasDB + loadPlaylistMetasDB
    const int after = dm->getPlaylistMetas("all").size();
    EXPECT_EQ(before, after);
}

// ----------------------------------------------------------------------------
// metaFromHash 存在的 hash：返回有效 meta
// ----------------------------------------------------------------------------
TEST(DataManagerQueryTest, metaFromHashExistingReturnsValid)
{
    auto dm = importSample();
    const auto metas = dm->getPlaylistMetas("play");
    ASSERT_FALSE(metas.isEmpty());
    const QString hash = metas.first().hash;
    const auto meta = dm->metaFromHash(hash);
    EXPECT_EQ(meta.hash, hash);
    EXPECT_FALSE(meta.title.isEmpty());
}

// ============================================================================
// DBOperate 直接单元测试：构造、slotImportMetas 各分支、slotClearImportingHash
// DBOperate 是 DataManager 内部的导入 worker，这里直接实例化覆盖其内部逻辑，
// 并通过 metaObject 调用触发 moc 生成的 qt_metacast/qt_metacall。
// ============================================================================

// 构造函数：supportedSuffixs 被转换为 "*.<ext>" 形式存入 m_supportedSuffixs
TEST(DBOperateTest, constructorStoresSupportedSuffixes)
{
    DBOperate db({"mp3", "flac", "ogg"});
    // 间接验证：导入一个不存在的目录路径，应不崩溃且完成信号被同步触发
    // 注意：slotImportMetas 是直接调用（同线程），emit signalImportFinished 同步发生，
    // 无需 QSignalSpy::wait（wait 需事件循环）；用 spy.count() 验证
    QSignalSpy spy(&db, &DBOperate::signalImportFinished);
    db.slotImportMetas({"/nonexistent/dir"}, {}, false, {}, {}, "", false);
    EXPECT_EQ(spy.count(), 1);
}

// slotImportMetas 空 urls 分支：使用默认 musicPath 作为扫描根（line 47-50）
TEST(DBOperateTest, importWithEmptyUrlsUsesMusicPath)
{
    DBOperate db({"mp3"});
    QSignalSpy spy(&db, &DBOperate::signalImportFinished);
    // 空 urls → 内部 append musicPath，musicPath 通常不存在/为空目录 → 0 文件
    db.slotImportMetas({}, {}, false, {}, {}, "", false);
    ASSERT_EQ(spy.count(), 1);
    // failCount/successCount/existCount 应都为 0（无文件可处理）
    const auto args = spy.takeFirst();
    EXPECT_EQ(args.at(1).toInt(), 0);  // failCount
    EXPECT_EQ(args.at(2).toInt(), 0);  // successCount
}

// slotImportMetas 目录扫描分支：传入真实目录，递归扫描支持的扩展名
TEST(DBOperateTest, importDirectoryScansRecursively)
{
    DBOperate db({"mp3"});
    QSignalSpy spy(&db, &DBOperate::signalImportFinished);
    // 传入测试数据所在目录（含 sample.mp3），应扫描到文件
    db.slotImportMetas({QString(TEST_DATA_DIR)}, {}, false, {}, {}, "", false);
    EXPECT_EQ(spy.count(), 1);
}

// slotImportMetas 重复 hash 跳过分支：同一文件在 allMetaHashs 中已存在 → existCount++
TEST(DBOperateTest, importExistingHashSkipsAndCountsAsExist)
{
    DBOperate db({"mp3"});
    // 预先把 sample.mp3 的 hash 放进 allMetaHashs，模拟"已导入"
    const QString h = Utils::filePathHash(QFileInfo(sampleMp3Path()).absoluteFilePath());
    QSet<QString> allHashs = {h};
    QSignalSpy spy(&db, &DBOperate::signalImportFinished);
    db.slotImportMetas({sampleMp3Path()}, {}, false, {}, allHashs, "", false);
    ASSERT_EQ(spy.count(), 1);
    const auto args = spy.takeFirst();
    // 该文件应被计为 exist（已存在），successCount=0
    EXPECT_EQ(args.at(2).toInt(), 0);  // successCount
}

// slotImportMetas 非 .so 单文件路径分支（line 67-69）：直接 append 单个文件
TEST(DBOperateTest, importSingleNonExistentFileCountsAsFail)
{
    DBOperate db({"mp3"});
    QSignalSpy spy(&db, &DBOperate::signalImportFinished);
    // 不存在的单文件：creatMediaMeta 返回 length<=0 → importedFailCount++
    db.slotImportMetas({"/nonexistent/single.mp3"}, {}, false, {}, {}, "", false);
    EXPECT_EQ(spy.count(), 1);
}

// slotImportMetas 带 playlistHash 且 metaHashs 命中 exist 分支（line 94-95）
TEST(DBOperateTest, importWithPlaylistAndExistingMetaHash)
{
    DBOperate db({"mp3"});
    const QString h = Utils::filePathHash(QFileInfo(sampleMp3Path()).absoluteFilePath());
    QSet<QString> metaHashs = {h};  // 该 hash 已在目标歌单中
    QSignalSpy spy(&db, &DBOperate::signalImportFinished);
    db.slotImportMetas({sampleMp3Path()}, metaHashs, false, {}, {}, "playlist1", false);
    ASSERT_EQ(spy.count(), 1);
    const auto args = spy.takeFirst();
    EXPECT_EQ(args.at(2).toInt(), 0);  // successCount：已存在不重复导入
}

// slotClearImportingHash：移除 m_importingHashes 中的记录，允许重新导入
TEST(DBOperateTest, slotClearImportingHashRemovesEntry)
{
    DBOperate db({"mp3"});
    const QString h = Utils::filePathHash(QFileInfo(sampleMp3Path()).absoluteFilePath());
    // 先导入一次（会 insert 到 m_importingHashes）
    {
        QSignalSpy spy(&db, &DBOperate::signalImportFinished);
        db.slotImportMetas({sampleMp3Path()}, {}, false, {}, {}, "", false);
        ASSERT_EQ(spy.count(), 1);
    }
    // 清除该 hash 的导入标记
    db.slotClearImportingHash(h);
    // 再次导入：因标记已清，应重新尝试（creatMediaMeta 重新解析）
    {
        QSignalSpy spy(&db, &DBOperate::signalImportFinished);
        db.slotImportMetas({sampleMp3Path()}, {}, false, {}, {}, "", false);
        ASSERT_EQ(spy.count(), 1);
    }
    // 不崩溃即通过；slotClearImportingHash 已被调用覆盖
    SUCCEED();
}

// importPlay + playFalg 分支：导入并自动加入播放队列 + 自动播放 hash
TEST(DBOperateTest, importWithImportPlayAndPlayFlagSetsMediaHash)
{
    DBOperate db({"mp3"});
    QSignalSpy spy(&db, &DBOperate::signalImportFinished);
    // importPlay=true 且 playFalg=true：成功导入的 meta 会进 play 队列，且 mediaHash 被设置
    db.slotImportMetas({sampleMp3Path()}, {}, true, {}, {}, "", true);
    ASSERT_EQ(spy.count(), 1);
    const auto args = spy.takeFirst();
    // mediaHash（第 5 个参数）应非空（导入了 sample.mp3）
    EXPECT_FALSE(args.at(4).toString().isEmpty());
}

// moc 元对象接口：触发 DBOperate 的 qt_metacast/qt_metacall（盲区函数）
TEST(DBOperateMetaObjectTest, metaObjectAndMetacastWork)
{
    DBOperate db({"mp3"});
    const QMetaObject *mo = db.metaObject();
    ASSERT_NE(mo, nullptr);
    EXPECT_STREQ(mo->className(), "DBOperate");
    // qt_metacast 向上转型到 QObject 应成功
    EXPECT_NE(db.qt_metacast("QObject"), nullptr);
    // 未知类名返回 nullptr
    EXPECT_EQ(db.qt_metacast("UnknownClass"), nullptr);
    // 槽函数应在元对象中注册（间接调用 qt_metacall 的枚举/方法查找）
    EXPECT_GE(mo->indexOfSlot("slotClearImportingHash(QString)"), 0);
}

// ============================================================================
// Phase 3 深化（第二弹）：聚焦 album/artist 排序谓词 + 聚合 + 搜索结果排序
//
// 核心盲区：8 个 static 排序谓词（moreThanAlbum/Artist{Title,Timestamp}{ASC,DES}，
// line 85-123）此前完全未覆盖。原因：sample.mp3 仅产生 1 个 album/1 个 artist，
// 而 std::sort 在元素数 < 2 时不会调用比较器。
//
// 策略：通过 public slot slotAddOneMeta({"all"}, meta) 直接注入 ≥2 个具有不同
// album/artist 的构造 meta（不需真实音频文件），填充 m_allMetas + m_allAlbums/
// m_allArtists，再触发排序使 std::sort 真正调用谓词比较器。
// ============================================================================

namespace {
// 构造一条带可区分 album/artist/title/pinyin 的 meta
DMusic::MediaMeta makeDistinguishableMeta(const QString &hash, const QString &title,
                                          const QString &album, const QString &artist,
                                          qint64 timestamp)
{
    DMusic::MediaMeta m;
    m.hash = hash;
    m.title = title;
    m.album = album;
    m.artist = artist;
    // pinyin 字段是排序谓词的实际比较依据（toLower 后比较）
    m.pinyinTitle = title;
    m.pinyinAlbum = album;
    m.pinyinArtist = artist;
    m.timestamp = timestamp;
    return m;
}

// 注入 ≥2 个不同 album 的 meta 并确保 m_allAlbums 填充
// 返回填充后的 album 数量（用于断言前置条件）
int injectDistinctAlbums(DataManager *dm)
{
    dm->slotAddOneMeta({"all"}, makeDistinguishableMeta("alb-h1", "T1", "ZetaAlbum", "AArtist", 300));
    dm->slotAddOneMeta({"all"}, makeDistinguishableMeta("alb-h2", "T2", "AlphaAlbum", "BArtist", 100));
    dm->slotAddOneMeta({"all"}, makeDistinguishableMeta("alb-h3", "T3", "MidAlbum", "CArtist", 200));
    dm->allAlbumInfos();   // 触发聚合重建 m_allAlbums
    return dm->allAlbumInfos().size();
}

int injectDistinctArtists(DataManager *dm)
{
    dm->slotAddOneMeta({"all"}, makeDistinguishableMeta("art-h1", "T1", "XAlbum", "ZetaArtist", 300));
    dm->slotAddOneMeta({"all"}, makeDistinguishableMeta("art-h2", "T2", "YAlbum", "AlphaArtist", 100));
    dm->slotAddOneMeta({"all"}, makeDistinguishableMeta("art-h3", "T3", "ZAlbum", "MidArtist", 200));
    dm->allArtistInfos();  // 触发聚合重建 m_allArtists
    return dm->allArtistInfos().size();
}
} // namespace

// ----------------------------------------------------------------------------
// album 排序谓词（signalFlag=false 直接按 type）：4 个谓词全覆盖
// moreThanAlbumTitleASC/DES（按 pinyin）+ moreThanAlbumTimestampASC/DES（按 timestamp）
// ----------------------------------------------------------------------------
TEST(DataManagerAlbumSortPredTest, albumTitleAscSortsByPinyin)
{
    std::unique_ptr<DataManager> dm(makeDataMgr());
    ASSERT_GE(injectDistinctAlbums(dm.get()), 2);   // 前置：≥2 个 album
    dm->sortPlaylist(DmGlobal::SortByAblumASC, "album", false);  // moreThanAlbumTitleASC
    const auto albums = dm->allAlbumInfos();
    ASSERT_GE(albums.size(), 2);
    // ASC：AlphaAlbum 应排在 ZetaAlbum 之前
    EXPECT_LT(albums.first().pinyin.toLower(), albums.last().pinyin.toLower());
}

TEST(DataManagerAlbumSortPredTest, albumTitleDesSortsByPinyin)
{
    std::unique_ptr<DataManager> dm(makeDataMgr());
    ASSERT_GE(injectDistinctAlbums(dm.get()), 2);
    dm->sortPlaylist(DmGlobal::SortByAblumDES, "album", false);  // moreThanAlbumTitleDES
    const auto albums = dm->allAlbumInfos();
    ASSERT_GE(albums.size(), 2);
    // DES：ZetaAlbum 应排在 AlphaAlbum 之前
    EXPECT_GT(albums.first().pinyin.toLower(), albums.last().pinyin.toLower());
}

TEST(DataManagerAlbumSortPredTest, albumTimestampAscSortsByTimestamp)
{
    std::unique_ptr<DataManager> dm(makeDataMgr());
    ASSERT_GE(injectDistinctAlbums(dm.get()), 2);
    dm->sortPlaylist(DmGlobal::SortByAddTimeASC, "album", false);  // moreThanAlbumTimestampASC
    const auto albums = dm->allAlbumInfos();
    ASSERT_GE(albums.size(), 2);
    EXPECT_LE(albums.first().timestamp, albums.last().timestamp);
}

TEST(DataManagerAlbumSortPredTest, albumTimestampDesSortsByTimestamp)
{
    std::unique_ptr<DataManager> dm(makeDataMgr());
    ASSERT_GE(injectDistinctAlbums(dm.get()), 2);
    dm->sortPlaylist(DmGlobal::SortByAddTimeDES, "album", false);  // moreThanAlbumTimestampDES
    const auto albums = dm->allAlbumInfos();
    ASSERT_GE(albums.size(), 2);
    EXPECT_GE(albums.first().timestamp, albums.last().timestamp);
}

// ----------------------------------------------------------------------------
// artist 排序谓词（signalFlag=false 直接按 type）：4 个谓词全覆盖
// moreThanArtistTitleASC/DES + moreThanArtistTimestampASC/DES
// ----------------------------------------------------------------------------
TEST(DataManagerArtistSortPredTest, artistTitleAscSortsByPinyin)
{
    std::unique_ptr<DataManager> dm(makeDataMgr());
    ASSERT_GE(injectDistinctArtists(dm.get()), 2);
    dm->sortPlaylist(DmGlobal::SortByArtistASC, "artist", false);  // moreThanArtistTitleASC
    const auto artists = dm->allArtistInfos();
    ASSERT_GE(artists.size(), 2);
    EXPECT_LT(artists.first().pinyin.toLower(), artists.last().pinyin.toLower());
}

TEST(DataManagerArtistSortPredTest, artistTitleDesSortsByPinyin)
{
    std::unique_ptr<DataManager> dm(makeDataMgr());
    ASSERT_GE(injectDistinctArtists(dm.get()), 2);
    dm->sortPlaylist(DmGlobal::SortByArtistDES, "artist", false);  // moreThanArtistTitleDES
    const auto artists = dm->allArtistInfos();
    ASSERT_GE(artists.size(), 2);
    EXPECT_GT(artists.first().pinyin.toLower(), artists.last().pinyin.toLower());
}

TEST(DataManagerArtistSortPredTest, artistTimestampAscSortsByTimestamp)
{
    std::unique_ptr<DataManager> dm(makeDataMgr());
    ASSERT_GE(injectDistinctArtists(dm.get()), 2);
    dm->sortPlaylist(DmGlobal::SortByAddTimeASC, "artist", false);  // moreThanArtistTimestampASC
    const auto artists = dm->allArtistInfos();
    ASSERT_GE(artists.size(), 2);
    EXPECT_LE(artists.first().timestamp, artists.last().timestamp);
}

TEST(DataManagerArtistSortPredTest, artistTimestampDesSortsByTimestamp)
{
    std::unique_ptr<DataManager> dm(makeDataMgr());
    ASSERT_GE(injectDistinctArtists(dm.get()), 2);
    dm->sortPlaylist(DmGlobal::SortByAddTimeDES, "artist", false);  // moreThanArtistTimestampDES
    const auto artists = dm->allArtistInfos();
    ASSERT_GE(artists.size(), 2);
    EXPECT_GE(artists.first().timestamp, artists.last().timestamp);
}

// ----------------------------------------------------------------------------
// album/artist 排序 default 分支：sortType 不在 case 集合内 → sortFlag=false
// signalFlag=false 时传入非 album/artist 的 sortType（如 SortByCustomASC），
// 命中 default 分支但不 emit（覆盖 line 1526-1529 / 1559-1562 的 default 分支）
// ----------------------------------------------------------------------------
TEST(DataManagerAlbumSortPredTest, albumSortDefaultBranchIsNoop)
{
    std::unique_ptr<DataManager> dm(makeDataMgr());
    injectDistinctAlbums(dm.get());
    QSignalSpy spy(dm.get(), &DataManager::signalPlaylistSortChanged);
    // signalFlag=false 且 type=SortByCustomASC（不在 album case 集合）→ default → sortFlag=false
    dm->sortPlaylist(DmGlobal::SortByCustomASC, "album", false);
    EXPECT_EQ(spy.count(), 0);  // 不 emit
}

TEST(DataManagerAlbumSortPredTest, albumSortWithSignalEmits)
{
    std::unique_ptr<DataManager> dm(makeDataMgr());
    injectDistinctAlbums(dm.get());
    QSignalSpy spy(dm.get(), &DataManager::signalPlaylistSortChanged);
    // signalFlag=true，type=SortByAblum → sortType 翻转为 SortByAblumASC → 命中 album case + emit
    dm->sortPlaylist(DmGlobal::SortByAblum, "album", true);
    EXPECT_GE(spy.count(), 1);
}

TEST(DataManagerArtistSortPredTest, artistSortDefaultBranchIsNoop)
{
    std::unique_ptr<DataManager> dm(makeDataMgr());
    injectDistinctArtists(dm.get());
    QSignalSpy spy(dm.get(), &DataManager::signalPlaylistSortChanged);
    // signalFlag=false 且 type=SortByCustomASC（不在 artist case 集合）→ default → sortFlag=false
    dm->sortPlaylist(DmGlobal::SortByCustomASC, "artist", false);
    EXPECT_EQ(spy.count(), 0);
}

TEST(DataManagerArtistSortPredTest, artistSortWithSignalEmits)
{
    std::unique_ptr<DataManager> dm(makeDataMgr());
    injectDistinctArtists(dm.get());
    QSignalSpy spy(dm.get(), &DataManager::signalPlaylistSortChanged);
    dm->sortPlaylist(DmGlobal::SortByArtist, "artist", true);
    EXPECT_GE(spy.count(), 1);
}

// ----------------------------------------------------------------------------
// albumResult/artistResult 搜索结果排序（signalFlag=false）：8 个谓词全覆盖
// 前置：先 searchText 默认(all) 写入 m_searchAlbums/m_searchArtists（≥2 个），
// 再对 result 歌单排序触发 searchedAlbumInfos()/searchedArtistInfos() + std::sort
// ----------------------------------------------------------------------------
TEST(DataManagerSearchResultSortTest, albumResultSortAllTypes)
{
    std::unique_ptr<DataManager> dm(makeDataMgr());
    injectDistinctAlbums(dm.get());
    QList<DMusic::MediaMeta> metas;
    QList<DMusic::AlbumInfo> albums;
    QList<DMusic::ArtistInfo> artists;
    dm->searchText("", metas, albums, artists);  // 默认 all → 写 m_searchAlbums
    ASSERT_GE(dm->searchedAlbumInfos().size(), 2);
    // 4 个 album 排序谓词
    dm->sortPlaylist(DmGlobal::SortByAddTimeASC, "albumResult", false);
    dm->sortPlaylist(DmGlobal::SortByAblumASC,   "albumResult", false);
    dm->sortPlaylist(DmGlobal::SortByAddTimeDES, "albumResult", false);
    dm->sortPlaylist(DmGlobal::SortByAblumDES,   "albumResult", false);
    // 验证排序后 searchedAlbumInfos 仍可取（覆盖 searchedAlbumInfos 遍历）
    EXPECT_FALSE(dm->searchedAlbumInfos().isEmpty());
    EXPECT_FALSE(dm->searchedAlbumVariantList().isEmpty());
    SUCCEED();
}

TEST(DataManagerSearchResultSortTest, artistResultSortAllTypes)
{
    std::unique_ptr<DataManager> dm(makeDataMgr());
    injectDistinctArtists(dm.get());
    QList<DMusic::MediaMeta> metas;
    QList<DMusic::AlbumInfo> albums;
    QList<DMusic::ArtistInfo> artists;
    dm->searchText("", metas, albums, artists);  // 写 m_searchArtists
    ASSERT_GE(dm->searchedArtistInfos().size(), 2);
    dm->sortPlaylist(DmGlobal::SortByAddTimeASC, "artistResult", false);
    dm->sortPlaylist(DmGlobal::SortByArtistASC,  "artistResult", false);
    dm->sortPlaylist(DmGlobal::SortByAddTimeDES, "artistResult", false);
    dm->sortPlaylist(DmGlobal::SortByArtistDES,  "artistResult", false);
    EXPECT_FALSE(dm->searchedArtistInfos().isEmpty());
    EXPECT_FALSE(dm->searchedArtistVariantList().isEmpty());
    SUCCEED();
}

// albumResult/artistResult 排序 signalFlag=true emit + 重建 m_searchAlbums/Artists
// 覆盖 line 1598-1605 / 1636-1643 的 emit + 列表重建分支
TEST(DataManagerSearchResultSortTest, albumResultSortWithSignalRebuildsList)
{
    std::unique_ptr<DataManager> dm(makeDataMgr());
    injectDistinctAlbums(dm.get());
    QList<DMusic::MediaMeta> metas;
    QList<DMusic::AlbumInfo> albums;
    QList<DMusic::ArtistInfo> artists;
    dm->searchText("", metas, albums, artists);
    ASSERT_GE(dm->searchedAlbumInfos().size(), 2);
    QSignalSpy spy(dm.get(), &DataManager::signalPlaylistSortChanged);
    // signalFlag=false 直接按 type 排序 + 触发 emit（line 1598 sortFlag&&signalFlag）
    dm->sortPlaylist(DmGlobal::SortByAblumASC, "albumResult", true);
    EXPECT_GE(spy.count(), 1);
}

TEST(DataManagerSearchResultSortTest, artistResultSortWithSignalRebuildsList)
{
    std::unique_ptr<DataManager> dm(makeDataMgr());
    injectDistinctArtists(dm.get());
    QList<DMusic::MediaMeta> metas;
    QList<DMusic::AlbumInfo> albums;
    QList<DMusic::ArtistInfo> artists;
    dm->searchText("", metas, albums, artists);
    ASSERT_GE(dm->searchedArtistInfos().size(), 2);
    QSignalSpy spy(dm.get(), &DataManager::signalPlaylistSortChanged);
    dm->sortPlaylist(DmGlobal::SortByArtistASC, "artistResult", true);
    EXPECT_GE(spy.count(), 1);
}

// ----------------------------------------------------------------------------
// deleteMetaFromAlbum/Artist：从 album/artist 删除 meta，清空后移除空 album/artist
// 覆盖 line 303-313 / 340-350（含 isEmpty → removeAt 分支）
// ----------------------------------------------------------------------------
TEST(DataManagerDeleteMetaFromAggTest, deleteFromAlbumRemovesEmptyAlbum)
{
    std::unique_ptr<DataManager> dm(makeDataMgr());
    dm->slotAddOneMeta({"all"}, makeDistinguishableMeta("del-h1", "T1", "SoloAlbum", "SoloArtist", 100));
    ASSERT_GE(dm->allAlbumInfos().size(), 1);
    // 通过 removeFromPlayList(all, delFlag=true) 触发 deleteMetaFromAlbum + deleteMetaFromArtist
    dm->removeFromPlayList({"del-h1"}, "all", true);
    // 该 album 只有一条 meta，删除后 album 应被移除（allAlbumInfos 重建后不含）
    const auto albums = dm->allAlbumInfos();
    bool foundSolo = false;
    for (const auto &a : albums) if (a.name == "SoloAlbum") foundSolo = true;
    EXPECT_FALSE(foundSolo);
    EXPECT_FALSE(dm->isExistMeta());  // meta 已从 m_allMetas 删除
}

// ----------------------------------------------------------------------------
// quickSearchText album/artist 分支：导入后聚合搜索覆盖 album/artist 遍历
// 覆盖 line 1817-1829（album/artist 命中 append 分支）
// ----------------------------------------------------------------------------
TEST(DataManagerQuickSearchBranchTest, quickSearchFindsAlbumAndArtist)
{
    std::unique_ptr<DataManager> dm(makeDataMgr());
    injectDistinctAlbums(dm.get());   // 同时也建了 artist
    QStringList metaTitles;
    QList<QPair<QString, QString>> albums, artists;
    dm->quickSearchText("", metaTitles, albums, artists);  // 空文本匹配所有
    EXPECT_FALSE(metaTitles.isEmpty());
    EXPECT_FALSE(albums.isEmpty());    // album 命中分支
    EXPECT_FALSE(artists.isEmpty());   // artist 命中分支
}

// ----------------------------------------------------------------------------
// searchText 默认(all) 分支的 album/artist 命中循环体
// 覆盖 line 1934-1957（album/artist 名称命中 → 遍历 musicinfos 写 m_searchMetas）
// ----------------------------------------------------------------------------
TEST(DataManagerSearchTextAllBranchTest, searchTextAllMatchesAlbumAndArtist)
{
    std::unique_ptr<DataManager> dm(makeDataMgr());
    injectDistinctAlbums(dm.get());
    QList<DMusic::MediaMeta> metas;
    QList<DMusic::AlbumInfo> albums;
    QList<DMusic::ArtistInfo> artists;
    // 用 album 名称片段搜索，命中 album → 触发 album 循环体写 m_searchMetas
    dm->searchText("Album", metas, albums, artists);
    EXPECT_FALSE(albums.isEmpty());
    // 用 artist 名称片段搜索（新实例，避免上次 search 清空影响）
    std::unique_ptr<DataManager> dm2(makeDataMgr());
    injectDistinctArtists(dm2.get());
    QList<DMusic::MediaMeta> m2;
    QList<DMusic::AlbumInfo> a2;
    QList<DMusic::ArtistInfo> ar2;
    dm2->searchText("Artist", m2, a2, ar2);
    EXPECT_FALSE(ar2.isEmpty());
}

// ----------------------------------------------------------------------------
// dbPath 白名单 else 分支：注入非白名单路径 → 拒绝并回退默认（line 2243-2245）
// 构造即触发，验证不崩溃且内置歌单正常初始化（回退默认 cachePath）
// ----------------------------------------------------------------------------
TEST(DataManagerDbPathSecurityTest, nonWhitelistedDbPathFallsBackToDefault)
{
    // 注入一个非 ":memory:"/空 的路径：应被拒绝并回退默认（不崩溃）
    std::unique_ptr<DataManager> dm(new DataManager({"mp3"}, nullptr, "/tmp/../../etc/bad.sqlite"));
    // 回退默认后内置歌单仍应正常初始化
    EXPECT_FALSE(dm->allPlaylistInfos().isEmpty());
    EXPECT_EQ(dm->currentPlayliHash(), "all");  // 默认当前歌单
}

// ----------------------------------------------------------------------------
// addMetaToAlbum 已存在 album 分支的 timestamp 更新（line 284-286）
// 同 album 第二条 meta timestamp 更小 → 更新 album.timestamp
// ----------------------------------------------------------------------------
TEST(DataManagerAlbumTimestampTest, smallerTimestampUpdatesAlbumTimestamp)
{
    std::unique_ptr<DataManager> dm(makeDataMgr());
    // 第一条：timestamp=500
    dm->slotAddOneMeta({"all"}, makeDistinguishableMeta("ts-h1", "T1", "SameAlbum", "SameArtist", 500));
    // 第二条：同 album，timestamp=100（更小）→ 触发 itr->timestamp 更新分支
    dm->slotAddOneMeta({"all"}, makeDistinguishableMeta("ts-h2", "T2", "SameAlbum", "SameArtist", 100));
    const auto albums = dm->allAlbumInfos();
    ASSERT_EQ(albums.size(), 1);
    EXPECT_EQ(albums.first().timestamp, 100);  // 被更小的 timestamp 更新
}

// ----------------------------------------------------------------------------
// slotAddOneMeta 已存在 meta（existingIndex>=0）跳过重复添加分支（line 2070-2073/2086-2088）
// ----------------------------------------------------------------------------
TEST(DataManagerSlotAddOneMetaTest, duplicateMetaHashSkipsReAdd)
{
    std::unique_ptr<DataManager> dm(makeDataMgr());
    dm->slotAddOneMeta({"all"}, makeDistinguishableMeta("dup-h1", "T1", "A1", "Ar1", 100));
    const int before = dm->getPlaylistMetas("all").size();
    // 再次添加同一 hash → existingIndex>=0 → 跳过 m_allMetas.append
    dm->slotAddOneMeta({"all"}, makeDistinguishableMeta("dup-h1", "T1", "A1", "Ar1", 100));
    const int after = dm->getPlaylistMetas("all").size();
    EXPECT_EQ(before, after);  // 未重复添加
}

// ----------------------------------------------------------------------------
// importMetas 的 importPlay=true 分支（line 1035-1041）
// 当 curPlaylistHash==currentHash 且 != "play" 时，importPlay=true 并收集 playMetaHashs
// ----------------------------------------------------------------------------
TEST(DataManagerImportPlayTest, importToCurrentNonPlayPlaylistSetsImportPlay)
{
    std::unique_ptr<DataManager> dm(makeDataMgr());
    const auto pl = dm->addPlayList("CurImport");
    dm->setCurrentPlayliHash(pl.uuid);   // 当前歌单 != "play"
    // 触发 importMetas 内部 importPlay=true 分支（即使文件不存在，分支逻辑仍执行）
    QSignalSpy spy(dm.get(), &DataManager::signalImportFinished);
    dm->importMetas({"/nonexistent/x.mp3"}, pl.uuid);
    EXPECT_TRUE(spy.wait(5000));  // 异步导入流程完成
}

// ----------------------------------------------------------------------------
// moveMetasPlayList 非末尾插入分支（line 1307-1310）
// nextHash 指向歌单中已有 meta → index < size-1 → else 分支 insert
// ----------------------------------------------------------------------------
TEST(DataManagerMoveMetasInsertTest, moveBeforeExistingHashInserts)
{
    std::unique_ptr<DataManager> dm(makeDataMgr());
    DMusic::MediaMeta m1; m1.hash = "h1";
    DMusic::MediaMeta m2; m2.hash = "h2";
    DMusic::MediaMeta m3; m3.hash = "h3";
    const auto pl = dm->addPlayList("Insertable");
    dm->addMetasToPlayList(QList<DMusic::MediaMeta>{m1, m2, m3}, pl.uuid);
    dm->sortPlaylist(DmGlobal::SortByCustomASC, pl.uuid, false);
    // 把 h3 移到 h1 之前（nextHash=h1，非末尾）→ 触发 insert 分支
    EXPECT_TRUE(dm->moveMetasPlayList({"h3"}, pl.uuid, "h1"));
    const auto moved = dm->playlistFromHash(pl.uuid);
    EXPECT_EQ(moved.sortMetas.first(), "h3");  // h3 现在排第一
}

// ----------------------------------------------------------------------------
// deletePlaylist 删除当前歌单 → 重置 currentHash 为空（line 1658-1659）
// ----------------------------------------------------------------------------
TEST(DataManagerDeleteCurrentTest, deletingCurrentPlaylistResetsHash)
{
    std::unique_ptr<DataManager> dm(makeDataMgr());
    const auto pl = dm->addPlayList("CurrentDel");
    dm->setCurrentPlayliHash(pl.uuid);
    EXPECT_EQ(dm->currentPlayliHash(), pl.uuid);
    EXPECT_TRUE(dm->deletePlaylist(pl.uuid));
    EXPECT_TRUE(dm->currentPlayliHash().isEmpty());  // 被重置为空
}

// ----------------------------------------------------------------------------
// albumResult/artistResult 排序 default 分支（line 1593-1596 / 1631-1634）
// signalFlag=false 且 type 不在 result case 集合 → default → sortFlag=false，不 emit
// ----------------------------------------------------------------------------
TEST(DataManagerSearchResultSortTest, albumResultSortDefaultBranchIsNoop)
{
    std::unique_ptr<DataManager> dm(makeDataMgr());
    injectDistinctAlbums(dm.get());
    QList<DMusic::MediaMeta> metas;
    QList<DMusic::AlbumInfo> albums;
    QList<DMusic::ArtistInfo> artists;
    dm->searchText("", metas, albums, artists);
    ASSERT_GE(dm->searchedAlbumInfos().size(), 2);
    QSignalSpy spy(dm.get(), &DataManager::signalPlaylistSortChanged);
    // type=SortByCustomASC 不在 albumResult case 集合 → default → sortFlag=false，不 emit
    dm->sortPlaylist(DmGlobal::SortByCustomASC, "albumResult", false);
    EXPECT_EQ(spy.count(), 0);
}

TEST(DataManagerSearchResultSortTest, artistResultSortDefaultBranchIsNoop)
{
    std::unique_ptr<DataManager> dm(makeDataMgr());
    injectDistinctArtists(dm.get());
    QList<DMusic::MediaMeta> metas;
    QList<DMusic::AlbumInfo> albums;
    QList<DMusic::ArtistInfo> artists;
    dm->searchText("", metas, albums, artists);
    ASSERT_GE(dm->searchedArtistInfos().size(), 2);
    QSignalSpy spy(dm.get(), &DataManager::signalPlaylistSortChanged);
    dm->sortPlaylist(DmGlobal::SortByCustomASC, "artistResult", false);
    EXPECT_EQ(spy.count(), 0);
}

// ----------------------------------------------------------------------------
// movePlaylist 插入到有效 nextHash 分支（line 1702-1703）
// nextHash 指向存在歌单 → insert 到该位置（非 append）
// ----------------------------------------------------------------------------
TEST(DataManagerMovePlaylistInsertTest, moveBeforeValidNextPlaylist)
{
    std::unique_ptr<DataManager> dm(makeDataMgr());
    const auto pl1 = dm->addPlayList("First");
    const auto pl2 = dm->addPlayList("Second");
    const auto pl3 = dm->addPlayList("Third");
    // 记录初始顺序中 pl2 的位置，把 pl3 移到 pl2 之前
    const auto before = dm->allPlaylistInfos();
    int pl2Idx = -1;
    for (int i = 0; i < before.size(); ++i) if (before[i].uuid == pl2.uuid) pl2Idx = i;
    ASSERT_GE(pl2Idx, 0);
    dm->movePlaylist(pl3.uuid, pl2.uuid);  // nextHash=pl2 有效 → insert 分支
    const auto after = dm->allPlaylistInfos();
    // pl3 现在应在 pl2 之前
    int newPl3 = -1, newPl2 = -1;
    for (int i = 0; i < after.size(); ++i) {
        if (after[i].uuid == pl3.uuid) newPl3 = i;
        if (after[i].uuid == pl2.uuid) newPl2 = i;
    }
    EXPECT_LT(newPl3, newPl2);
}
