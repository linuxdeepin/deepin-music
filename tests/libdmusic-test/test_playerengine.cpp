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

#include "playerengine.h"
#include "global.h"

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
