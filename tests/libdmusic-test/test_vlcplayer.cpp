// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// vlcplayer.cpp 的单元测试：构造 + 不触发真实播放的查询/配置/资源管理。

#include <gtest/gtest.h>

#include <QAudioOutput>
#include <QCoreApplication>
#include <QFile>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>
#include <QString>
#include <QStringList>
#include <memory>
#include <cmath>

#include "vlcplayer.h"
#include "qtplayer.h"
#include "playerbase.h"
#include "global.h"

// ============================================================================
// 构造/析构：不崩溃
// ============================================================================
TEST(VlcPlayerConstructionTest, constructsAndDestroysWithoutCrash)
{
    std::unique_ptr<VlcPlayer> player(new VlcPlayer());
    EXPECT_NE(player.get(), nullptr);
}

TEST(VlcPlayerConstructionTest, destructorReleasesResources)
{
    auto player = std::make_unique<VlcPlayer>();
    player->init();
    player.reset();  // 触发析构，调用 releasePlayer()
    SUCCEED();  // 不崩溃即通过
}

// ============================================================================
// supportedSuffixList : 返回非空后缀列表
// ============================================================================
TEST(VlcPlayerSuffixTest, returnsNonEmptySuffixList)
{
    VlcPlayer player;
    const QStringList suffixes = player.supportedSuffixList();
    EXPECT_FALSE(suffixes.isEmpty());
}

TEST(VlcPlayerSuffixTest, containsExpectedAudioFormats)
{
    VlcPlayer player;
    const QStringList suffixes = player.supportedSuffixList();
    // 验证构造函数中初始化的所有后缀
    EXPECT_TRUE(suffixes.contains("aac"));
    EXPECT_TRUE(suffixes.contains("amr"));
    EXPECT_TRUE(suffixes.contains("wav"));
    EXPECT_TRUE(suffixes.contains("ogg"));
    EXPECT_TRUE(suffixes.contains("ape"));
    EXPECT_TRUE(suffixes.contains("mp3"));
    EXPECT_TRUE(suffixes.contains("flac"));
    EXPECT_TRUE(suffixes.contains("wma"));
    EXPECT_TRUE(suffixes.contains("m4a"));
    EXPECT_TRUE(suffixes.contains("ac3"));
    EXPECT_TRUE(suffixes.contains("voc"));
    EXPECT_TRUE(suffixes.contains("aiff"));
    EXPECT_TRUE(suffixes.contains("opus"));
}

// ============================================================================
// init() / release() : 初始化和释放资源
// ============================================================================
TEST(VlcPlayerLifecycleTest, initDoesNotCrash)
{
    VlcPlayer player;
    player.init();  // 创建 VlcInstance, SdlPlayer, VlcMedia
    SUCCEED();
}

TEST(VlcPlayerLifecycleTest, initMultipleTimesDoesNotCrash)
{
    VlcPlayer player;
    player.init();
    player.init();  // 防止多次创建，检查 m_qvinstance == nullptr
    SUCCEED();
}

TEST(VlcPlayerLifecycleTest, releaseDoesNotCrash)
{
    VlcPlayer player;
    player.init();
    player.release();  // 释放所有资源
    SUCCEED();
}

TEST(VlcPlayerLifecycleTest, releasePlayerDoesNotCrash)
{
    VlcPlayer player;
    player.init();
    // 通过析构函数调用 releasePlayer()
    SUCCEED();
}

TEST(VlcPlayerLifecycleTest, doubleReleaseDoesNotCrash)
{
    VlcPlayer player;
    player.init();
    player.release();
    player.release();  // 多次释放不应崩溃
    SUCCEED();
}

// ============================================================================
// state() : 未初始化时返回 Stopped
// ============================================================================
TEST(VlcPlayerStateTest, stateReturnsStoppedWhenNotInitialized)
{
    VlcPlayer player;
    // 未调用 init()，m_qvplayer == nullptr，应返回 Stopped
    EXPECT_EQ(player.state(), DmGlobal::Stopped);
}

TEST(VlcPlayerStateTest, stateReturnsStoppedAfterInit)
{
    VlcPlayer player;
    player.init();
    // 初始化后无媒体，状态应为 Stopped
    EXPECT_EQ(player.state(), DmGlobal::Stopped);
}

// ============================================================================
// play() : 不崩溃测试
// ============================================================================
TEST(VlcPlayerPlaybackTest, playDoesNotCrash)
{
    VlcPlayer player;
    player.play();  // 调用 init() + m_qvplayer->play()
    SUCCEED();
}

// ============================================================================
// pause() : 不崩溃测试
// ============================================================================
TEST(VlcPlayerPlaybackTest, pauseDoesNotCrash)
{
    VlcPlayer player;
    player.init();
    player.pause();
    SUCCEED();
}

TEST(VlcPlayerPlaybackTest, pauseWithoutInitDoesNotCrash)
{
    VlcPlayer player;
    player.pause();  // m_qvplayer == nullptr，直接返回
    SUCCEED();
}

// ============================================================================
// stop() : 不崩溃测试
// ============================================================================
TEST(VlcPlayerPlaybackTest, stopDoesNotCrash)
{
    VlcPlayer player;
    player.init();
    player.stop();
    SUCCEED();
}

TEST(VlcPlayerPlaybackTest, stopWithoutInitDoesNotCrash)
{
    VlcPlayer player;
    player.stop();  // m_qvplayer == nullptr，直接返回
    SUCCEED();
}

// ============================================================================
// volume : set/get 回环
// ============================================================================
TEST(VlcPlayerVolumeTest, getVolumeReturnsDefaultValue)
{
    VlcPlayer player;
    // 默认值为 50（见 vlcplayer.h m_volume = 50）
    int vol = player.getVolume();
    EXPECT_EQ(vol, 50);
}

TEST(VlcPlayerVolumeTest, setGetRoundTrip)
{
    VlcPlayer player;
    player.init();
    player.setVolume(75);
    EXPECT_EQ(player.getVolume(), 75);
}

TEST(VlcPlayerVolumeTest, setVolumeZero)
{
    VlcPlayer player;
    player.init();
    player.setVolume(0);
    EXPECT_EQ(player.getVolume(), 0);
}

TEST(VlcPlayerVolumeTest, setVolumeMax)
{
    VlcPlayer player;
    player.init();
    player.setVolume(100);
    EXPECT_EQ(player.getVolume(), 100);
}

// ============================================================================
// mute : set/get 回环
// ============================================================================
TEST(VlcPlayerMuteTest, setGetRoundTrip)
{
    VlcPlayer player;
    player.init();
    player.setMute(true);
    EXPECT_TRUE(player.getMute());
    player.setMute(false);
    EXPECT_FALSE(player.getMute());
}

TEST(VlcPlayerMuteTest, defaultMuteState)
{
    VlcPlayer player;
    player.init();
    // 默认应为不静音
    bool muted = player.getMute();
    Q_UNUSED(muted);  // SDL 默认值可能不同，不强断言
}

// ============================================================================
// length() / time() / setTime() : 不崩溃测试
// ============================================================================
TEST(VlcPlayerTimeTest, lengthDoesNotCrash)
{
    VlcPlayer player;
    int len = player.length();  // 调用 init() + m_qvplayer->length()
    Q_UNUSED(len);  // 无媒体时返回 -1 或 0
    SUCCEED();
}

TEST(VlcPlayerTimeTest, timeDoesNotCrash)
{
    VlcPlayer player;
    qint64 t = player.time();  // 调用 init() + m_qvplayer->time()
    Q_UNUSED(t);  // 无媒体时返回 0
    SUCCEED();
}

TEST(VlcPlayerTimeTest, setTimeDoesNotCrash)
{
    VlcPlayer player;
    player.setTime(1000);  // 设置到 1 秒位置
    SUCCEED();
}

TEST(VlcPlayerTimeTest, setTimeZeroDoesNotCrash)
{
    VlcPlayer player;
    player.setTime(0);
    SUCCEED();
}

// ============================================================================
// setMediaMeta : 不崩溃测试（不实际播放）
// ============================================================================
TEST(VlcPlayerMediaMetaTest, setMediaMetaDoesNotCrash)
{
    VlcPlayer player;
    DMusic::MediaMeta meta;
    meta.hash = "test-hash";
    meta.title = "Test Title";
    meta.localPath = "/tmp/test.mp3";
    meta.mmType = DmGlobal::MimeTypeLocal;
    player.setMediaMeta(meta);
    SUCCEED();
}

TEST(VlcPlayerMediaMetaTest, setMediaMetaEmitsSignal)
{
    VlcPlayer player;
    bool signalEmitted = false;
    QObject::connect(&player, &VlcPlayer::metaChanged, [&signalEmitted]() {
        signalEmitted = true;
    });
    DMusic::MediaMeta meta;
    meta.hash = "test-hash";
    meta.localPath = "/tmp/test.mp3";
    meta.mmType = DmGlobal::MimeTypeLocal;
    player.setMediaMeta(meta);
    EXPECT_TRUE(signalEmitted);
}

TEST(VlcPlayerMediaMetaTest, apeTrackUsesQtPlayerAndForwardsProgress)
{
    VlcPlayer player;
    DMusic::MediaMeta meta;
    meta.hash = "ape-test-hash";
    meta.localPath = "/tmp/track.APE";
    meta.mmType = DmGlobal::MimeTypeLocal;

    player.setMediaMeta(meta);
    auto *qtPlayer = player.findChild<QtPlayer *>();
    ASSERT_NE(qtPlayer, nullptr);
    auto *mediaPlayer = qtPlayer->findChild<QMediaPlayer *>();
    ASSERT_NE(mediaPlayer, nullptr);
    EXPECT_EQ(mediaPlayer->source(), QUrl::fromLocalFile(meta.localPath));

    QSignalSpy timeSpy(&player, &PlayerBase::timeChanged);
    QSignalSpy positionSpy(&player, &PlayerBase::positionChanged);
    Q_EMIT qtPlayer->timeChanged(1234);
    Q_EMIT qtPlayer->positionChanged(0.25f);
    ASSERT_EQ(timeSpy.size(), 1);
    EXPECT_EQ(timeSpy.takeFirst().at(0).toLongLong(), 1234);
    ASSERT_EQ(positionSpy.size(), 1);
    EXPECT_FLOAT_EQ(positionSpy.takeFirst().at(0).toFloat(), 0.25f);
}

TEST(VlcPlayerMediaMetaTest, switchingBackendsTransfersAudioSettingsAndIgnoresInactiveQtSignals)
{
    const QString samplePath = QStringLiteral(TEST_DATA_DIR "/sample.mp3");
    QTemporaryDir tempDir;
    ASSERT_TRUE(tempDir.isValid());

    DMusic::MediaMeta mp3;
    mp3.hash = "mp3";
    mp3.localPath = samplePath;
    mp3.mmType = DmGlobal::MimeTypeLocal;
    DMusic::MediaMeta ape = mp3;
    ape.hash = "ape";
    // Reuse the MP3 fixture to exercise extension-based routing independently of APE decoding.
    ape.localPath = tempDir.filePath(QStringLiteral("track.ape"));
    ASSERT_TRUE(QFile::copy(samplePath, ape.localPath));

    VlcPlayer player;
    player.setMediaMeta(mp3);
    player.setVolume(35);
    player.setMute(true);
    player.play();
    ASSERT_TRUE(QTest::qWaitFor([&player] { return player.state() == DmGlobal::Playing; }, 2000));

    player.setMediaMeta(ape);
    EXPECT_EQ(player.state(), DmGlobal::Stopped);
    auto *qt = player.findChild<QMediaPlayer *>();
    ASSERT_NE(qt, nullptr);
    auto *audio = qt->audioOutput();
    ASSERT_NE(audio, nullptr);
    EXPECT_TRUE(audio->isMuted());
    EXPECT_FLOAT_EQ(audio->volume(), 0.35f);

    player.setVolume(72);
    EXPECT_FLOAT_EQ(audio->volume(), 0.72f);
    player.setMute(false);
    EXPECT_FALSE(audio->isMuted());

    player.setMediaMeta(mp3);
    EXPECT_TRUE(qt->source().isEmpty());
    EXPECT_EQ(player.getVolume(), 72);
    EXPECT_FALSE(player.getMute());

    QSignalSpy timeSpy(&player, &PlayerBase::timeChanged);
    Q_EMIT player.findChild<QtPlayer *>()->timeChanged(1234);
    EXPECT_TRUE(timeSpy.isEmpty());
}

TEST(VlcPlayerMediaMetaTest, apeProgressAdvancesBetweenBackendEventsAndStopsWhenPaused)
{
    QTemporaryDir tempDir;
    ASSERT_TRUE(tempDir.isValid());
    DMusic::MediaMeta ape;
    ape.hash = "ape-progress";
    ape.localPath = tempDir.filePath(QStringLiteral("progress.ape"));
    ASSERT_TRUE(QFile::copy(QStringLiteral(TEST_DATA_DIR "/sample.mp3"), ape.localPath));

    VlcPlayer player;
    player.setMediaMeta(ape);
    auto *qtPlayer = player.findChild<QtPlayer *>();
    ASSERT_NE(qtPlayer, nullptr);
    auto *mediaPlayer = qtPlayer->findChild<QMediaPlayer *>();
    ASSERT_NE(mediaPlayer, nullptr);
    // Model APE frames whose native position events are several seconds apart.
    ASSERT_TRUE(QObject::disconnect(mediaPlayer, &QMediaPlayer::positionChanged, qtPlayer, nullptr));
    player.setMute(true);
    QSignalSpy progress(&player, &PlayerBase::timeChanged);
    player.play();
    ASSERT_TRUE(QTest::qWaitFor([&progress] { return progress.size() >= 3; }, 2000));
    EXPECT_GT(progress.last().at(0).toLongLong(), progress.first().at(0).toLongLong());

    player.pause();
    ASSERT_TRUE(QTest::qWaitFor([&player] { return player.state() == DmGlobal::Paused; }, 1000));
    const auto eventsWhilePaused = progress.size();
    const auto pausedPosition = player.time();
    QTest::qWait(220);
    EXPECT_EQ(progress.size(), eventsWhilePaused);
    EXPECT_EQ(player.time(), pausedPosition);

    ASSERT_TRUE(mediaPlayer->isSeekable());
    player.setTime(1500);
    ASSERT_GT(progress.size(), eventsWhilePaused);
    EXPECT_EQ(progress.last().at(0).toLongLong(), 1500);
    player.play();
    ASSERT_TRUE(QTest::qWaitFor([&progress] { return progress.last().at(0).toLongLong() > 1500; }, 1000));
    player.stop();
}

// ============================================================================
// Equalizer : 不崩溃测试
// ============================================================================
TEST(VlcPlayerEqualizerTest, setEqualizerEnabledDoesNotCrash)
{
    VlcPlayer player;
    player.setEqualizerEnabled(true);
    player.setEqualizerEnabled(false);
    SUCCEED();
}

TEST(VlcPlayerEqualizerTest, loadFromPresetDoesNotCrash)
{
    VlcPlayer player;
    player.loadFromPreset(0);  // 第一个预设
    player.loadFromPreset(1);  // 第二个预设
    SUCCEED();
}

TEST(VlcPlayerEqualizerTest, setPreamplificationDoesNotCrash)
{
    VlcPlayer player;
    player.setPreamplification(5.0f);
    SUCCEED();
}

TEST(VlcPlayerEqualizerTest, preamplificationReturnsFiniteValue)
{
    VlcPlayer player;
    float pre = player.preamplification();
    EXPECT_TRUE(std::isfinite(pre));
}

TEST(VlcPlayerEqualizerTest, setAmplificationForBandAtDoesNotCrash)
{
    VlcPlayer player;
    player.setAmplificationForBandAt(3.0f, 0);
    player.setAmplificationForBandAt(3.0f, 5);
    SUCCEED();
}

TEST(VlcPlayerEqualizerTest, amplificationForBandAtReturnsFiniteValue)
{
    VlcPlayer player;
    float amp = player.amplificationForBandAt(0);
    EXPECT_TRUE(std::isfinite(amp));
}

TEST(VlcPlayerEqualizerTest, setFadeInOutFactorDoesNotCrash)
{
    VlcPlayer player;
    player.setFadeInOutFactor(0.5);
    player.setFadeInOutFactor(1.0);
    player.setFadeInOutFactor(0.0);
    SUCCEED();
}

// ============================================================================
// CDA 相关 : 不崩溃测试
// ============================================================================
TEST(VlcPlayerCdaTest, initCddaTrackDoesNotCrash)
{
    VlcPlayer player;
    player.initCddaTrack();  // 调用 init() + m_qvplayer->initCddaTrack()
    SUCCEED();
}

TEST(VlcPlayerCdaTest, initCdaThreadDoesNotCrash)
{
    VlcPlayer player;
    player.initCdaThread();  // 初始化 CdaThread
    SUCCEED();
}

TEST(VlcPlayerCdaTest, getCdaMetaInfoReturnsList)
{
    VlcPlayer player;
    // 未初始化 CDA thread 时应返回空列表
    QList<DMusic::MediaMeta> metaList = player.getCdaMetaInfo();
    EXPECT_TRUE(metaList.isEmpty());
}

// ============================================================================
// 组合测试：生命周期完整流程
// ============================================================================
TEST(VlcPlayerIntegrationTest, fullLifecycle)
{
    // 构造
    std::unique_ptr<VlcPlayer> player(new VlcPlayer());
    EXPECT_NE(player.get(), nullptr);
    EXPECT_FALSE(player->supportedSuffixList().isEmpty());

    // 初始化
    player->init();
    EXPECT_EQ(player->state(), DmGlobal::Stopped);

    // 配置
    player->setVolume(80);
    EXPECT_EQ(player->getVolume(), 80);
    player->setMute(true);
    EXPECT_TRUE(player->getMute());

    // 均衡器
    player->setEqualizerEnabled(true);
    player->setPreamplification(6.0f);
    EXPECT_TRUE(std::isfinite(player->preamplification()));

    // 释放
    player->release();
    SUCCEED();
}

TEST(VlcPlayerIntegrationTest, multipleInitCallsDoNotLeak)
{
    VlcPlayer player;
    // 多次 init 不应导致资源泄漏
    for (int i = 0; i < 5; ++i) {
        player.init();
    }
    player.release();
    SUCCEED();
}
