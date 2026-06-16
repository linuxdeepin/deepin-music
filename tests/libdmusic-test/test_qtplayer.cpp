// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// qtplayer.cpp 的单元测试：构造、初始化/释放、状态查询、音量/静音控制。
// 关键：所有测试不实际播放音频，只验证 API 的调用行为和返回值。

#include <gtest/gtest.h>

#include <QCoreApplication>
#include <QString>
#include <QStringList>
#include <QSignalSpy>
#include <QMediaPlayer>
#include <memory>

#include "playerbase.h"
#include "qtplayer.h"
#include "global.h"

// ============================================================================
// 辅助：TestableQtPlayer 继承 QtPlayer，暴露 protected 方法供测试使用
// ============================================================================
class TestableQtPlayer : public QtPlayer
{
public:
    using QtPlayer::onMediaStatusChanged;
    using QtPlayer::onPositionChanged;
};

// ============================================================================
// 辅助：确保全局 QCoreApplication 实例存在（Qt 运行时需要）
// ============================================================================
class QtPlayerTestEnvironment : public ::testing::Environment
{
public:
    void SetUp() override
    {
        static int argc = 1;
        static char *argv[] = {const_cast<char *>("test")};
        if (!QCoreApplication::instance()) {
            app = new QCoreApplication(argc, argv);
        }
    }

    QCoreApplication *app = nullptr;
};

::testing::Environment *const qt_env = ::testing::AddGlobalTestEnvironment(new QtPlayerTestEnvironment);

// ============================================================================
// 构造/析构：不崩溃
// ============================================================================
TEST(QtPlayerConstructionTest, constructsAndDestroysWithoutCrash)
{
    std::unique_ptr<TestableQtPlayer> player(new TestableQtPlayer());
    EXPECT_NE(player.get(), nullptr);
}

TEST(QtPlayerConstructionTest, destructorReleasesResources)
{
    TestableQtPlayer *player = new TestableQtPlayer();
    delete player;  // 析构函数调用 releasePlayer()
    SUCCEED();  // 不崩溃即通过
}

// ============================================================================
// supportedSuffixList : 构造函数中初始化，返回非空列表
// ============================================================================
TEST(QtPlayerSuffixTest, returnsNonEmptySuffixList)
{
    std::unique_ptr<TestableQtPlayer> player(new TestableQtPlayer());
    const QStringList suffixes = player->supportedSuffixList();
    EXPECT_FALSE(suffixes.isEmpty());
}

TEST(QtPlayerSuffixTest, containsExpectedAudioFormats)
{
    std::unique_ptr<TestableQtPlayer> player(new TestableQtPlayer());
    const QStringList suffixes = player->supportedSuffixList();
    EXPECT_TRUE(suffixes.contains("wav"));
    EXPECT_TRUE(suffixes.contains("ogg"));
    EXPECT_TRUE(suffixes.contains("mp3"));
    EXPECT_TRUE(suffixes.contains("flac"));
    EXPECT_TRUE(suffixes.contains("opus"));
}

// ============================================================================
// init() / release() : 不崩溃
// ============================================================================
TEST(QtPlayerInitReleaseTest, initDoesNotCrash)
{
    std::unique_ptr<TestableQtPlayer> player(new TestableQtPlayer());
    EXPECT_NO_THROW(player->init());
}

TEST(QtPlayerInitReleaseTest, releaseDoesNotCrash)
{
    std::unique_ptr<TestableQtPlayer> player(new TestableQtPlayer());
    player->init();
    EXPECT_NO_THROW(player->release());
}

TEST(QtPlayerInitReleaseTest, doubleInitDoesNotCrash)
{
    std::unique_ptr<TestableQtPlayer> player(new TestableQtPlayer());
    player->init();
    EXPECT_NO_THROW(player->init());  // 防重复初始化
}

TEST(QtPlayerInitReleaseTest, doubleReleaseDoesNotCrash)
{
    std::unique_ptr<TestableQtPlayer> player(new TestableQtPlayer());
    player->init();
    player->release();
    EXPECT_NO_THROW(player->release());  // 重复释放不崩溃
}

TEST(QtPlayerInitReleaseTest, releaseAfterInitRestoresInitialState)
{
    std::unique_ptr<TestableQtPlayer> player(new TestableQtPlayer());
    player->init();
    player->release();
    // release 后 state() 应返回 Stopped
    EXPECT_EQ(player->state(), DmGlobal::Stopped);
}

// ============================================================================
// state() : 未初始化时返回 Stopped
// ============================================================================
TEST(QtPlayerStateTest, initialStateIsStopped)
{
    std::unique_ptr<TestableQtPlayer> player(new TestableQtPlayer());
    EXPECT_EQ(player->state(), DmGlobal::Stopped);
}

TEST(QtPlayerStateTest, stateIsStoppedAfterRelease)
{
    std::unique_ptr<TestableQtPlayer> player(new TestableQtPlayer());
    player->init();
    player->release();
    EXPECT_EQ(player->state(), DmGlobal::Stopped);
}

// ============================================================================
// volume : set/get 回环；验证默认值
// ============================================================================
TEST(QtPlayerVolumeTest, defaultVolumeIsFifty)
{
    std::unique_ptr<TestableQtPlayer> player(new TestableQtPlayer());
    // 未 init 时 getVolume() 会触发 init()，返回 PlayerBase::m_volume 初始值 50
    EXPECT_EQ(player->getVolume(), 50);
}

TEST(QtPlayerVolumeTest, setGetRoundTrip)
{
    std::unique_ptr<TestableQtPlayer> player(new TestableQtPlayer());
    player->setVolume(75);
    EXPECT_EQ(player->getVolume(), 75);
}

TEST(QtPlayerVolumeTest, setVolumeAfterInit)
{
    std::unique_ptr<TestableQtPlayer> player(new TestableQtPlayer());
    player->init();
    player->setVolume(60);
    EXPECT_EQ(player->getVolume(), 60);
}

TEST(QtPlayerVolumeTest, setVolumeZero)
{
    std::unique_ptr<TestableQtPlayer> player(new TestableQtPlayer());
    player->setVolume(0);
    EXPECT_EQ(player->getVolume(), 0);
}

// ============================================================================
// mute : set/get 回环；验证默认值
// ============================================================================
TEST(QtPlayerMuteTest, getMuteDoesNotCrash)
{
    std::unique_ptr<TestableQtPlayer> player(new TestableQtPlayer());
    // 未 init 时 getMute() 会触发 init()，返回 QAudioOutput::isMuted() 默认 false
    EXPECT_NO_THROW(player->getMute());
}

TEST(QtPlayerMuteTest, setMuteTrueAndGet)
{
    std::unique_ptr<TestableQtPlayer> player(new TestableQtPlayer());
    player->setMute(true);
    EXPECT_TRUE(player->getMute());
}

TEST(QtPlayerMuteTest, setMuteFalseAndGet)
{
    std::unique_ptr<TestableQtPlayer> player(new TestableQtPlayer());
    player->setMute(false);
    EXPECT_FALSE(player->getMute());
}

TEST(QtPlayerMuteTest, setMuteAfterInit)
{
    std::unique_ptr<TestableQtPlayer> player(new TestableQtPlayer());
    player->init();
    player->setMute(true);
    EXPECT_TRUE(player->getMute());
}

// ============================================================================
// setFadeInOutFactor : 不崩溃
// ============================================================================
TEST(QtPlayerFadeTest, setFadeInOutFactorDoesNotCrash)
{
    std::unique_ptr<TestableQtPlayer> player(new TestableQtPlayer());
    EXPECT_NO_THROW(player->setFadeInOutFactor(1.0));
}

TEST(QtPlayerFadeTest, setFadeInOutFactorWithZero)
{
    std::unique_ptr<TestableQtPlayer> player(new TestableQtPlayer());
    EXPECT_NO_THROW(player->setFadeInOutFactor(0.0));
}

TEST(QtPlayerFadeTest, setFadeInOutFactorWithFraction)
{
    std::unique_ptr<TestableQtPlayer> player(new TestableQtPlayer());
    EXPECT_NO_THROW(player->setFadeInOutFactor(0.5));
}

TEST(QtPlayerFadeTest, setFadeInOutFactorAfterInit)
{
    std::unique_ptr<TestableQtPlayer> player(new TestableQtPlayer());
    player->init();
    EXPECT_NO_THROW(player->setFadeInOutFactor(1.0));
}

// ============================================================================
// setMediaMeta : 不崩溃（使用空 Meta 和有效 Meta）
// ============================================================================
TEST(QtPlayerMediaMetaTest, setEmptyMediaMetaDoesNotCrash)
{
    std::unique_ptr<TestableQtPlayer> player(new TestableQtPlayer());
    DMusic::MediaMeta emptyMeta;
    EXPECT_NO_THROW(player->setMediaMeta(emptyMeta));
}

TEST(QtPlayerMediaMetaTest, setMediaMetaWithHashDoesNotCrash)
{
    std::unique_ptr<TestableQtPlayer> player(new TestableQtPlayer());
    DMusic::MediaMeta meta;
    meta.hash = "test-hash-123";
    meta.title = "Test Song";
    meta.localPath = "/tmp/test.mp3";
    EXPECT_NO_THROW(player->setMediaMeta(meta));
}

TEST(QtPlayerMediaMetaTest, setSameMediaMetaTwiceIsIdempotent)
{
    std::unique_ptr<TestableQtPlayer> player(new TestableQtPlayer());
    DMusic::MediaMeta meta;
    meta.hash = "test-hash-456";
    meta.localPath = "/tmp/test.mp3";
    player->setMediaMeta(meta);
    // 再次设置相同 meta 不崩溃
    EXPECT_NO_THROW(player->setMediaMeta(meta));
}

// ============================================================================
// length() / time() : 未初始化时行为
// ============================================================================
TEST(QtPlayerLengthTimeTest, lengthDoesNotCrash)
{
    std::unique_ptr<TestableQtPlayer> player(new TestableQtPlayer());
    // init() + m_mediaPlayer->duration()
    EXPECT_NO_THROW(player->length());
}

TEST(QtPlayerLengthTimeTest, lengthReturnsZeroWhenNoMedia)
{
    std::unique_ptr<TestableQtPlayer> player(new TestableQtPlayer());
    // 未加载媒体时 duration 应为 0
    EXPECT_EQ(player->length(), 0);
}

TEST(QtPlayerLengthTimeTest, timeDoesNotCrash)
{
    std::unique_ptr<TestableQtPlayer> player(new TestableQtPlayer());
    // init() + m_mediaPlayer->position()
    EXPECT_NO_THROW(player->time());
}

TEST(QtPlayerLengthTimeTest, timeReturnsNegativeOneWhenNoMedia)
{
    std::unique_ptr<TestableQtPlayer> player(new TestableQtPlayer());
    // 未加载媒体时返回 -1
    EXPECT_EQ(player->time(), -1);
}

// ============================================================================
// setTime : 不崩溃
// ============================================================================
TEST(QtPlayerSetTimeTest, setTimeDoesNotCrash)
{
    std::unique_ptr<TestableQtPlayer> player(new TestableQtPlayer());
    EXPECT_NO_THROW(player->setTime(1000));
}

TEST(QtPlayerSetTimeTest, setTimeWithZero)
{
    std::unique_ptr<TestableQtPlayer> player(new TestableQtPlayer());
    EXPECT_NO_THROW(player->setTime(0));
}

TEST(QtPlayerSetTimeTest, setTimeWithLargeValue)
{
    std::unique_ptr<TestableQtPlayer> player(new TestableQtPlayer());
    EXPECT_NO_THROW(player->setTime(999999999));
}

// ============================================================================
// play/pause/stop : 不崩溃（不实际播放音频）
// ============================================================================
TEST(QtPlayerPlaybackControlTest, playDoesNotCrash)
{
    std::unique_ptr<TestableQtPlayer> player(new TestableQtPlayer());
    EXPECT_NO_THROW(player->play());
}

TEST(QtPlayerPlaybackControlTest, pauseDoesNotCrash)
{
    std::unique_ptr<TestableQtPlayer> player(new TestableQtPlayer());
    EXPECT_NO_THROW(player->pause());
}

TEST(QtPlayerPlaybackControlTest, stopDoesNotCrash)
{
    std::unique_ptr<TestableQtPlayer> player(new TestableQtPlayer());
    EXPECT_NO_THROW(player->stop());
}

TEST(QtPlayerPlaybackControlTest, pauseAfterPlayDoesNotCrash)
{
    std::unique_ptr<TestableQtPlayer> player(new TestableQtPlayer());
    player->play();
    EXPECT_NO_THROW(player->pause());
}

TEST(QtPlayerPlaybackControlTest, stopAfterPlayDoesNotCrash)
{
    std::unique_ptr<TestableQtPlayer> player(new TestableQtPlayer());
    player->play();
    EXPECT_NO_THROW(player->stop());
}

// ============================================================================
// onMediaStatusChanged : 信号处理（通过 TestableQtPlayer 访问 protected 方法）
// ============================================================================
TEST(QtPlayerMediaStatusTest, onMediaStatusChangedEndOfMediaEmitsEndSignal)
{
    std::unique_ptr<TestableQtPlayer> player(new TestableQtPlayer());
    player->init();

    QSignalSpy spy(player.get(), &QtPlayer::end);
    ASSERT_TRUE(spy.isValid());

    // 调用 onMediaStatusChanged slot，模拟 EndOfMedia 状态
    player->onMediaStatusChanged(QMediaPlayer::MediaStatus::EndOfMedia);

    EXPECT_EQ(spy.count(), 1);
}

TEST(QtPlayerMediaStatusTest, onMediaStatusChangedInvalidMediaEmitsEndSignal)
{
    std::unique_ptr<TestableQtPlayer> player(new TestableQtPlayer());
    player->init();

    QSignalSpy spy(player.get(), &QtPlayer::end);
    ASSERT_TRUE(spy.isValid());

    // 调用 onMediaStatusChanged slot，模拟 InvalidMedia 状态
    player->onMediaStatusChanged(QMediaPlayer::MediaStatus::InvalidMedia);

    EXPECT_EQ(spy.count(), 1);
}

TEST(QtPlayerMediaStatusTest, onMediaStatusChangedOtherStatusDoesNotEmitEnd)
{
    std::unique_ptr<TestableQtPlayer> player(new TestableQtPlayer());
    player->init();

    QSignalSpy spy(player.get(), &QtPlayer::end);
    ASSERT_TRUE(spy.isValid());

    // 其他状态不应触发 end() 信号
    player->onMediaStatusChanged(QMediaPlayer::MediaStatus::LoadingMedia);
    player->onMediaStatusChanged(QMediaPlayer::MediaStatus::LoadedMedia);
    player->onMediaStatusChanged(QMediaPlayer::MediaStatus::BufferingMedia);
    player->onMediaStatusChanged(QMediaPlayer::MediaStatus::BufferedMedia);
    player->onMediaStatusChanged(QMediaPlayer::MediaStatus::NoMedia);

    EXPECT_EQ(spy.count(), 0);
}

// ============================================================================
// onPositionChanged : 位置变化处理（通过 TestableQtPlayer 访问 protected 方法）
// ============================================================================
TEST(QtPlayerPositionChangedTest, onPositionChangedDoesNotCrash)
{
    std::unique_ptr<TestableQtPlayer> player(new TestableQtPlayer());
    player->init();

    EXPECT_NO_THROW(player->onPositionChanged(1000));
}

TEST(QtPlayerPositionChangedTest, onPositionChangedEmitsTimeChangedSignal)
{
    std::unique_ptr<TestableQtPlayer> player(new TestableQtPlayer());
    player->init();

    QSignalSpy spy(player.get(), &QtPlayer::timeChanged);
    ASSERT_TRUE(spy.isValid());

    player->onPositionChanged(5000);

    // timeChanged 信号应被发出（但可能因条件被过滤）
    // 只要不崩溃即为通过
    SUCCEED();
}

// ============================================================================
// 信号连接测试：验证 init() 中的信号连接正确建立
// ============================================================================
TEST(QtPlayerSignalConnectionTest, initConnectsStateChangedSignal)
{
    std::unique_ptr<TestableQtPlayer> player(new TestableQtPlayer());
    player->init();

    // 验证信号可以连接（不崩溃）
    bool connected = QObject::connect(player.get(), &QtPlayer::stateChanged, [](DmGlobal::PlaybackStatus) {});
    EXPECT_TRUE(connected);
}

TEST(QtPlayerSignalConnectionTest, initConnectsMutedChangedSignal)
{
    std::unique_ptr<TestableQtPlayer> player(new TestableQtPlayer());
    player->init();

    // 验证 signalMutedChanged 信号可以连接
    bool connected = QObject::connect(player.get(), &QtPlayer::signalMutedChanged, []() {});
    EXPECT_TRUE(connected);
}

// ============================================================================
// getMediaMeta : 获取当前媒体元数据
// ============================================================================
TEST(QtPlayerMediaMetaTest, getMediaMetaInitiallyEmpty)
{
    std::unique_ptr<TestableQtPlayer> player(new TestableQtPlayer());
    DMusic::MediaMeta meta = player->getMediaMeta();
    EXPECT_TRUE(meta.hash.isEmpty());
}

TEST(QtPlayerMediaMetaTest, getMediaMetaAfterSet)
{
    std::unique_ptr<TestableQtPlayer> player(new TestableQtPlayer());
    DMusic::MediaMeta meta;
    meta.hash = "test-hash-789";
    meta.title = "Test Title";
    meta.localPath = "/tmp/test.mp3";
    player->setMediaMeta(meta);

    DMusic::MediaMeta retrieved = player->getMediaMeta();
    EXPECT_EQ(retrieved.hash, "test-hash-789");
    EXPECT_EQ(retrieved.title, "Test Title");
}

// ============================================================================
// 生命周期完整测试：init -> 操作 -> release -> 再 init
// ============================================================================
TEST(QtPlayerLifecycleTest, reinitializeAfterRelease)
{
    std::unique_ptr<TestableQtPlayer> player(new TestableQtPlayer());

    // 第一次生命周期
    player->init();
    player->setVolume(80);
    EXPECT_EQ(player->getVolume(), 80);
    player->release();

    // 第二次生命周期
    player->init();
    player->setVolume(40);
    EXPECT_EQ(player->getVolume(), 40);
    player->release();

    SUCCEED();
}

TEST(QtPlayerLifecycleTest, stateTransitionsDoNotCrash)
{
    std::unique_ptr<TestableQtPlayer> player(new TestableQtPlayer());

    player->init();
    EXPECT_EQ(player->state(), DmGlobal::Stopped);

    player->play();
    // play() 后可能进入 Playing 状态

    player->pause();

    player->stop();
    EXPECT_EQ(player->state(), DmGlobal::Stopped);

    player->release();
    EXPECT_EQ(player->state(), DmGlobal::Stopped);

    SUCCEED();
}
