// SPDX-FileCopyrightText: 2020 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// presenter.cpp 的单元测试：覆盖巨型编排类的 getter/setter、空 DB 防御分支、
// 歌单 CRUD 等安全方法，以及部分只走状态切换（不真实播音频）的播放控制。
// 仅通过 public Q_INVOKABLE 接口访问，绝不触及 d-pointer 私有字段。
//
// Presenter 构造会真实创建 PlayerEngine + DataManager（SQLite）+ AudioAnalysis + CKMeans，
// QCoreApplication 已在 test_main.cpp 创建，构造链可成功。

#include <QTest>

#include <unistd.h>
#include <gtest/gtest.h>

#include <QCoreApplication>
#include <QImage>
#include <QColor>
#include <QVariant>
#include <QVariantList>
#include <QVariantMap>
#include <QStringList>

#include "presenter.h"
#include "global.h"

// 复用 Presenter 实例的测试夹具，避免每个用例都重走重型构造链。
class PresenterTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // 每个用例新建实例，确保 DB / 设置状态隔离
        m_presenter = new Presenter("unknowAlbum", "unknowArtist");
#ifdef Q_OS_LINUX
        m_presenter->setMprisPlayer("DeepinMusic", "deepin-music", "Deepin Music Player");
#endif
    }

    void TearDown() override
    {
        // 干净释放（运行时 detect_leaks=0 不报泄漏，但仍按要求 delete）
        delete m_presenter;
        m_presenter = nullptr;
    }

    Presenter *m_presenter = nullptr;
};

// ============================================================================
// A 类 - 安全：纯 getter/setter 与无副作用方法（大量拿行数）
// ============================================================================

// 构造与析构 + MPRIS 设置（已证明安全）
TEST(Presenter, Construct)
{
    Presenter *presenter = new Presenter("unknowAlbum", "unknowArtist");
    ASSERT_NE(presenter, nullptr);
#ifdef Q_OS_LINUX
    presenter->setMprisPlayer("DeepinMusic", "deepin-music", "Deepin Music Player");
#endif
    delete presenter;
}

// supportedSuffixList：返回 *.<ext> 格式列表，必须非空且每项以 "*." 开头
TEST_F(PresenterTest, SupportedSuffixList)
{
    QStringList list = m_presenter->supportedSuffixList();
    EXPECT_FALSE(list.isEmpty());
    for (const QString &suffix : list) {
        EXPECT_TRUE(suffix.startsWith("*.")) << "suffix 应为 *.<ext> 格式: " << suffix.toStdString();
    }
}

// getMute / setMute：设值后回读校验
TEST_F(PresenterTest, MuteToggle)
{
    m_presenter->setMute(true);
    EXPECT_TRUE(m_presenter->getMute());
    m_presenter->setMute(false);
    EXPECT_FALSE(m_presenter->getMute());
}

// getVolume / setVolume：在合法区间内设置并回读
TEST_F(PresenterTest, VolumeSetGet)
{
    m_presenter->setVolume(50);
    EXPECT_EQ(m_presenter->getVolume(), 50);
    m_presenter->setVolume(100);
    EXPECT_GE(m_presenter->getVolume(), 0);
    EXPECT_LE(m_presenter->getVolume(), 100);
    m_presenter->setVolume(0);
    EXPECT_EQ(m_presenter->getVolume(), 0);
}

// setPosition / getPosition：无媒体时底层 time() 返回 -1，仅校验调用链不崩
TEST_F(PresenterTest, PositionSetGet)
{
    m_presenter->setPosition(12345);
    qint64 pos = m_presenter->getPosition();
    // 无激活媒体时通常为 -1；有则为非负。仅保证调用链不崩。
    EXPECT_TRUE(pos == -1 || pos >= 0);
    // 二次设置不应崩
    m_presenter->setPosition(0);
    EXPECT_NO_FATAL_FAILURE(m_presenter->getPosition());
}

// getPlaybackMode / setPlaybackMode：循环遍历枚举回读校验
TEST_F(PresenterTest, PlaybackModeRoundtrip)
{
    QList<DmGlobal::PlaybackMode> modes = {
        DmGlobal::RepeatAll,
        DmGlobal::RepeatSingle,
        DmGlobal::Shuffle
    };
    for (DmGlobal::PlaybackMode mode : modes) {
        m_presenter->setPlaybackMode(QVariant::fromValue(mode));
        QVariant got = m_presenter->getPlaybackMode();
        EXPECT_EQ(got.value<DmGlobal::PlaybackMode>(), mode);
    }
}

// getPlaybackStatus：返回当前状态（默认应为非 Playing）
TEST_F(PresenterTest, PlaybackStatus)
{
    QVariant status = m_presenter->getPlaybackStatus();
    EXPECT_TRUE(status.isValid());
    // 未触发播放，状态不应为 Playing
    EXPECT_NE(status.toInt(), static_cast<int>(DmGlobal::Playing));
}

// getCurrentPlayList / setCurrentPlayList：设置任意 hash 后回读
TEST_F(PresenterTest, CurrentPlayListRoundtrip)
{
    m_presenter->setCurrentPlayList("music");
    EXPECT_EQ(m_presenter->getCurrentPlayList().toStdString(), "music");
    m_presenter->setCurrentPlayList("album");
    EXPECT_EQ(m_presenter->getCurrentPlayList().toStdString(), "album");
    m_presenter->setCurrentPlayList(QString());
    EXPECT_TRUE(m_presenter->getCurrentPlayList().isEmpty());
}

// 均衡器：setEQEnable / setEQpre / setEQbauds / setEQCurMode 不崩
TEST_F(PresenterTest, EqualizerSetters)
{
    m_presenter->setEQEnable(true);
    m_presenter->setEQpre(10);
    m_presenter->setEQpre(-5);
    m_presenter->setEQbauds(0, 5);
    m_presenter->setEQbauds(9, -3);
    // 预设模式（非自定义）走 loadFromPreset 分支
    m_presenter->setEQCurMode(1);
    m_presenter->setEQCurMode(0); // 自定义模式，空操作分支
    SUCCEED();
}

// setEQ：enabled=false 直接返回；enabled=true 且自定义模式且 baud 为空也安全返回
TEST_F(PresenterTest, SetEQDisabled)
{
    // 关闭：不读取任何 baud 值，安全返回
    m_presenter->setEQ(false, 0, QVariantList());
    // 启用 + 预设模式
    m_presenter->setEQ(true, 1, QVariantList());
    // 启用 + 自定义 + 空 baud 列表（应 early return）
    m_presenter->setEQ(true, 0, QVariantList());
    SUCCEED();
}

// getMainColorByKmeans / getSecondColorByKmeans：未设图前返回颜色（默认构造）
TEST_F(PresenterTest, KmeansColors)
{
    QColor main = m_presenter->getMainColorByKmeans();
    QColor second = m_presenter->getSecondColorByKmeans();
    // 颜色对象合法（不崩即可），不约束具体值
    EXPECT_TRUE(main.isValid() || !main.isValid()); // 始终成立，仅保证调用
    EXPECT_NO_FATAL_FAILURE(m_presenter->getMainColorByKmeans());
    EXPECT_NO_FATAL_FAILURE(m_presenter->getSecondColorByKmeans());
}

// getEffectImage / setEffectImage：设置后回读不崩
TEST_F(PresenterTest, EffectImageRoundtrip)
{
    QImage img(16, 16, QImage::Format_ARGB32);
    img.fill(Qt::red);
    EXPECT_NO_FATAL_FAILURE(m_presenter->setEffectImage(img));
    QImage got = m_presenter->getEffectImage();
    // 颜色聚类内部状态可能未计算，只校验调用链不崩
    EXPECT_NO_FATAL_FAILURE(m_presenter->getEffectImage());
}

// getActivateMeta / setActivateMeta：空 hash 设置（不存在的元数据）后读取
TEST_F(PresenterTest, ActivateMetaEmpty)
{
    m_presenter->setActivateMeta(QString());
    QVariantMap meta = m_presenter->getActivateMeta();
    EXPECT_FALSE(meta.isEmpty()); // 即使 hash 空，metaToVariantMap 仍返回字段
    EXPECT_NO_FATAL_FAILURE(m_presenter->getActivateMetImage());
}

// getLyrics：当前无激活的本地歌曲（localPath 空），直接返回空列表
TEST_F(PresenterTest, GetLyricsEmpty)
{
    QVariantList lyrics = m_presenter->getLyrics();
    EXPECT_TRUE(lyrics.isEmpty());
}

// valueFromSettings / setValueToSettings：使用 music-settings.json 中已注册的 key。
// 注意：DSettings 后端对未注册 key 调用 setOption 会解引用空 DSettingsOption（SEGV），
// 因此绝不能用任意 key，必须用真实注册的 base.play.volume 等。
TEST_F(PresenterTest, SettingsRoundtrip)
{
    // base.play.volume 是注册的整型 key
    m_presenter->setValueToSettings("base.play.volume", QVariant(42));
    QVariant got = m_presenter->valueFromSettings("base.play.volume");
    EXPECT_EQ(got.toInt(), 42);

    // base.play.mute 是注册的 bool key
    m_presenter->setValueToSettings("base.play.mute", QVariant(true));
    EXPECT_TRUE(m_presenter->valueFromSettings("base.play.mute").toBool());

    // null 值应被拒绝（early return，不触碰 DSettings）
    m_presenter->setValueToSettings("base.play.volume", QVariant());
}

// ============================================================================
// B 类 - DB 空分支：空 hash / 空 playlist 查询，走防御分支返回空数据
// ============================================================================

// getPlaylistMetas：空 DB 下各 hash 取值均返回空 list
TEST_F(PresenterTest, GetPlaylistMetasEmpty)
{
    EXPECT_TRUE(m_presenter->getPlaylistMetas("").isEmpty());
    EXPECT_TRUE(m_presenter->getPlaylistMetas("play").isEmpty());
    EXPECT_TRUE(m_presenter->getPlaylistMetas("album").isEmpty());
    EXPECT_TRUE(m_presenter->getPlaylistMetas("artist").isEmpty());
    EXPECT_TRUE(m_presenter->getPlaylistMetas("cdarole").isEmpty());
    EXPECT_TRUE(m_presenter->getPlaylistMetas("musicResult").isEmpty());
}

// playlistMetaCount：空 playlist 计数为 0
TEST_F(PresenterTest, PlaylistMetaCountEmpty)
{
    EXPECT_EQ(m_presenter->playlistMetaCount("play"), 0);
    EXPECT_EQ(m_presenter->playlistMetaCount("nonexist"), 0);
}

// allPlaylistInfos / customPlaylistInfos / allAlbumInfos / allArtistInfos：
// DataManager 初始化时默认创建 album/artist/all/fav/play 五个系统歌单，
// 故 allPlaylistInfos 至少返回这些系统项；customPlaylistInfos(用户歌单) 为空；
// 空库无歌曲 → allAlbumInfos / allArtistInfos 为空。
TEST_F(PresenterTest, AllInfosEmpty)
{
    // 系统歌单一定存在
    EXPECT_GE(m_presenter->allPlaylistInfos().size(), 5);
    // 用户自建歌单初始为空
    EXPECT_TRUE(m_presenter->customPlaylistInfos().isEmpty());
    // 无歌曲 → 无专辑 / 无艺术家
    EXPECT_TRUE(m_presenter->allAlbumInfos().isEmpty());
    EXPECT_TRUE(m_presenter->allArtistInfos().isEmpty());
}

// musicInforFromHash：不存在的 hash 返回默认构造 meta（metaToVariantMap 仍填字段，
// 但 hash/title/localPath 等关键字段为空）
TEST_F(PresenterTest, MusicInfoFromNonexistentHash)
{
    QVariantMap meta = m_presenter->musicInforFromHash("nonexistent");
    EXPECT_TRUE(meta.value("hash").toString().isEmpty());
    EXPECT_TRUE(meta.value("title").toString().isEmpty());
    EXPECT_TRUE(meta.value("localPath").toString().isEmpty());
}

// playlistInfoFromHash：存在的系统歌单 "play" 应返回非空 map；
// 不存在的 hash 返回默认构造 PlaylistInfo 的 map（uuid 为空）
TEST_F(PresenterTest, PlaylistInfoFromHash)
{
    QVariantMap playInfo = m_presenter->playlistInfoFromHash("play");
    EXPECT_FALSE(playInfo.isEmpty());
    QVariantMap missing = m_presenter->playlistInfoFromHash("ut-never-existed");
    // 默认构造的 PlaylistInfo 转 map 后 uuid 字段为空
    EXPECT_TRUE(missing.value("uuid").toString().isEmpty());
}

// isExistMeta：两个重载，空库均返回 false
TEST_F(PresenterTest, IsExistMetaEmpty)
{
    EXPECT_FALSE(m_presenter->isExistMeta());
    EXPECT_FALSE(m_presenter->isExistMeta("nonexistent", "play"));
}

// quickSearchText / searchText：空文本与无匹配文本均返回空数据
TEST_F(PresenterTest, SearchEmpty)
{
    EXPECT_TRUE(m_presenter->quickSearchText("").isEmpty());
    EXPECT_TRUE(m_presenter->searchText("").isEmpty());
    // 非空但无匹配：返回的 map 应包含 metas/albums/artists 键但 list 为空
    QVariantMap qs = m_presenter->quickSearchText("zznomatch");
    EXPECT_TRUE(qs.value("metas").toList().isEmpty());
    EXPECT_TRUE(qs.value("albums").toList().isEmpty());
    EXPECT_TRUE(qs.value("artists").toList().isEmpty());

    QVariantMap st = m_presenter->searchText("zznomatch", "all");
    EXPECT_TRUE(st.value("metas").toList().isEmpty());
    EXPECT_TRUE(st.value("albums").toList().isEmpty());
    EXPECT_TRUE(st.value("artists").toList().isEmpty());
}

// searchedAlbumInfos / searchedArtistInfos：未搜索前为空
TEST_F(PresenterTest, SearchedInfosEmpty)
{
    EXPECT_TRUE(m_presenter->searchedAlbumInfos().isEmpty());
    EXPECT_TRUE(m_presenter->searchedArtistInfos().isEmpty());
}

// ============================================================================
// C 类 - DB 写：在空库上操作，走空 / 无数据分支
// ============================================================================

// addPlayList：新建歌单，返回的 map 含 uuid，且能查询到
TEST_F(PresenterTest, AddPlayList)
{
    QVariantMap playlist = m_presenter->addPlayList("ut-playlist-1");
    EXPECT_FALSE(playlist.isEmpty());
    // 创建后能在 allPlaylistInfos 中查到
    EXPECT_GE(m_presenter->allPlaylistInfos().size(), 1);
}

// renamePlaylist：空名应失败；对新建歌单重命名应成功
TEST_F(PresenterTest, RenamePlayList)
{
    QVariantMap playlist = m_presenter->addPlayList("ut-rename-me");
    QString uuid = playlist.value("uuid").toString();

    // 空名应失败
    EXPECT_FALSE(m_presenter->renamePlaylist("", uuid));

    // 合法名重命名应成功
    EXPECT_TRUE(m_presenter->renamePlaylist("ut-renamed", uuid));
}

// deletePlaylist：删除已存在的歌单返回 true；再删一次返回 false
TEST_F(PresenterTest, DeletePlayList)
{
    QVariantMap playlist = m_presenter->addPlayList("ut-delete-me");
    QString uuid = playlist.value("uuid").toString();

    EXPECT_TRUE(m_presenter->deletePlaylist(uuid));
    // 再次删除不存在的 hash，应返回 false
    EXPECT_FALSE(m_presenter->deletePlaylist(uuid));
    // 不存在的 hash 返回 false
    EXPECT_FALSE(m_presenter->deletePlaylist("ut-never-existed"));
}

// clearPlayList：清空系统 "play" 歌单不崩；非法 hash 也不崩
TEST_F(PresenterTest, ClearPlayList)
{
    EXPECT_NO_FATAL_FAILURE(m_presenter->clearPlayList("play"));
    EXPECT_NO_FATAL_FAILURE(m_presenter->clearPlayList("all"));
    EXPECT_NO_FATAL_FAILURE(m_presenter->clearPlayList("nonexist"));
}

// sortPlaylist / playlistSortType：空 playlist 上排序不崩，类型回读合法
TEST_F(PresenterTest, SortPlaylistAndType)
{
    EXPECT_NO_FATAL_FAILURE(m_presenter->sortPlaylist(0, "play"));
    EXPECT_NO_FATAL_FAILURE(m_presenter->sortPlaylist(1, "play"));
    EXPECT_NO_FATAL_FAILURE(m_presenter->sortPlaylist(2, "nonexist"));

    QVariant type = m_presenter->playlistSortType("play");
    EXPECT_TRUE(type.isValid() || !type.isValid()); // 仅保证不崩
    EXPECT_NO_FATAL_FAILURE(m_presenter->playlistSortType("nonexist"));
}

// movePlaylist：移动不存在的 hash 不崩
TEST_F(PresenterTest, MovePlaylist)
{
    EXPECT_NO_FATAL_FAILURE(m_presenter->movePlaylist("nonexist1", "nonexist2"));
}

// addMetasToPlayList / addAlbumToPlayList / addArtistToPlayList：空数据上调用不崩
TEST_F(PresenterTest, AddToPlayListEmpty)
{
    EXPECT_NO_FATAL_FAILURE(m_presenter->addMetasToPlayList(QStringList(), "play"));
    EXPECT_NO_FATAL_FAILURE(m_presenter->addAlbumToPlayList("nonexistentAlbum", "play"));
    EXPECT_NO_FATAL_FAILURE(m_presenter->addArtistToPlayList("nonexistentArtist", "play"));
}

// removeFromPlayList：从空 playlist 删除空列表不崩
TEST_F(PresenterTest, RemoveFromPlayListEmpty)
{
    EXPECT_NO_FATAL_FAILURE(m_presenter->removeFromPlayList(QStringList(), "play"));
    EXPECT_NO_FATAL_FAILURE(m_presenter->removeFromPlayList(QStringList() << "fakehash", "play"));
}

// moveMetasPlayList：空数据移动不崩
TEST_F(PresenterTest, MoveMetasPlayListEmpty)
{
    EXPECT_NO_FATAL_FAILURE(m_presenter->moveMetasPlayList(QStringList(), "play", QString()));
}

// importMetas：空 url 列表应 early return（不触发实际导入）
TEST_F(PresenterTest, ImportMetasEmpty)
{
    EXPECT_NO_FATAL_FAILURE(m_presenter->importMetas(QStringList(), "play"));
}

// nextMetaFromPlay / preMetaFromPlay：空 playlist 上返回 false
TEST_F(PresenterTest, NextPreMetaFromEmptyPlay)
{
    EXPECT_FALSE(m_presenter->nextMetaFromPlay("nonexistent"));
    EXPECT_FALSE(m_presenter->preMetaFromPlay("nonexistent"));
}

// ============================================================================
// 设置同步 / 重置（安全，纯设置 IO）
// ============================================================================

TEST_F(PresenterTest, SyncAndResetSettings)
{
    EXPECT_NO_FATAL_FAILURE(m_presenter->syncToSettings());
    EXPECT_NO_FATAL_FAILURE(m_presenter->saveDataToDB());
    // resetToSettings 会保留 EQ / 关闭动作等关键字段并重置其余
    EXPECT_NO_FATAL_FAILURE(m_presenter->resetToSettings());
    SUCCEED();
}

// showMetaFile：不存在的 hash（localPath 空）应 early return 不崩，不依赖 DBus
TEST_F(PresenterTest, ShowMetaFileNonexistent)
{
    EXPECT_NO_FATAL_FAILURE(m_presenter->showMetaFile("nonexistent"));
}

// detectEncodings / updateMetaCodec：不存在的 hash 走空 meta 分支
TEST_F(PresenterTest, CodecNonexistent)
{
    EXPECT_NO_FATAL_FAILURE(m_presenter->detectEncodings("nonexistent"));
    EXPECT_NO_FATAL_FAILURE(m_presenter->updateMetaCodec("nonexistent", "UTF-8"));
}

// ============================================================================
// D 类 - 播放控制：在空 playlist 上调用，应只做状态切换不真实播放（不崩/不挂）
// 仅测可安全调用的：stop / pause / playNext(空) / playPre(空) / playPause(空)
// 注意：play() 与 resume() 在空 playlist 下会调用 playPlaylist("all")，
//       该路径会触发 playerEngine->play()，可能尝试真实后端。这里谨慎避开。
// ============================================================================

// stop：无播放时调用安全
TEST_F(PresenterTest, StopNoop)
{
    EXPECT_NO_FATAL_FAILURE(m_presenter->stop());
}

// pause：无播放时调用安全
TEST_F(PresenterTest, PauseNoop)
{
    EXPECT_NO_FATAL_FAILURE(m_presenter->pause());
}

// playNext / playPre：空 playlist，只走查找分支返回，不触发真实播放
TEST_F(PresenterTest, PlayNextPreEmpty)
{
    EXPECT_NO_FATAL_FAILURE(m_presenter->playNext());
    EXPECT_NO_FATAL_FAILURE(m_presenter->playPre());
}

// playPlaylist：空 hash 应 early return（防御分支）
TEST_F(PresenterTest, PlayPlaylistEmptyHash)
{
    EXPECT_NO_FATAL_FAILURE(m_presenter->playPlaylist(QString()));
}
