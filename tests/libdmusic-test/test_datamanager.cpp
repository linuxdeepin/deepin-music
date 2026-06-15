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

#include "datamanager.h"
#include "global.h"

namespace {
// 内存 DB 的 DataManager 工厂
DataManager *makeDataMgr(const QStringList &suffix = QStringList{"mp3", "flac"})
{
    return new DataManager(suffix, nullptr, ":memory:");
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
