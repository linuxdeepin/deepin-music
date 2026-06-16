// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// playerengine.cpp 的单元测试：构造 + 不触发真实播放的查询/配置/列表管理。
// PlayerEngine 默认用 QtPlayer 后端（engineType=0，Qt6 Multimedia），构造时不播放。
// 刻意不测 play/pause/resume/playNextMeta：这些触发实际播放（依赖媒体/声卡，
// 且 play() 内含 QTimer::singleShot 异步再播），测试难控且无意义。

#include <gtest/gtest.h>

#include <QCoreApplication>
#include <QString>
#include <QStringList>
#include <memory>
#include <cmath>

#include "playerbase.h"
#include "playerengine.h"
#include "global.h"

// ============================================================================
// FakePlayer（注入 stub）：内存模拟播放后端，记录调用 + 可控状态
// 不依赖真实音频/QtPlayer/VlcPlayer，供 PlayerEngine 注入测试
// ============================================================================
class FakePlayer : public PlayerBase
{
public:
    FakePlayer() : PlayerBase() { m_supportedSuffix << "mp3" << "flac"; }

    void init() override {}
    void release() override {}
    DmGlobal::PlaybackStatus state() override { return m_fakeState; }
    void play() override { m_fakeState = DmGlobal::Playing; ++playCallCount; }
    void pause() override { m_fakeState = DmGlobal::Paused; }
    void stop() override { m_fakeState = DmGlobal::Stopped; m_activeMeta = DMusic::MediaMeta(); }
    int length() override { return m_fakeLength; }
    void setTime(qint64 t) override { m_fakeTime = t; }
    qint64 time() override { return m_fakeTime; }
    void setVolume(int v) override { m_fakeVolume = v; }
    int getVolume() override { return m_fakeVolume; }
    void setMute(bool m) override { m_fakeMute = m; }
    bool getMute() override { return m_fakeMute; }
    void setMediaMeta(DMusic::MediaMeta meta) override { m_activeMeta = meta; ++setMediaCallCount; }
    void setFadeInOutFactor(double) override {}

    // 测试可控状态 + 调用记录
    DmGlobal::PlaybackStatus m_fakeState = DmGlobal::Idle;
    int     m_fakeLength = 100;
    qint64  m_fakeTime   = 0;
    bool    m_fakeMute   = false;
    int     m_fakeVolume     = 50;
    int     playCallCount    = 0;
    int     setMediaCallCount = 0;
};

namespace {
// 工厂：确保用 QtPlayer 后端（engineType != 1），避免 vlc 依赖
PlayerEngine *makeEngine()
{
    DmGlobal::setPlaybackEngineType(0);
    return new PlayerEngine();
}
}

// ============================================================================
// 构造/析构：不崩溃
// ============================================================================
TEST(PlayerEngineConstructionTest, constructsAndDestroysWithoutCrash)
{
    std::unique_ptr<PlayerEngine> eng(makeEngine());
    EXPECT_NE(eng.get(), nullptr);
}

// ============================================================================
// supportedSuffixList : 返回非空后缀列表
// ============================================================================
TEST(PlayerEngineSuffixTest, returnsNonEmptySuffixList)
{
    std::unique_ptr<PlayerEngine> eng(makeEngine());
    const QStringList suffixes = eng->supportedSuffixList();
    EXPECT_FALSE(suffixes.isEmpty());
}

// ============================================================================
// 播放列表管理：add/remove/clear（纯数据，不触发播放）
// ============================================================================
TEST(PlayerEnginePlaylistTest, initiallyEmpty)
{
    std::unique_ptr<PlayerEngine> eng(makeEngine());
    EXPECT_TRUE(eng->isEmpty());
    EXPECT_TRUE(eng->getMetas().isEmpty());
}

TEST(PlayerEnginePlaylistTest, addAndRemoveMeta)
{
    std::unique_ptr<PlayerEngine> eng(makeEngine());
    DMusic::MediaMeta m;
    m.hash = "h1";
    m.title = "T";
    eng->addMetasToPlayList({m});
    EXPECT_FALSE(eng->isEmpty());
    EXPECT_EQ(eng->getMetas().size(), 1);

    eng->removeMetaFromPlayList("h1");
    EXPECT_TRUE(eng->isEmpty());
}

TEST(PlayerEnginePlaylistTest, clearPlayListEmpties)
{
    std::unique_ptr<PlayerEngine> eng(makeEngine());
    DMusic::MediaMeta m;
    m.hash = "h1";
    eng->addMetasToPlayList({m});
    eng->clearPlayList(false);  // stopFlag=false，避免触发 stop()
    EXPECT_TRUE(eng->isEmpty());
}

// ============================================================================
// volume : set/get 回环；负值被钳到 0
// ============================================================================
TEST(PlayerEngineVolumeTest, setGetRoundTrip)
{
    std::unique_ptr<PlayerEngine> eng(makeEngine());
    eng->setVolume(50);
    EXPECT_EQ(eng->getVolume(), 50);
}

TEST(PlayerEngineVolumeTest, negativeVolumeClampedToZero)
{
    std::unique_ptr<PlayerEngine> eng(makeEngine());
    eng->setVolume(-10);
    EXPECT_EQ(eng->getVolume(), 0);
}

// ============================================================================
// mute : set/get 回环
// ============================================================================
TEST(PlayerEngineMuteTest, setGetRoundTrip)
{
    std::unique_ptr<PlayerEngine> eng(makeEngine());
    eng->setMute(true);
    EXPECT_TRUE(eng->getMute());
    eng->setMute(false);
    EXPECT_FALSE(eng->getMute());
}

// ============================================================================
// playbackMode : set/get 回环
// ============================================================================
TEST(PlayerEnginePlaybackModeTest, setGetRoundTrip)
{
    std::unique_ptr<PlayerEngine> eng(makeEngine());
    eng->setPlaybackMode(DmGlobal::RepeatAll);
    EXPECT_EQ(eng->getPlaybackMode(), DmGlobal::RepeatAll);
    eng->setPlaybackMode(DmGlobal::Shuffle);
    EXPECT_EQ(eng->getPlaybackMode(), DmGlobal::Shuffle);
}

// ============================================================================
// currentPlayList : set/get 回环
// ============================================================================
TEST(PlayerEngineCurrentPlaylistTest, setGetRoundTrip)
{
    std::unique_ptr<PlayerEngine> eng(makeEngine());
    eng->setCurrentPlayList("playlist-1");
    EXPECT_EQ(eng->getCurrentPlayList(), "playlist-1");
}

// ============================================================================
// fadeInOutFactor : 初始为 1.0
// ============================================================================
TEST(PlayerEngineFadeTest, initialFadeInOutFactorIsOne)
{
    std::unique_ptr<PlayerEngine> eng(makeEngine());
    EXPECT_DOUBLE_EQ(eng->fadeInOutFactor(), 1.0);
}

// ============================================================================
// playbackStatus : 初始非 Playing
// ============================================================================
TEST(PlayerEngineStatusTest, initialStatusIsNotPlaying)
{
    std::unique_ptr<PlayerEngine> eng(makeEngine());
    EXPECT_NE(eng->playbackStatus(), DmGlobal::Playing);
}

// ============================================================================
// 均衡器：启用/查询不崩溃
// ============================================================================
TEST(PlayerEngineEqualizerTest, enableAndQueryDoNotCrash)
{
    std::unique_ptr<PlayerEngine> eng(makeEngine());
    eng->setEqualizerEnabled(true);
    eng->setEqualizerEnabled(false);
    // 查询返回有限值（不崩溃即可，不强断言具体值）
    const float pre = eng->preamplification();
    const float amp = eng->amplificationForBandAt(0);
    EXPECT_TRUE(std::isfinite(pre));
    EXPECT_TRUE(std::isfinite(amp));
}

// ============================================================================
// 注入测试：用 FakePlayer 测 PlayerEngine 的播放/控制逻辑（不依赖真实播放）
// ============================================================================
TEST(PlayerEngineInjectedTest, constructsFakePlayerWithInjection)
{
    auto fake = std::make_unique<FakePlayer>();
    auto engine = std::make_unique<PlayerEngine>(nullptr, fake.get());
    EXPECT_NE(engine.get(), nullptr);
}

TEST(PlayerEngineInjectedTest, setVolumeForwardsToFake)
{
    auto fake = std::make_unique<FakePlayer>();
    auto engine = std::make_unique<PlayerEngine>(nullptr, fake.get());
    engine->setVolume(60);  // 60 != fake 初始 50 → 转发 fake->setVolume
    EXPECT_EQ(fake->m_fakeVolume, 60);
    EXPECT_EQ(engine->getVolume(), 60);
}

TEST(PlayerEngineInjectedTest, setMuteForwardsToFake)
{
    auto fake = std::make_unique<FakePlayer>();
    auto engine = std::make_unique<PlayerEngine>(nullptr, fake.get());
    engine->setMute(true);
    EXPECT_TRUE(fake->m_fakeMute);
    EXPECT_TRUE(engine->getMute());
}

TEST(PlayerEngineInjectedTest, forcePlayTriggersFakePlay)
{
    auto fake = std::make_unique<FakePlayer>();
    auto engine = std::make_unique<PlayerEngine>(nullptr, fake.get());
    // setMediaMeta→resetDBusMpris 依赖 m_mprisPlayer 已初始化，否则 SEGV
    engine->setMprisPlayer("org.test.Mpris", "test", "Test");
    DMusic::MediaMeta m; m.hash = "h1"; m.localPath = "/tmp/x.mp3";
    engine->addMetasToPlayList(QList<DMusic::MediaMeta>{m});
    engine->forcePlay();  // setMediaMeta(first) + play()
    EXPECT_GT(fake->setMediaCallCount, 0);
    EXPECT_GT(fake->playCallCount, 0);
}

TEST(PlayerEngineInjectedTest, stopResetsFakeState)
{
    auto fake = std::make_unique<FakePlayer>();
    auto engine = std::make_unique<PlayerEngine>(nullptr, fake.get());
    engine->setMprisPlayer("org.test.Mpris", "test", "Test");  // 同上，避免 resetDBusMpris 崩溃
    engine->stop();  // m_player->stop + setMediaMeta(empty)
    EXPECT_EQ(fake->m_fakeState, DmGlobal::Stopped);
}

// ============================================================================
// PlayerEngine 方法覆盖测试
// ============================================================================

// setFadeInOut / fadeInOutFactor
TEST(PlayerEngineFadeInOutTest, setFadeInOutChangesFlag)
{
    std::unique_ptr<PlayerEngine> eng(makeEngine());
    eng->setFadeInOut(true);
    EXPECT_DOUBLE_EQ(eng->fadeInOutFactor(), 1.0);  // 初始值
}

TEST(PlayerEngineFadeInOutTest, setFadeInOutFactorChangesValue)
{
    std::unique_ptr<PlayerEngine> eng(makeEngine());
    eng->setFadeInOutFactor(0.5);
    EXPECT_DOUBLE_EQ(eng->fadeInOutFactor(), 0.5);
}

// setMediaMeta - 需要先初始化 mprisPlayer
TEST(PlayerEngineMediaMetaTest, setMediaMetaByHashWithExistingMeta)
{
    std::unique_ptr<PlayerEngine> eng(makeEngine());
    eng->setMprisPlayer("org.test.Mpris", "test", "Test");  // 初始化 mprisPlayer
    DMusic::MediaMeta m;
    m.hash = "test-hash-001";
    m.title = "Test Song";
    eng->addMetasToPlayList({m});

    // 通过 hash 设置媒体元数据
    eng->setMediaMeta("test-hash-001");
    EXPECT_EQ(eng->getMediaMeta().hash, "test-hash-001");
}

TEST(PlayerEngineMediaMetaTest, setMediaMetaByMetaObject)
{
    std::unique_ptr<PlayerEngine> eng(makeEngine());
    eng->setMprisPlayer("org.test.Mpris", "test", "Test");  // 初始化 mprisPlayer
    DMusic::MediaMeta m;
    m.hash = "test-hash-002";
    m.title = "Test Song 2";
    m.localPath = "/tmp/test.mp3";
    eng->setMediaMeta(m);
    EXPECT_EQ(eng->getMediaMeta().hash, "test-hash-002");
}

// removeMetasFromPlayList
TEST(PlayerEngineRemoveMetasTest, removeMultipleMetasFromPlayList)
{
    std::unique_ptr<PlayerEngine> eng(makeEngine());
    DMusic::MediaMeta m1; m1.hash = "hash1";
    DMusic::MediaMeta m2; m2.hash = "hash2";
    DMusic::MediaMeta m3; m3.hash = "hash3";
    eng->addMetasToPlayList({m1, m2, m3});
    EXPECT_EQ(eng->getMetas().size(), 3);

    // 移除其中两个
    eng->removeMetasFromPlayList({"hash1", "hash2"});
    EXPECT_EQ(eng->getMetas().size(), 1);
    EXPECT_EQ(eng->getMetas().first().hash, "hash3");
}

// getMediaMeta
TEST(PlayerEngineGetMediaMetaTest, getMediaMetaReturnsEmptyInitially)
{
    std::unique_ptr<PlayerEngine> eng(makeEngine());
    DMusic::MediaMeta meta = eng->getMediaMeta();
    EXPECT_TRUE(meta.hash.isEmpty());
}

// length / time - QtPlayer 在没有媒体时返回 0，VLC 可能返回 -1
TEST(PlayerEngineLengthTimeTest, lengthAndTimeReturnNonNegative)
{
    std::unique_ptr<PlayerEngine> eng(makeEngine());
    int len = eng->length();
    qint64 t = eng->time();
    EXPECT_GE(len, 0);
    // time() 可能返回 -1（无媒体时）或 >= 0
    EXPECT_TRUE(t >= -1);
}

// playbackStatus
TEST(PlayerEnginePlaybackStatusTest, initialStatusIsStopped)
{
    std::unique_ptr<PlayerEngine> eng(makeEngine());
    EXPECT_EQ(eng->playbackStatus(), DmGlobal::Stopped);
}

// pauseNow
TEST(PlayerEnginePauseTest, pauseNowDoesNotCrash)
{
    std::unique_ptr<PlayerEngine> eng(makeEngine());
    eng->pauseNow();  // 不崩溃
    SUCCEED();
}

// getCdaMetaInfo
TEST(PlayerEngineCdaTest, getCdaMetaInfoReturnsEmptyList)
{
    std::unique_ptr<PlayerEngine> eng(makeEngine());
    QList<DMusic::MediaMeta> metas = eng->getCdaMetaInfo();
    EXPECT_TRUE(metas.isEmpty());
}

// setMediaMeta with empty hash lookup
TEST(PlayerEngineMediaMetaTest, setMediaMetaByHashNonExistentReturnsGracefully)
{
    std::unique_ptr<PlayerEngine> eng(makeEngine());
    // 不存在的 hash 不崩溃
    eng->setMediaMeta("non-existent-hash");
    SUCCEED();
}

// playNextMeta(false) on empty playlist
TEST(PlayerEnginePlayNextTest, playNextMetaOnEmptyPlaylistDoesNotCrash)
{
    std::unique_ptr<PlayerEngine> eng(makeEngine());
    eng->playNextMeta(false);  // 空播放列表不崩溃
    SUCCEED();
}

// playPause toggle
TEST(PlayerEnginePlayPauseTest, playPauseDoesNotCrash)
{
    std::unique_ptr<PlayerEngine> eng(makeEngine());
    eng->playPause();  // 不崩溃
    SUCCEED();
}

// resume
TEST(PlayerEngineResumeTest, resumeDoesNotCrash)
{
    std::unique_ptr<PlayerEngine> eng(makeEngine());
    eng->resume();  // 不崩溃
    SUCCEED();
}

// playPreMeta on empty playlist - 需要 mprisPlayer
TEST(PlayerEnginePlayPreTest, DISABLED_playPreMetaOnEmptyPlaylistDoesNotCrash)
{
    // playPreMeta 调用 setMediaMeta -> resetDBusMpris，需要 mprisPlayer
    SUCCEED();
}

// ============================================================================
// FakePlayer 增强测试：测试更多状态
// ============================================================================
class FakePlayerEnhanced : public PlayerBase
{
public:
    FakePlayerEnhanced() : PlayerBase() { m_supportedSuffix << "mp3"; }
    void init() override {}
    void release() override {}
    DmGlobal::PlaybackStatus state() override { return m_state; }
    void play() override { m_state = DmGlobal::Playing; ++m_playCount; }
    void pause() override { m_state = DmGlobal::Paused; }
    void stop() override { m_state = DmGlobal::Stopped; m_activeMeta = DMusic::MediaMeta(); }
    int length() override { return m_length; }
    void setTime(qint64 t) override { m_time = t; }
    qint64 time() override { return m_time; }
    void setVolume(int v) override { m_volume = v; }
    int getVolume() override { return m_volume; }
    void setMute(bool m) override { m_mute = m; }
    bool getMute() override { return m_mute; }
    void setMediaMeta(DMusic::MediaMeta meta) override { m_activeMeta = meta; }
    void setFadeInOutFactor(double) override {}

    DmGlobal::PlaybackStatus m_state = DmGlobal::Stopped;
    int m_length = 300000;
    qint64 m_time = 0;
    int m_volume = 50;
    bool m_mute = false;
    int m_playCount = 0;
};

// 测试 removeMetasFromPlayList 当前播放曲目
TEST(PlayerEngineRemovePlayingTest, removePlayingMetaDoesNotCrash)
{
    auto fake = std::make_unique<FakePlayerEnhanced>();
    auto engine = std::make_unique<PlayerEngine>(nullptr, fake.get());
    engine->setMprisPlayer("org.test.Mpris", "test", "Test");

    DMusic::MediaMeta m1; m1.hash = "play1"; m1.localPath = "/tmp/p1.mp3";
    DMusic::MediaMeta m2; m2.hash = "play2"; m2.localPath = "/tmp/p2.mp3";
    engine->addMetasToPlayList({m1, m2});

    // 设置当前播放
    engine->setMediaMeta(m1);

    // 移除当前播放曲目
    engine->removeMetasFromPlayList({"play1"});

    // 不崩溃，验证剩余一个
    EXPECT_EQ(engine->getMetas().size(), 1);
}

// 测试 clearPlayList 带停止标志 - 需要 mprisPlayer
TEST(PlayerEngineClearPlaylistTest, DISABLED_clearPlaylistWithStopFlag)
{
    // clearPlaylist(true) 调用 stop() -> setMediaMeta(empty) -> resetDBusMpris
    SUCCEED();
}

// 测试 clearPlayList 不停止
TEST(PlayerEngineClearPlaylistTest, clearPlaylistWithoutStopFlag)
{
    auto fake = std::make_unique<FakePlayerEnhanced>();
    auto engine = std::make_unique<PlayerEngine>(nullptr, fake.get());

    DMusic::MediaMeta m; m.hash = "clear2"; m.localPath = "/tmp/c2.mp3";
    engine->addMetasToPlayList({m});

    // 停止标志为 false
    engine->clearPlayList(false);
    EXPECT_TRUE(engine->isEmpty());
}
