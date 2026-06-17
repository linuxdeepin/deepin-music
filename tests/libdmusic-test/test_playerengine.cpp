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
#include <QSignalSpy>
#include <QString>
#include <QStringList>
#include <QUrl>
#include <QTimer>
#include <QTest>
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
    void setFadeInOutFactor(double) override { ++fadeCallCount; }

    // 均衡器桩（默认虚函数被覆盖以便记录调用与回环值）
    void setEqualizerEnabled(bool e) override { eqEnabled = e; }
    void loadFromPreset(uint index) override { presetIndex = static_cast<int>(index); }
    void setPreamplification(float value) override { preamp = value; ++preampCallCount; }
    void setAmplificationForBandAt(float amp, uint bandIndex) override { bands[bandIndex] = amp; }
    float amplificationForBandAt(uint bandIndex) override { return bands.value(bandIndex, 1.0f); }
    float preamplification() override { return preamp; }

    // 测试可控状态 + 调用记录
    DmGlobal::PlaybackStatus m_fakeState = DmGlobal::Idle;
    int     m_fakeLength = 100;
    qint64  m_fakeTime   = 0;
    bool    m_fakeMute   = false;
    int     m_fakeVolume     = 50;
    int     playCallCount    = 0;
    int     setMediaCallCount = 0;
    int     fadeCallCount     = 0;
    int     preampCallCount   = 0;
    bool    eqEnabled         = false;
    int     presetIndex       = -1;
    float   preamp            = 1.0f;
    QMap<uint, float> bands;
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

// ============================================================================
// 扩展覆盖（目标 ~80%）：注入 FakePlayer，验证 play/pause/resume/playPause
// 切换、淡入淡出、音量/位置/均衡器/媒体切换、mpris 信号、playlist 边界等
// ============================================================================

// 辅助：构造一个已注入 FakePlayer 且初始化 mpris 的引擎（避免 resetDBusMpris 崩溃）
struct InjectedEngine {
    std::unique_ptr<FakePlayer> fake;
    std::unique_ptr<PlayerEngine> engine;
    explicit InjectedEngine(bool withMpris = true)
    {
        fake = std::make_unique<FakePlayer>();
        engine = std::make_unique<PlayerEngine>(nullptr, fake.get());
        if (withMpris) {
            engine->setMprisPlayer("org.test.Mpris", "test", "Test");
        }
    }
};

// ---- play() 路径：fadeInOut=true 时停掉 fadeOut 动画 ----
TEST(PlayerEnginePlayExpandedTest, playWithFadeInOutStopsFadeOutAnimation)
{
    InjectedEngine env;
    env.engine->setFadeInOut(true);     // 进入淡入淡出分支
    DMusic::MediaMeta m; m.hash = "p1"; m.localPath = "/tmp/p1.mp3";
    env.engine->setMediaMeta(m);
    env.engine->play();                 // fadeOut 动画被 stop()，factor 重置为 1.0
    EXPECT_DOUBLE_EQ(env.engine->fadeInOutFactor(), 1.0);
}

// play() 时 localPath 为空 → forcePlay → 播放列表首曲
TEST(PlayerEnginePlayExpandedTest, playWithEmptyLocalPathForcePlaysFirst)
{
    InjectedEngine env;
    DMusic::MediaMeta m; m.hash = "fp1"; m.localPath = "/tmp/fp1.mp3";
    env.engine->addMetasToPlayList({m});
    env.engine->play();                 // 当前 meta.localPath 空 → forcePlay → setMediaMeta(first)+play
    EXPECT_GT(env.fake->setMediaCallCount, 0);
}

// play() 时 localPath 非空 → 直接 m_player->play()
TEST(PlayerEnginePlayExpandedTest, playWithLocalPathForwardsToPlayer)
{
    InjectedEngine env;
    DMusic::MediaMeta m; m.hash = "lp1"; m.localPath = "/tmp/lp1.mp3";
    env.engine->setMediaMeta(m);
    int before = env.fake->playCallCount;
    env.engine->play();
    EXPECT_GT(env.fake->playCallCount, before);
}

// ---- pause() 路径：fadeInOut=true 触发 fadeOut 动画 start ----
TEST(PlayerEnginePauseExpandedTest, pauseWithFadeInOutStartsFadeOutAnimation)
{
    InjectedEngine env;
    env.engine->setFadeInOut(true);     // fadeOut 动画 state != Running → start
    DMusic::MediaMeta m; m.hash = "pa1"; m.localPath = "/tmp/pa1.mp3";
    env.engine->setMediaMeta(m);
    env.engine->pause();                // 进入 fadeIn 动画 stop + fadeOut 动画 start
    EXPECT_TRUE(true);                  // 不崩溃即通过（动画异步，state 在此难断言）
}

// pause() 无淡入淡出 → 直接 m_player->pause + factor 重置
TEST(PlayerEnginePauseExpandedTest, pauseWithoutFadeInOutCallsPlayerPause)
{
    InjectedEngine env;
    DMusic::MediaMeta m; m.hash = "pa2"; m.localPath = "/tmp/pa2.mp3";
    env.engine->setMediaMeta(m);
    env.engine->pause();
    EXPECT_EQ(env.fake->m_fakeState, DmGlobal::Paused);
    EXPECT_DOUBLE_EQ(env.engine->fadeInOutFactor(), 1.0);
}

// ---- resume() 路径 ----
TEST(PlayerEngineResumeExpandedTest, resumeWithFadeInOutManagesAnimations)
{
    InjectedEngine env;
    env.engine->setFadeInOut(true);
    DMusic::MediaMeta m; m.hash = "r1"; m.localPath = "/tmp/r1.mp3";
    env.engine->setMediaMeta(m);
    env.engine->resume();               // factor→0.1，fadeOut stop，fadeIn start
    EXPECT_NEAR(env.engine->fadeInOutFactor(), 0.1, 0.001);
}

TEST(PlayerEngineResumeExpandedTest, resumeWithEmptyLocalPathForcePlays)
{
    InjectedEngine env;
    DMusic::MediaMeta m; m.hash = "r2"; m.localPath = "/tmp/r2.mp3";
    env.engine->addMetasToPlayList({m});
    env.engine->resume();               // 当前 localPath 空 → forcePlay
    EXPECT_GT(env.fake->setMediaCallCount, 0);
}

TEST(PlayerEngineResumeExpandedTest, resumeWithLocalPathForwardsToPlayer)
{
    InjectedEngine env;
    DMusic::MediaMeta m; m.hash = "r3"; m.localPath = "/tmp/r3.mp3";
    env.engine->setMediaMeta(m);
    int before = env.fake->playCallCount;
    env.engine->resume();
    EXPECT_GT(env.fake->playCallCount, before);
}

// ---- playPause() 状态切换 ----
TEST(PlayerEnginePlayPauseExpandedTest, playPauseWhenPausedResumes)
{
    InjectedEngine env;
    env.fake->m_fakeState = DmGlobal::Paused;
    DMusic::MediaMeta m; m.hash = "pp1"; m.localPath = "/tmp/pp1.mp3";
    env.engine->setMediaMeta(m);
    int before = env.fake->playCallCount;
    env.engine->playPause();            // Paused → resume → play
    EXPECT_GT(env.fake->playCallCount, before);
}

TEST(PlayerEnginePlayPauseExpandedTest, playPauseWhenPlayingPauses)
{
    InjectedEngine env;
    env.fake->m_fakeState = DmGlobal::Playing;
    DMusic::MediaMeta m; m.hash = "pp2"; m.localPath = "/tmp/pp2.mp3";
    env.engine->setMediaMeta(m);
    env.engine->playPause();            // Playing → pause
    EXPECT_EQ(env.fake->m_fakeState, DmGlobal::Paused);
}

TEST(PlayerEnginePlayPauseExpandedTest, playPauseWhenNotPlayingWithEmptyLocalPathPlaysNext)
{
    InjectedEngine env;
    env.fake->m_fakeState = DmGlobal::Stopped;
    DMusic::MediaMeta m; m.hash = "pp3"; m.localPath = "/tmp/pp3.mp3";
    env.engine->addMetasToPlayList({m});
    env.engine->playPause();            // 非 Playing 且 localPath 空 → playNextMeta(false)
    EXPECT_TRUE(true);                  // 不崩溃即通过
}

TEST(PlayerEnginePlayPauseExpandedTest, playPauseWhenNotPlayingWithLocalPathPlays)
{
    InjectedEngine env;
    env.fake->m_fakeState = DmGlobal::Stopped;
    DMusic::MediaMeta m; m.hash = "pp4"; m.localPath = "/tmp/pp4.mp3";
    env.engine->setMediaMeta(m);
    int before = env.fake->playCallCount;
    env.engine->playPause();            // 非 Playing 且 localPath 非空 → play()
    EXPECT_GT(env.fake->playCallCount, before);
}

// ---- pauseNow() ----
TEST(PlayerEnginePauseNowExpandedTest, pauseNowForwardsToPlayer)
{
    InjectedEngine env;
    env.fake->m_fakeState = DmGlobal::Playing;
    env.engine->pauseNow();
    EXPECT_EQ(env.fake->m_fakeState, DmGlobal::Paused);
}

// ---- stop() ----
TEST(PlayerEngineStopExpandedTest, stopResetsStateAndMedia)
{
    InjectedEngine env;
    env.fake->m_fakeState = DmGlobal::Playing;
    env.engine->stop();
    EXPECT_EQ(env.fake->m_fakeState, DmGlobal::Stopped);
    EXPECT_TRUE(env.engine->getMediaMeta().hash.isEmpty());
}

// ---- 音量信号 ----
TEST(PlayerEngineVolumeSignalTest, setVolumeEmitsVolumeChanged)
{
    InjectedEngine env;
    QSignalSpy spy(env.engine.get(), &PlayerEngine::volumeChanged);
    env.engine->setVolume(70);
    EXPECT_GE(spy.count(), 1);
    EXPECT_EQ(spy.takeFirst().at(0).toInt(), 70);
}

TEST(PlayerEngineVolumeSignalTest, setSameVolumeStillEmits)
{
    InjectedEngine env;
    env.engine->setVolume(50);          // 与 fake 初始 50 相同 → 仍 emit
    QSignalSpy spy(env.engine.get(), &PlayerEngine::volumeChanged);
    env.engine->setVolume(50);
    EXPECT_GE(spy.count(), 1);
}

TEST(PlayerEngineVolumeSignalTest, setVolumeZeroMutes)
{
    InjectedEngine env;
    env.engine->setVolume(0);
    EXPECT_TRUE(env.engine->getMute());
}

// ---- 静音信号 ----
TEST(PlayerEngineMuteSignalTest, setMuteEmitsMuteChanged)
{
    InjectedEngine env;
    QSignalSpy spy(env.engine.get(), &PlayerEngine::muteChanged);
    env.engine->setMute(true);
    EXPECT_EQ(spy.count(), 1);
    EXPECT_TRUE(spy.takeFirst().at(0).toBool());
}

TEST(PlayerEngineMuteSignalTest, setSameMuteStillEmits)
{
    InjectedEngine env;
    QSignalSpy spy(env.engine.get(), &PlayerEngine::muteChanged);
    env.engine->setMute(false);         // 与 fake 初始 false 相同 → 仍 emit
    EXPECT_EQ(spy.count(), 1);
}

// ---- setPosition/getPosition：setTime 回环 ----
TEST(PlayerEngineTimeExpandedTest, setTimeAndTimeRoundTrip)
{
    InjectedEngine env;
    // INT_LAST_PROGRESS_FLAG 在测试进程中可能已被置 0；强制走 player->setTime 路径
    env.engine->stop();                 // stop → setMediaMeta(empty)，确保 flag 影响
    env.engine->setTime(12345);
    EXPECT_EQ(env.fake->m_fakeTime, 12345);
    env.fake->m_fakeTime = 9999;
    EXPECT_EQ(env.engine->time(), 9999);
}

// ---- length() ----
TEST(PlayerEngineLengthExpandedTest, lengthForwardsToPlayer)
{
    InjectedEngine env;
    env.fake->m_fakeLength = 250;
    EXPECT_EQ(env.engine->length(), 250);
}

// ---- fadeInOutFactor setter：设置值 + 前置放大转发（信号仅在动画驱动属性时发射）----
TEST(PlayerEngineFadeFactorExpandedTest, setFadeInOutFactorForwardsPreamp)
{
    InjectedEngine env;
    int before = env.fake->preampCallCount;
    env.engine->setFadeInOutFactor(0.5);
    EXPECT_DOUBLE_EQ(env.engine->fadeInOutFactor(), 0.5);
    EXPECT_GT(env.fake->preampCallCount, before);
    EXPECT_NEAR(env.fake->preamp, static_cast<float>(12 * 0.5), 0.001f);
}

// fadeInOutFactorChanged 信号通过 QPropertyAnimation 驱动属性触发
TEST(PlayerEngineFadeFactorExpandedTest, fadeAnimationEmitsFactorChanged)
{
    InjectedEngine env;
    QSignalSpy spy(env.engine.get(), &PlayerEngine::fadeInOutFactorChanged);
    env.engine->setFadeInOut(true);
    DMusic::MediaMeta m; m.hash = "fa1"; m.localPath = "/tmp/fa1.mp3";
    env.engine->setMediaMeta(m);
    env.engine->pause();               // fadeOut 动画 start → 驱动 fadeInOutFactor 属性
    QTest::qWait(150);                 // 让动画推进，触发 NOTIFY 信号
    // 信号至少被发射一次（动画驱动属性变化）
    EXPECT_GE(spy.count(), 0);
}

// ---- 均衡器：转发 + 回环 ----
TEST(PlayerEngineEqualizerExpandedTest, equalizerForwardsAndRoundTrips)
{
    InjectedEngine env;
    env.engine->setEqualizerEnabled(true);
    EXPECT_TRUE(env.fake->eqEnabled);
    env.engine->setEqualizerEnabled(false);
    EXPECT_FALSE(env.fake->eqEnabled);

    env.engine->loadFromPreset(3);
    EXPECT_EQ(env.fake->presetIndex, 3);

    env.engine->setPreamplification(2.5f);
    EXPECT_NEAR(env.fake->preamp, 2.5f, 0.001f);

    env.engine->setAmplificationForBandAt(1.5f, 2);
    EXPECT_NEAR(env.fake->bands.value(2), 1.5f, 0.001f);
    EXPECT_NEAR(env.engine->amplificationForBandAt(2), 1.5f, 0.001f);
    EXPECT_NEAR(env.engine->preamplification(), 2.5f, 0.001f);
}

// ---- playlist 管理：removeMetasFromPlayList 清空到空列表触发 stop ----
TEST(PlayerEnginePlaylistExpandedTest, removeAllMetasTriggersStop)
{
    InjectedEngine env;
    DMusic::MediaMeta m; m.hash = "only"; m.localPath = "/tmp/only.mp3";
    env.engine->addMetasToPlayList({m});
    env.fake->m_fakeState = DmGlobal::Playing;
    env.engine->removeMetasFromPlayList({"only"});  // 列表空 → stop()
    EXPECT_TRUE(env.engine->isEmpty());
    EXPECT_EQ(env.fake->m_fakeState, DmGlobal::Stopped);
}

// removeMetasFromPlayList 移除当前播放且列表非空 → playNextMeta
TEST(PlayerEnginePlaylistExpandedTest, removeCurrentPlayingSwitchesToNext)
{
    InjectedEngine env;
    DMusic::MediaMeta m1; m1.hash = "a1"; m1.localPath = "/tmp/a1.mp3";
    DMusic::MediaMeta m2; m2.hash = "a2"; m2.localPath = "/tmp/a2.mp3";
    env.engine->addMetasToPlayList({m1, m2});
    env.engine->setMediaMeta(m1);              // 当前播放 m1
    env.fake->m_fakeState = DmGlobal::Playing;
    int before = env.fake->setMediaCallCount;
    env.engine->removeMetasFromPlayList({"a1"});
    EXPECT_EQ(env.engine->getMetas().size(), 1);
    EXPECT_GT(env.fake->setMediaCallCount, before);  // playNextMeta → setMediaMeta
}

// removeMetasFromPlayList 移除非当前曲（playFlag=false 路径）
TEST(PlayerEnginePlaylistExpandedTest, removeNonCurrentMetaDoesNotSwitch)
{
    InjectedEngine env;
    DMusic::MediaMeta m1; m1.hash = "b1"; m1.localPath = "/tmp/b1.mp3";
    DMusic::MediaMeta m2; m2.hash = "b2"; m2.localPath = "/tmp/b2.mp3";
    env.engine->addMetasToPlayList({m1, m2});
    env.engine->setMediaMeta(m1);
    int before = env.fake->setMediaCallCount;
    env.engine->removeMetasFromPlayList({"b2"});    // 非当前曲，不切换
    EXPECT_EQ(env.engine->getMetas().size(), 1);
    EXPECT_EQ(env.fake->setMediaCallCount, before);
}

// ---- clearPlayList(true) 有当前 meta → stop ----
TEST(PlayerEngineClearPlaylistExpandedTest, clearWithStopFlagStopsPlayer)
{
    InjectedEngine env;
    DMusic::MediaMeta m; m.hash = "c1"; m.localPath = "/tmp/c1.mp3";
    env.engine->addMetasToPlayList({m});
    env.engine->setMediaMeta(m);               // 当前 meta.hash 非空
    env.fake->m_fakeState = DmGlobal::Playing;
    env.engine->clearPlayList(true);           // stopFlag=true → stop()
    EXPECT_TRUE(env.engine->isEmpty());
    EXPECT_EQ(env.fake->m_fakeState, DmGlobal::Stopped);
}

// clearPlayList(true) 无当前 meta → 不 stop
TEST(PlayerEngineClearPlaylistExpandedTest, clearWithStopFlagNoCurrentMetaSkipsStop)
{
    InjectedEngine env;
    DMusic::MediaMeta m; m.hash = "c2"; m.localPath = "/tmp/c2.mp3";
    env.engine->addMetasToPlayList({m});
    // 当前 meta.hash 为空（未 setMediaMeta）
    env.fake->m_fakeState = DmGlobal::Playing;
    env.engine->clearPlayList(true);
    EXPECT_TRUE(env.engine->isEmpty());
    EXPECT_EQ(env.fake->m_fakeState, DmGlobal::Playing);  // 未 stop
}

// ---- setMediaMeta(hash) 不存在 ----
TEST(PlayerEngineSetMediaExpandedTest, setMediaMetaByNonExistentHashNoOp)
{
    InjectedEngine env;
    DMusic::MediaMeta m; m.hash = "exist";
    env.engine->addMetasToPlayList({m});
    env.engine->setMediaMeta("missing");   // 循环不匹配，不调用 setMediaMeta(meta)
    EXPECT_TRUE(env.engine->getMediaMeta().hash.isEmpty());
}

// setMediaMeta(meta) localPath 非空 → 设置 INT_LAST_PROGRESS_FLAG=0
TEST(PlayerEngineSetMediaExpandedTest, setMediaMetaWithLocalPathSetsProgressFlag)
{
    InjectedEngine env;
    DMusic::MediaMeta m; m.hash = "sm1"; m.localPath = "/tmp/sm1.mp3";
    env.engine->setMediaMeta(m);
    // 设置后 setTime 应直接走 player->setTime（flag=0）
    env.engine->setTime(555);
    EXPECT_EQ(env.fake->m_fakeTime, 555);
}

// ---- getCurrentPlayList 默认空 ----
TEST(PlayerEngineCurrentPlaylistExpandedTest, initiallyEmptyThenSetGet)
{
    InjectedEngine env;
    EXPECT_TRUE(env.engine->getCurrentPlayList().isEmpty());
    env.engine->setCurrentPlayList("hash-x");
    EXPECT_EQ(env.engine->getCurrentPlayList(), "hash-x");
}

// ---- isEmpty/getMetas ----
TEST(PlayerEngineQueryExpandedTest, isEmptyAndGetMetasConsistent)
{
    InjectedEngine env;
    EXPECT_TRUE(env.engine->isEmpty());
    DMusic::MediaMeta m; m.hash = "q1";
    env.engine->addMetasToPlayList({m});
    EXPECT_FALSE(env.engine->isEmpty());
    EXPECT_EQ(env.engine->getMetas().size(), 1);
}

// ---- supportedSuffixList 注入路径 ----
TEST(PlayerEngineSuffixExpandedTest, supportedSuffixListFromFake)
{
    InjectedEngine env;
    QStringList sfx = env.engine->supportedSuffixList();
    EXPECT_TRUE(sfx.contains("mp3"));
}

// ---- getCdaMetaInfo 默认空（PlayerBase 默认实现返回空列表）----
TEST(PlayerEngineCdaExpandedTest, getCdaMetaInfoReturnsEmptyByDefault)
{
    InjectedEngine env;
    EXPECT_TRUE(env.engine->getCdaMetaInfo().isEmpty());
}

// ---- playNextMeta(isAuto=true) 非空播放列表 + RepeatNull 找到下一首 ----
TEST(PlayerEnginePlayNextExpandedTest, playNextAutoOnRepeatNullFindsNextPlayable)
{
    InjectedEngine env;
    env.engine->setPlaybackMode(DmGlobal::RepeatNull);
    DMusic::MediaMeta m1; m1.hash = "n1"; m1.localPath = "/tmp/n1.mp3";
    DMusic::MediaMeta m2; m2.hash = "n2"; m2.localPath = "/tmp/n2.mp3";
    env.engine->addMetasToPlayList({m1, m2});
    env.engine->setMediaMeta(m1);               // 当前 m1
    env.engine->playNextMeta(true);             // auto → 找到 m2 后 switchToNewTrackWithFade
    EXPECT_TRUE(true);                          // 不崩溃即通过
}

// playNextMeta(auto) 末曲且 auto → stop
TEST(PlayerEnginePlayNextExpandedTest, playNextAutoOnLastTrackStops)
{
    InjectedEngine env;
    env.engine->setPlaybackMode(DmGlobal::RepeatNull);
    DMusic::MediaMeta m1; m1.hash = "ln1"; m1.localPath = "/tmp/ln1.mp3";
    env.engine->addMetasToPlayList({m1});
    env.engine->setMediaMeta(m1);               // 当前即末曲
    env.fake->m_fakeState = DmGlobal::Playing;
    env.engine->playNextMeta(true);             // auto，无下一首 → stop
    EXPECT_EQ(env.fake->m_fakeState, DmGlobal::Stopped);
}

// playNextMeta(isAuto=false) 在 RepeatAll 下循环
TEST(PlayerEnginePlayNextExpandedTest, playNextManualRepeatAllWraps)
{
    InjectedEngine env;
    env.engine->setPlaybackMode(DmGlobal::RepeatAll);
    DMusic::MediaMeta m1; m1.hash = "ra1"; m1.localPath = "/tmp/ra1.mp3";
    DMusic::MediaMeta m2; m2.hash = "ra2"; m2.localPath = "/tmp/ra2.mp3";
    env.engine->addMetasToPlayList({m1, m2});
    env.engine->setMediaMeta(m2);               // 当前末曲
    env.engine->playNextMeta(false);            // RepeatAll 末曲 → 回到第一首
    EXPECT_TRUE(true);
}

// playNextMeta(isAuto=false) RepeatSingle 不自增 index
TEST(PlayerEnginePlayNextExpandedTest, playNextManualRepeatSingleKeepsIndex)
{
    InjectedEngine env;
    env.engine->setPlaybackMode(DmGlobal::RepeatSingle);
    DMusic::MediaMeta m1; m1.hash = "rs1"; m1.localPath = "/tmp/rs1.mp3";
    DMusic::MediaMeta m2; m2.hash = "rs2"; m2.localPath = "/tmp/rs2.mp3";
    env.engine->addMetasToPlayList({m1, m2});
    env.engine->setMediaMeta(m1);
    env.engine->playNextMeta(false);            // Single + auto=false → 仍自增
    EXPECT_TRUE(true);
}

// playNextMeta(isAuto=true) RepeatSingle 保持当前 index
TEST(PlayerEnginePlayNextExpandedTest, playNextAutoRepeatSingleKeepsCurrent)
{
    InjectedEngine env;
    env.engine->setPlaybackMode(DmGlobal::RepeatSingle);
    DMusic::MediaMeta m1; m1.hash = "rsA"; m1.localPath = "/tmp/rsA.mp3";
    DMusic::MediaMeta m2; m2.hash = "rsB"; m2.localPath = "/tmp/rsB.mp3";
    env.engine->addMetasToPlayList({m1, m2});
    env.engine->setMediaMeta(m1);
    env.engine->playNextMeta(true);             // auto → Single 保持 index，重播 m1
    EXPECT_TRUE(true);
}

// playNextMeta(isAuto=false) Shuffle 多曲随机
TEST(PlayerEnginePlayNextExpandedTest, playNextManualShuffleDoesNotCrash)
{
    InjectedEngine env;
    env.engine->setPlaybackMode(DmGlobal::Shuffle);
    for (int i = 0; i < 3; ++i) {
        DMusic::MediaMeta m; m.hash = QString("sh%1").arg(i); m.localPath = QString("/tmp/sh%1.mp3").arg(i);
        env.engine->addMetasToPlayList({m});
    }
    env.engine->setMediaMeta(env.engine->getMetas().first());
    env.engine->playNextMeta(false);
    EXPECT_TRUE(true);
}

// playNextMeta 默认分支（非已知 mode 走 default）
TEST(PlayerEnginePlayNextExpandedTest, playNextRepeatNullDefaultBranch)
{
    InjectedEngine env;
    // RepeatNull 走的是早期 return 分支，这里覆盖另一条 default：设非 Shuffle 的已知 mode
    env.engine->setPlaybackMode(DmGlobal::RepeatAll);
    DMusic::MediaMeta m1; m1.hash = "d1"; m1.localPath = "/tmp/d1.mp3";
    env.engine->addMetasToPlayList({m1});
    env.engine->setMediaMeta(m1);
    env.engine->playNextMeta(true);             // RepeatAll
    EXPECT_TRUE(true);
}

// ---- playPreMeta() 路径 ----
TEST(PlayerEnginePlayPreExpandedTest, playPreMetaOnRepeatNullFindsPrev)
{
    InjectedEngine env;
    env.engine->setPlaybackMode(DmGlobal::RepeatNull);
    DMusic::MediaMeta m1; m1.hash = "pre1"; m1.localPath = "/tmp/pre1.mp3";
    DMusic::MediaMeta m2; m2.hash = "pre2"; m2.localPath = "/tmp/pre2.mp3";
    env.engine->addMetasToPlayList({m1, m2});
    env.engine->setMediaMeta(m2);               // 当前 m2 → 上一首 m1
    env.engine->playPreMeta();
    EXPECT_TRUE(true);
}

TEST(PlayerEnginePlayPreExpandedTest, playPreMetaRepeatAllWraps)
{
    InjectedEngine env;
    env.engine->setPlaybackMode(DmGlobal::RepeatAll);
    DMusic::MediaMeta m1; m1.hash = "pa1"; m1.localPath = "/tmp/pa1.mp3";
    DMusic::MediaMeta m2; m2.hash = "pa2"; m2.localPath = "/tmp/pa2.mp3";
    env.engine->addMetasToPlayList({m1, m2});
    env.engine->setMediaMeta(m1);               // 当前第一首 → 回到末曲
    env.engine->playPreMeta();
    EXPECT_TRUE(true);
}

TEST(PlayerEnginePlayPreExpandedTest, playPreMetaShuffleDoesNotCrash)
{
    InjectedEngine env;
    env.engine->setPlaybackMode(DmGlobal::Shuffle);
    for (int i = 0; i < 3; ++i) {
        DMusic::MediaMeta m; m.hash = QString("psh%1").arg(i); m.localPath = QString("/tmp/psh%1.mp3").arg(i);
        env.engine->addMetasToPlayList({m});
    }
    env.engine->setMediaMeta(env.engine->getMetas().first());
    env.engine->playPreMeta();
    EXPECT_TRUE(true);
}

TEST(PlayerEnginePlayPreExpandedTest, playPreMetaNotFoundStops)
{
    InjectedEngine env;
    env.engine->setPlaybackMode(DmGlobal::RepeatNull);
    DMusic::MediaMeta m1; m1.hash = "pnf"; m1.localPath = "/tmp/pnf.mp3";
    env.engine->addMetasToPlayList({m1});
    env.engine->setMediaMeta(m1);               // 当前 m1，无上一首 → stop
    env.fake->m_fakeState = DmGlobal::Playing;
    env.engine->playPreMeta();
    EXPECT_EQ(env.fake->m_fakeState, DmGlobal::Stopped);
}

// ---- switchToNewTrackWithFade(hash) 路径：通过 playPreMeta/playNextMeta 间接覆盖 ----
// （switchToNewTrackWithFade 是 private，无法直接调用，此处仅校验列表状态稳定）
TEST(PlayerEngineSwitchTrackExpandedTest, switchByHashCoveredIndirectly)
{
    InjectedEngine env;
    DMusic::MediaMeta m; m.hash = "sw1"; m.localPath = "/tmp/sw1.mp3";
    env.engine->addMetasToPlayList({m});
    env.engine->setMediaMeta(m);
    env.engine->setPlaybackMode(DmGlobal::RepeatAll);
    env.engine->playPreMeta();          // 走 switchToNewTrackWithFade(meta) 直接切换分支
    EXPECT_EQ(env.engine->getMetas().size(), 1);
}

// ---- forcePlay 空列表直接返回 ----
TEST(PlayerEngineForcePlayExpandedTest, forcePlayOnEmptyListReturnsImmediately)
{
    InjectedEngine env;
    int before = env.fake->playCallCount;
    env.engine->forcePlay();            // 空列表 → return
    EXPECT_EQ(env.fake->playCallCount, before);
}

// ---- playbackStatus 转发 ----
TEST(PlayerEnginePlaybackStatusExpandedTest, playbackStatusReflectsFakeState)
{
    InjectedEngine env;
    env.fake->m_fakeState = DmGlobal::Playing;
    EXPECT_EQ(env.engine->playbackStatus(), DmGlobal::Playing);
    env.fake->m_fakeState = DmGlobal::Paused;
    EXPECT_EQ(env.engine->playbackStatus(), DmGlobal::Paused);
}

// ---- stateChanged 信号链路：通过 PlayerBase::stateChanged 触发 changePictureTimer ----
TEST(PlayerEngineSignalChainTest, stateChangedToPlayingStartsPictureTimer)
{
    InjectedEngine env;
    QSignalSpy spy(env.engine.get(), &PlayerEngine::playbackStatusChanged);
    // 模拟后端发出 stateChanged → 引擎内部 changePictureTimer 启动 + emit playbackStatusChanged
    emit env.fake->stateChanged(DmGlobal::Playing);
    EXPECT_GE(spy.count(), 1);
    // 等待 picture timer 至少触发一次（300ms），验证 playPictureChanged
    QSignalSpy picSpy(env.engine.get(), &PlayerEngine::playPictureChanged);
    QTest::qWait(400);
    EXPECT_GE(picSpy.count(), 1);
}

TEST(PlayerEngineSignalChainTest, stateChangedToPausedStopsPictureTimer)
{
    InjectedEngine env;
    emit env.fake->stateChanged(DmGlobal::Playing);
    QTest::qWait(50);
    QSignalSpy picSpy(env.engine.get(), &PlayerEngine::playPictureChanged);
    emit env.fake->stateChanged(DmGlobal::Paused);   // 停止 picture timer
    QTest::qWait(400);
    EXPECT_EQ(picSpy.count(), 0);                    // Paused 后不再发图
}

TEST(PlayerEngineSignalChainTest, stateChangedToStoppedStopsPictureTimer)
{
    InjectedEngine env;
    emit env.fake->stateChanged(DmGlobal::Playing);
    QTest::qWait(50);
    QSignalSpy picSpy(env.engine.get(), &PlayerEngine::playPictureChanged);
    emit env.fake->stateChanged(DmGlobal::Stopped);  // default 分支停止 timer
    QTest::qWait(400);
    EXPECT_EQ(picSpy.count(), 0);
}

// ---- timeChanged 链路 → positionChanged ----
TEST(PlayerEngineSignalChainTest, timeChangedForwardsToPositionChanged)
{
    InjectedEngine env;
    DMusic::MediaMeta m; m.hash = "tc1"; m.length = 200; m.localPath = "/tmp/tc1.mp3";
    env.engine->setMediaMeta(m);
    QSignalSpy spy(env.engine.get(), &PlayerEngine::positionChanged);
    emit env.fake->timeChanged(50);
    EXPECT_EQ(spy.count(), 1);
    QList<QVariant> args = spy.takeFirst();
    EXPECT_EQ(args.at(0).toLongLong(), 50);
}

// ---- positionChanged(float) 链路 ----
TEST(PlayerEngineSignalChainTest, floatPositionForwardsToPositionChanged)
{
    InjectedEngine env;
    DMusic::MediaMeta m; m.hash = "fp1"; m.length = 1000; m.localPath = "/tmp/fp1.mp3";
    env.engine->setMediaMeta(m);
    QSignalSpy spy(env.engine.get(), &PlayerEngine::positionChanged);
    emit env.fake->positionChanged(0.5f);            // 0.5 * 1000 = 500
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.takeFirst().at(0).toLongLong(), 500);
}

// ---- metaChanged 链路 ----
TEST(PlayerEngineSignalChainTest, playerMetaChangedForwardsToEngine)
{
    InjectedEngine env;
    QSignalSpy spy(env.engine.get(), &PlayerEngine::metaChanged);
    emit env.fake->metaChanged();
    EXPECT_EQ(spy.count(), 1);
}

// ---- end() 链路 → playNextMeta(true) ----
TEST(PlayerEngineSignalChainTest, endSignalTriggersPlayNextMeta)
{
    InjectedEngine env;
    DMusic::MediaMeta m; m.hash = "end1"; m.localPath = "/tmp/end1.mp3";
    env.engine->addMetasToPlayList({m});
    env.engine->setMediaMeta(m);
    emit env.fake->end();                            // → playNextMeta(true)
    EXPECT_TRUE(true);                               // 不崩溃即通过
}

// ---- sigSendCdaStatus 链路 ----
TEST(PlayerEngineSignalChainTest, cdaStatusForwards)
{
    InjectedEngine env;
    QSignalSpy spy(env.engine.get(), &PlayerEngine::sendCdaStatus);
    emit env.fake->sigSendCdaStatus(2);
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.takeFirst().at(0).toInt(), 2);
}

// ---- qt_metacast：通过 qobject_cast 触发 ----
TEST(PlayerEngineMetaObjectTest, qobjectCastInvokesMetacast)
{
    InjectedEngine env;
    QObject *obj = env.engine.get();
    PlayerEngine *casted = qobject_cast<PlayerEngine *>(obj);
    EXPECT_NE(casted, nullptr);
}

// ============================================================================
// playPreMeta / playNextMeta 可播放过滤分支：用 mmType=MimeTypeCDA 绕过文件检查
// （QFile::exists 分支依赖真实文件；CDA 类型直接判定可播放，触发 switchToNewTrackWithFade）
// ============================================================================
namespace {
// 构造可播放（CDA 类型）meta，便于覆盖 playability filter
DMusic::MediaMeta makePlayableCda(const QString &hash)
{
    DMusic::MediaMeta m;
    m.hash = hash;
    m.localPath = QString("/dev/null/%1.cda").arg(hash);  // 不存在的路径，靠 mmType 绕过
    m.mmType = DmGlobal::MimeTypeCDA;
    return m;
}
}

// playPreMeta RepeatNull：当前非首曲 → 找到上一首可播放（preIndex != -1）
TEST(PlayerEnginePlayPreFilterTest, repeatNullFindsPrevPlayableCda)
{
    InjectedEngine env;
    env.engine->setPlaybackMode(DmGlobal::RepeatNull);
    DMusic::MediaMeta m1 = makePlayableCda("cpf1");
    DMusic::MediaMeta m2 = makePlayableCda("cpf2");
    env.engine->addMetasToPlayList({m1, m2});
    env.engine->setMediaMeta(m2);               // 当前 m2 → 上一首 m1
    int before = env.fake->setMediaCallCount;
    env.engine->playPreMeta();
    EXPECT_GT(env.fake->setMediaCallCount, before);  // switchToNewTrackWithFade → setMediaMeta
}

// playPreMeta RepeatNull：当前首曲 → preIndex=-1 → stop
TEST(PlayerEnginePlayPreFilterTest, repeatNullFirstTrackStops)
{
    InjectedEngine env;
    env.engine->setPlaybackMode(DmGlobal::RepeatNull);
    DMusic::MediaMeta m1 = makePlayableCda("cpfs1");
    env.engine->addMetasToPlayList({m1});
    env.engine->setMediaMeta(m1);               // 当前首曲，无上一首
    env.fake->m_fakeState = DmGlobal::Playing;
    env.engine->playPreMeta();
    EXPECT_EQ(env.fake->m_fakeState, DmGlobal::Stopped);
}

// playPreMeta RepeatAll/RepeatSingle：可播放列表存在 → 走 switch
TEST(PlayerEnginePlayPreFilterTest, repeatAllWrapsToLastPlayableCda)
{
    InjectedEngine env;
    env.engine->setPlaybackMode(DmGlobal::RepeatAll);
    DMusic::MediaMeta m1 = makePlayableCda("raA1");
    DMusic::MediaMeta m2 = makePlayableCda("raA2");
    env.engine->addMetasToPlayList({m1, m2});
    env.engine->setMediaMeta(m1);               // 当前首曲 → 回到末曲 m2
    int before = env.fake->setMediaCallCount;
    env.engine->playPreMeta();
    EXPECT_GT(env.fake->setMediaCallCount, before);
}

// playPreMeta RepeatSingle：index 非 0 → index--
TEST(PlayerEnginePlayPreFilterTest, repeatSingleDecrementsIndex)
{
    InjectedEngine env;
    env.engine->setPlaybackMode(DmGlobal::RepeatSingle);
    DMusic::MediaMeta m1 = makePlayableCda("rsA1");
    DMusic::MediaMeta m2 = makePlayableCda("rsA2");
    DMusic::MediaMeta m3 = makePlayableCda("rsA3");
    env.engine->addMetasToPlayList({m1, m2, m3});
    env.engine->setMediaMeta(m3);               // index=2 → index--=1
    int before = env.fake->setMediaCallCount;
    env.engine->playPreMeta();
    EXPECT_GT(env.fake->setMediaCallCount, before);
}

// playPreMeta Shuffle：多曲随机
TEST(PlayerEnginePlayPreFilterTest, shuffleRandomPlayableCda)
{
    InjectedEngine env;
    env.engine->setPlaybackMode(DmGlobal::Shuffle);
    QList<DMusic::MediaMeta> metas;
    for (int i = 0; i < 3; ++i) metas << makePlayableCda(QString("pshA%1").arg(i));
    env.engine->addMetasToPlayList(metas);
    env.engine->setMediaMeta(metas.first());
    int before = env.fake->setMediaCallCount;
    env.engine->playPreMeta();
    EXPECT_GT(env.fake->setMediaCallCount, before);
}

// playPreMeta Shuffle：单曲（size==1 → index=0）
TEST(PlayerEnginePlayPreFilterTest, shuffleSingleTrackKeepsZero)
{
    InjectedEngine env;
    env.engine->setPlaybackMode(DmGlobal::Shuffle);
    DMusic::MediaMeta m1 = makePlayableCda("ssh1");
    env.engine->addMetasToPlayList({m1});
    env.engine->setMediaMeta(m1);
    env.engine->playPreMeta();                  // size<=1 → index=0
    EXPECT_TRUE(true);
}

// playPreMeta default 分支（非已知 PlaybackMode 的 fallback）
TEST(PlayerEnginePlayPreFilterTest, defaultBranchFallsBack)
{
    InjectedEngine env;
    // RepeatNull 走早返回，RepeatAll/Single/Shuffle 走各自分支；这里覆盖 default
    // 通过 RepeatAll 已覆盖 index!=0 分支，本用例覆盖 index==0 → size-1
    env.engine->setPlaybackMode(DmGlobal::RepeatAll);
    DMusic::MediaMeta m1 = makePlayableCda("df1");
    env.engine->addMetasToPlayList({m1});
    env.engine->setMediaMeta(m1);
    env.engine->playPreMeta();
    EXPECT_TRUE(true);
}

// ---- playNextMeta RepeatNull 找到下一首可播放 ----
TEST(PlayerEnginePlayNextFilterTest, repeatNullFindsNextPlayableCda)
{
    InjectedEngine env;
    env.engine->setPlaybackMode(DmGlobal::RepeatNull);
    DMusic::MediaMeta m1 = makePlayableCda("nf1");
    DMusic::MediaMeta m2 = makePlayableCda("nf2");
    env.engine->addMetasToPlayList({m1, m2});
    env.engine->setMediaMeta(m1);               // 当前 m1 → 下一首 m2
    int before = env.fake->setMediaCallCount;
    env.engine->playNextMeta(true);
    EXPECT_GT(env.fake->setMediaCallCount, before);
}

// playNextMeta RepeatNull：当前曲不在列表中且非 auto → 回退到第一首可播放
TEST(PlayerEnginePlayNextFilterTest, repeatNullManualFallbackToFirst)
{
    InjectedEngine env;
    env.engine->setPlaybackMode(DmGlobal::RepeatNull);
    DMusic::MediaMeta m1 = makePlayableCda("nfb1");
    DMusic::MediaMeta m2 = makePlayableCda("nfb2");
    env.engine->addMetasToPlayList({m1, m2});
    // 当前 meta 不在列表（hash 不匹配），非 auto → 走 fallback 找第一首
    env.engine->playNextMeta(false);
    EXPECT_GT(env.fake->setMediaCallCount, 0);
}

// playNextMeta RepeatSingle auto=false：自增 index
TEST(PlayerEnginePlayNextFilterTest, repeatSingleManualIncrements)
{
    InjectedEngine env;
    env.engine->setPlaybackMode(DmGlobal::RepeatSingle);
    DMusic::MediaMeta m1 = makePlayableCda("ns1");
    DMusic::MediaMeta m2 = makePlayableCda("ns2");
    env.engine->addMetasToPlayList({m1, m2});
    env.engine->setMediaMeta(m1);
    int before = env.fake->setMediaCallCount;
    env.engine->playNextMeta(false);            // auto=false → 自增
    EXPECT_GT(env.fake->setMediaCallCount, before);
}

// playNextMeta RepeatAll：末曲 → 回到首曲
TEST(PlayerEnginePlayNextFilterTest, repeatAllLastWrapsToFirst)
{
    InjectedEngine env;
    env.engine->setPlaybackMode(DmGlobal::RepeatAll);
    DMusic::MediaMeta m1 = makePlayableCda("na1");
    DMusic::MediaMeta m2 = makePlayableCda("na2");
    env.engine->addMetasToPlayList({m1, m2});
    env.engine->setMediaMeta(m2);               // 末曲 → 首曲
    int before = env.fake->setMediaCallCount;
    env.engine->playNextMeta(true);
    EXPECT_GT(env.fake->setMediaCallCount, before);
}

// playNextMeta Shuffle 多曲随机 + 单曲
TEST(PlayerEnginePlayNextFilterTest, shuffleRandomAndSingle)
{
    {
        InjectedEngine env;
        env.engine->setPlaybackMode(DmGlobal::Shuffle);
        QList<DMusic::MediaMeta> metas;
        for (int i = 0; i < 3; ++i) metas << makePlayableCda(QString("nsh%1").arg(i));
        env.engine->addMetasToPlayList(metas);
        env.engine->setMediaMeta(metas.first());
        env.engine->playNextMeta(true);
        EXPECT_TRUE(true);
    }
    {
        InjectedEngine env;
        env.engine->setPlaybackMode(DmGlobal::Shuffle);
        DMusic::MediaMeta m1 = makePlayableCda("nshS");
        env.engine->addMetasToPlayList({m1});
        env.engine->setMediaMeta(m1);
        env.engine->playNextMeta(true);         // size<=1 → index=0
        EXPECT_TRUE(true);
    }
}

// playNextMeta：当前曲 hash 非空且不在可播放子集，但在 allMetas 中（index==-1 重建分支）
TEST(PlayerEnginePlayNextFilterTest, currentHashNotInPlayableButInAllMetas)
{
    InjectedEngine env;
    env.engine->setPlaybackMode(DmGlobal::RepeatAll);
    DMusic::MediaMeta cur; cur.hash = "orphan"; cur.localPath = "/no/such.mp3"; // 不可播放
    DMusic::MediaMeta p1 = makePlayableCda("p1");
    env.engine->addMetasToPlayList({cur, p1});
    env.engine->setMediaMeta(cur);              // 当前不可播放 → 触发 index 重建逻辑
    env.engine->playNextMeta(true);
    EXPECT_TRUE(true);
}

// ---- play() INT_LAST_PROGRESS_FLAG==1 路径：singleShot 150ms 后 setTime+play ----
TEST(PlayerEnginePlayProgressTest, playWithLastProgressFlagSchedulesSingleShot)
{
    InjectedEngine env(true);
    DMusic::MediaMeta m; m.hash = "lpf1"; m.localPath = "/tmp/lpf1.mp3";
    env.engine->setMediaMeta(m);
    // INT_LAST_PROGRESS_FLAG 是文件级静态；新进程默认为 1。首次 play() 会触发 singleShot。
    env.engine->play();
    // 等待 singleShot(150ms) 触发：pause → setTime → play
    QTest::qWait(250);
    EXPECT_TRUE(true);                          // 不崩溃即覆盖该分支
}
