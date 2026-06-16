// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// player/vlc 模块测试：用真实 libvlc 构造 VlcInstance/MediaPlayer/Equalizer，
// 覆盖构造 + getter/setter（不实际播放——播放需媒体+音频后端，难单测）。
// 前提：系统装了 libvlc（libvlc.so 可用）。

#include <gtest/gtest.h>

#include <QString>
#include <QStringList>
#include <memory>

#include "Common.h"
#include "Error.h"
#include "Enums.h"
#include "Instance.h"
#include "MediaPlayer.h"
#include "Equalizer.h"
#include "Media.h"
#include "vlcdynamicinstance.h"
#include "sdlplayer.h"

#include <QFile>

#ifndef TEST_DATA_DIR
#define TEST_DATA_DIR "."
#endif

namespace {
QString sampleMp3Path() { return QString(TEST_DATA_DIR) + "/sample.mp3"; }
}

// ============================================================================
// fixture：每个测试创建 VlcInstance（libvlc_new）
// ============================================================================
class VlcTestFixture : public ::testing::Test {
protected:
    void SetUp() override
    {
        instance = std::make_unique<VlcInstance>(QStringList(), nullptr);
    }
    std::unique_ptr<VlcInstance> instance;
};

// ============================================================================
// 纯工具（不依赖 libvlc 实例）
// ============================================================================
TEST(VlcCommonTest, argsReturnsDefaultArgs)
{
    const QStringList args = VlcCommon::args();
    EXPECT_FALSE(args.isEmpty());
    EXPECT_TRUE(args.contains("--intf=dummy"));  // 默认参数
}

TEST(VlcEnumsTest, constructs)
{
    Vlc vlc;
    SUCCEED();
}

// VlcError 暂跳过：errmsg 经 DynamicLibraries::resolve("libvlc_errmsg") 动态取符号，
// libvlc 未加载时返回 null 解引用 SEGV；需先确保 DynamicLibraries 已加载 libvlc，
// 放到 VlcInstance（触发加载）之后作为 TEST_F 才安全。先聚焦构造/getter/setter。

// ============================================================================
// VlcInstance（libvlc_new）
// ============================================================================
TEST_F(VlcTestFixture, instanceCoreValid)
{
    EXPECT_NE(instance->core(), nullptr);
}

TEST_F(VlcTestFixture, instanceVersion)
{
    EXPECT_FALSE(VlcInstance::version().isEmpty());
}

TEST_F(VlcTestFixture, instanceLogLevelDefault)
{
    EXPECT_GE(instance->logLevel(), 0);
}

TEST_F(VlcTestFixture, instanceCatchPulseError)
{
    instance->catchPulseError(0);  // 不崩溃
    SUCCEED();
}

// ============================================================================
// VlcMediaPlayer（libvlc_media_player_new）
// ============================================================================
TEST_F(VlcTestFixture, mediaPlayerCoreValid)
{
    VlcMediaPlayer mp(instance.get());
    EXPECT_NE(mp.core(), nullptr);
}

TEST_F(VlcTestFixture, mediaPlayerLengthWithoutMedia)
{
    VlcMediaPlayer mp(instance.get());
    EXPECT_EQ(mp.length(), -1);  // libvlc 契约：无媒体 length=-1
}

TEST_F(VlcTestFixture, mediaPlayerTimeWithoutMedia)
{
    VlcMediaPlayer mp(instance.get());
    EXPECT_LE(mp.time(), 0);  // 无媒体
}

TEST_F(VlcTestFixture, mediaPlayerStateWithoutMedia)
{
    VlcMediaPlayer mp(instance.get());
    EXPECT_NE(mp.state(), Vlc::State::Playing);
}

TEST_F(VlcTestFixture, mediaPlayerVolumeMute)
{
    VlcMediaPlayer mp(instance.get());
    mp.setVolume(50);
    mp.setMute(true);
    mp.setMute(false);
    EXPECT_GE(mp.getVolume(), 0);
}

TEST_F(VlcTestFixture, mediaPlayerSetTime)
{
    VlcMediaPlayer mp(instance.get());
    mp.setTime(1000);
    SUCCEED();
}

TEST_F(VlcTestFixture, mediaPlayerEqualizerValid)
{
    VlcMediaPlayer mp(instance.get());
    EXPECT_NE(mp.equalizer(), nullptr);
}

// ============================================================================
// VlcEqualizer
// ============================================================================
TEST_F(VlcTestFixture, equalizerPreamplificationDefault)
{
    VlcMediaPlayer mp(instance.get());
    EXPECT_GE(mp.equalizer()->preamplification(), -1.0);
}

TEST_F(VlcTestFixture, equalizerSetPreamplification)
{
    VlcMediaPlayer mp(instance.get());
    mp.equalizer()->setPreamplification(5.0);
    EXPECT_FLOAT_EQ(mp.equalizer()->preamplification(), 5.0);
}

TEST_F(VlcTestFixture, equalizerSetAmplificationForBand)
{
    VlcMediaPlayer mp(instance.get());
    mp.equalizer()->setAmplificationForBandAt(3.0, 0);
    EXPECT_FLOAT_EQ(mp.equalizer()->amplificationForBandAt(0), 3.0);
}

TEST_F(VlcTestFixture, equalizerSetEnabled)
{
    VlcMediaPlayer mp(instance.get());
    mp.equalizer()->setEnabled(true);
    mp.equalizer()->setEnabled(false);
    SUCCEED();
}

TEST_F(VlcTestFixture, equalizerLoadFromPreset)
{
    VlcMediaPlayer mp(instance.get());
    mp.equalizer()->loadFromPreset(0);  // 第一个预设
    SUCCEED();
}

// ============================================================================
// VlcMedia：用真实 sample.mp3 测构造/initMedia/core（libvlc_media_new_path）
// ============================================================================
TEST_F(VlcTestFixture, mediaInitFromSampleFile)
{
    ASSERT_TRUE(QFile::exists(sampleMp3Path())) << "testdata/sample.mp3 missing";
    VlcMedia media;
    media.initMedia(sampleMp3Path(), true, instance.get());
    EXPECT_NE(media.core(), nullptr);  // libvlc_media 创建成功
}

TEST_F(VlcTestFixture, mediaCdaTrackDefault)
{
    VlcMedia media;
    EXPECT_EQ(media.getCdaTrack(), -1);  // 非 CD，默认 -1
}

// ============================================================================
// VlcMediaPlayer::open：把 VlcMedia 绑定到 player（libvlc_media_player_set_media）
// 覆盖 MediaPlayer 的 open 路径，不实际播放
// ============================================================================
TEST_F(VlcTestFixture, mediaPlayerOpenMedia)
{
    ASSERT_TRUE(QFile::exists(sampleMp3Path()));
    VlcMedia media;
    media.initMedia(sampleMp3Path(), true, instance.get());
    ASSERT_NE(media.core(), nullptr);
    VlcMediaPlayer mp(instance.get());
    mp.open(&media);  // 绑定媒体到 player
    SUCCEED();
}

// ============================================================================
// VlcDynamicInstance：动态加载 libvlc/SDL + 符号解析（不依赖 SDL 音频）
// ============================================================================
TEST(VlcDynamicInstanceTest, functionInstanceAndResolveVlcSymbol)
{
    VlcDynamicInstance *inst = VlcDynamicInstance::VlcFunctionInstance();
    EXPECT_NE(inst, nullptr);
    // libvlc_new 应能 resolve（libvlc 已加载）
    QFunctionPointer fp = inst->resolveSymbol("libvlc_new");
    EXPECT_NE(fp, nullptr);
}

TEST(VlcDynamicInstanceTest, loadSdlLibraryAndResolveSdlSymbol)
{
    VlcDynamicInstance *inst = VlcDynamicInstance::VlcFunctionInstance();
    EXPECT_TRUE(inst->loadSdlLibrary());  // libSDL2 在系统
    QFunctionPointer sdl = inst->resolveSdlSymbol("SDL_Init");
    EXPECT_NE(sdl, nullptr);
}

// ============================================================================
// VlcMediaPlayer play/pause/stop：libvlc_media_player_play 路径（不实际播放完）
// open + play + pause + stop 快速往返，覆盖播放控制代码
// ============================================================================
TEST_F(VlcTestFixture, mediaPlayerPlayPauseStopSampleFile)
{
    VlcMedia media;
    media.initMedia(sampleMp3Path(), true, instance.get());
    VlcMediaPlayer mp(instance.get());
    mp.open(&media);
    mp.play();
    mp.pause();
    mp.stop();
    SUCCEED();
}

// ============================================================================
// SdlPlayer：构造（SDL_Init + libvlc audio 回调，不 play/open audio device）
// + 配置 setter（不触发 SDL audio callback）。需 SDL_AUDIODRIVER=dummy。
// ============================================================================
TEST_F(VlcTestFixture, sdlPlayerConstructsAndConfig)
{
    SdlPlayer sdlp(instance.get());  // SDL_Init + libvlc 回调 + new CheckDataZeroThread
    sdlp.setVolume(50);
    EXPECT_GE(sdlp.getVolume(), 0);
    sdlp.setMute(true);
    sdlp.setMute(false);
    SUCCEED();  // 构造 + 配置不崩溃
}
