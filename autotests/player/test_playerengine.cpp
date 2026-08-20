// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
// Graph-driven unit tests for src/libdmusic/player/playerengine.h (class PlayerEngine).
// Injects a FakePlayer backend so no real VLC/SDL/Qt playback is exercised.
#include <gtest/gtest.h>
#include <QList>
#include <QString>
#include <QStringList>
#include <QCoreApplication>
#include <QThread>
#include <QEventLoop>
#include <QTimer>
#include "global.h"
#include "player/playerengine.h"
#include "fake_player.h"

static void pumpEvents(int ms) {
    QEventLoop loop;
    QTimer::singleShot(ms, &loop, &QEventLoop::quit);
    loop.exec();
}

class PlayerEngineTest : public ::testing::Test {
protected:
    void SetUp() override {
        fake = new FakePlayer;
        eng = new PlayerEngine(nullptr, fake);
        // Populate m_mprisPlayer so setMediaMeta/stop/removeMetasFromPlayList (which
        // call resetDBusMpris) do not dereference a null MprisPlayer.
        eng->setMprisPlayer("org.deepin.music", "deepin-music", "Music");
    }
    void TearDown() override { delete eng; eng = nullptr; fake = nullptr; }
    FakePlayer *fake = nullptr;
    PlayerEngine *eng = nullptr;
};

TEST_F(PlayerEngineTest, Lifecycle_ConstructDestruct_NoThrow) { ASSERT_NE(eng, nullptr); }

TEST_F(PlayerEngineTest, Getters_NoThrow) {
    EXPECT_NO_THROW((void)eng->fadeInOutFactor());
    EXPECT_NO_THROW((void)eng->supportedSuffixList());
    EXPECT_NO_THROW((void)eng->getMediaMeta());
    EXPECT_NO_THROW((void)eng->getMetas());
    EXPECT_NO_THROW((void)eng->isEmpty());
    EXPECT_NO_THROW((void)eng->length());
    EXPECT_NO_THROW((void)eng->time());
    EXPECT_NO_THROW((void)eng->getVolume());
    EXPECT_NO_THROW((void)eng->getMute());
    EXPECT_NO_THROW((void)eng->playbackStatus());
    EXPECT_NO_THROW((void)eng->getPlaybackMode());
    EXPECT_NO_THROW((void)eng->getCurrentPlayList());
    EXPECT_NO_THROW((void)eng->getCdaMetaInfo());
    EXPECT_NO_THROW((void)eng->amplificationForBandAt(1));
    EXPECT_NO_THROW((void)eng->preamplification());
}

TEST_F(PlayerEngineTest, Setters_NoThrow) {
    EXPECT_NO_THROW(eng->setFadeInOut(true));
    EXPECT_NO_THROW(eng->setVolume(60));
    EXPECT_NO_THROW(eng->setMute(true));
    EXPECT_NO_THROW(eng->setPlaybackMode(DmGlobal::RepeatAll));
    EXPECT_NO_THROW(eng->setCurrentPlayList("play"));
    EXPECT_NO_THROW(eng->setEqualizerEnabled(true));
    EXPECT_NO_THROW(eng->loadFromPreset(0));
    EXPECT_NO_THROW(eng->setPreamplification(1.0f));
    EXPECT_NO_THROW(eng->setAmplificationForBandAt(2.0f, 1));
    EXPECT_NO_THROW(eng->setFadeInOutFactor(0.5));
}

TEST_F(PlayerEngineTest, SetMediaMeta_Meta_NoThrow) { DMusic::MediaMeta m; m.hash = "h1"; EXPECT_NO_THROW(eng->setMediaMeta(m)); }
TEST_F(PlayerEngineTest, SetMediaMeta_Hash_NoThrow) { EXPECT_NO_THROW(eng->setMediaMeta(QString("h1"))); }
TEST_F(PlayerEngineTest, AddMetasToPlayList_NoThrow) { DMusic::MediaMeta m; m.hash = "h1"; EXPECT_NO_THROW(eng->addMetasToPlayList(QList<DMusic::MediaMeta>{m})); }
TEST_F(PlayerEngineTest, ReplaceMetasToPlayList_NoThrow) { DMusic::MediaMeta m; m.hash = "h1"; EXPECT_NO_THROW(eng->replaceMetasToPlayList(QList<DMusic::MediaMeta>{m})); }
TEST_F(PlayerEngineTest, HasNextPlayableMeta_NoThrow) { EXPECT_NO_THROW((void)eng->hasNextPlayableMeta("h1")); }
TEST_F(PlayerEngineTest, HasPreviousPlayableMeta_NoThrow) { EXPECT_NO_THROW((void)eng->hasPreviousPlayableMeta("h1")); }
TEST_F(PlayerEngineTest, RemoveMetaFromPlayList_NoThrow) { EXPECT_NO_THROW(eng->removeMetaFromPlayList("h1")); }
TEST_F(PlayerEngineTest, RemoveMetasFromPlayList_NoThrow) { EXPECT_NO_THROW(eng->removeMetasFromPlayList(QStringList{"h1"})); }
TEST_F(PlayerEngineTest, ClearPlayList_NoThrow) { EXPECT_NO_THROW(eng->clearPlayList(false)); }
TEST_F(PlayerEngineTest, PlayNextMeta_NoThrow) { EXPECT_NO_THROW(eng->playNextMeta(false, true)); }
TEST_F(PlayerEngineTest, SetTime_NoThrow) { EXPECT_NO_THROW(eng->setTime(1000)); }

TEST_F(PlayerEngineTest, Play_NoThrow) { EXPECT_NO_THROW(eng->play()); }
TEST_F(PlayerEngineTest, ForcePlay_NoThrow) { EXPECT_NO_THROW(eng->forcePlay()); }
TEST_F(PlayerEngineTest, Pause_NoThrow) { EXPECT_NO_THROW(eng->pause()); }
TEST_F(PlayerEngineTest, PauseNow_NoThrow) { EXPECT_NO_THROW(eng->pauseNow()); }
TEST_F(PlayerEngineTest, PlayPause_NoThrow) { EXPECT_NO_THROW(eng->playPause()); }
TEST_F(PlayerEngineTest, Resume_NoThrow) { EXPECT_NO_THROW(eng->resume()); }
TEST_F(PlayerEngineTest, PlayPreMeta_NoThrow) { EXPECT_NO_THROW(eng->playPreMeta()); }
TEST_F(PlayerEngineTest, Stop_NoThrow) { EXPECT_NO_THROW(eng->stop()); }

// setMprisPlayer binds the session D-Bus; unsafe in a headless test process.
TEST_F(PlayerEngineTest, DISABLED_SetMprisPlayer_RequiresDBus_Skipped) {
    EXPECT_NO_THROW(eng->setMprisPlayer("org.deepin.music", "deepin-music", "Music"));
}

TEST_F(PlayerEngineTest, SignalEmission_DrivesEngineLambdas_NoThrow) {
    DMusic::MediaMeta m; m.hash = "h1"; m.length = 100;
    eng->addMetasToPlayList(QList<DMusic::MediaMeta>{m});
    eng->setMediaMeta(m);
    EXPECT_NO_THROW(fake->emitMetaChanged());
    EXPECT_NO_THROW(fake->emitTimeChanged(500));
    EXPECT_NO_THROW(fake->emitPositionChanged(0.5f));
    EXPECT_NO_THROW(fake->emitStateChanged(DmGlobal::Playing));
    EXPECT_NO_THROW(fake->emitStateChanged(DmGlobal::Paused));
    EXPECT_NO_THROW(fake->emitStateChanged(DmGlobal::Stopped));
    pumpEvents(1200);
}

TEST_F(PlayerEngineTest, Navigation_PlayNextAndPrevious_NoThrow) {
    DMusic::MediaMeta m1; m1.hash = "h1"; m1.length = 100; m1.filetype = "mp3";
    m1.localPath = QString::fromUtf8(TEST_DATA_DIR) + "/sample.mp3";
    DMusic::MediaMeta m2; m2.hash = "h2"; m2.length = 100; m2.filetype = "mp3";
    m2.localPath = m1.localPath;
    eng->addMetasToPlayList(QList<DMusic::MediaMeta>{m1, m2});
    eng->setMediaMeta(m1);
    // Auto path: playable metas -> switchToNewTrackWithFade(meta) + timer lambda.
    EXPECT_NO_THROW(eng->playNextMeta(true, true));
    pumpEvents(700);
    // Manual path: requestManualNavigation -> executeManualNavigation (timer).
    EXPECT_NO_THROW(eng->playPreMeta());
    EXPECT_NO_THROW(eng->playNextMeta(false, false));
    pumpEvents(700);
    EXPECT_NO_THROW(eng->forcePlay());
    EXPECT_NO_THROW(eng->setFadeInOut(true));
    EXPECT_NO_THROW(eng->play());
    pumpEvents(1200);
}

TEST_F(PlayerEngineTest, FadePath_DrivesFadeLambdas_NoThrow) {
    DMusic::MediaMeta m1; m1.hash = "h1"; m1.filetype = "mp3"; m1.localPath = QString::fromUtf8(TEST_DATA_DIR) + "/sample.mp3";
    DMusic::MediaMeta m2; m2.hash = "h2"; m2.filetype = "mp3"; m2.localPath = m1.localPath;
    eng->addMetasToPlayList(QList<DMusic::MediaMeta>{m1, m2});
    eng->setMediaMeta(m1);
    eng->setFadeInOut(true);
    fake->setState(DmGlobal::Playing);  // playbackStatus()==Playing -> fade path
    EXPECT_NO_THROW(eng->playNextMeta(true, true));
    // Fire the 400ms fade-out + 600ms fade-in animation finished signals.
    pumpEvents(1500);
}

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>

// Drives the MprisPlayer D-Bus command lambdas connected in setMprisPlayer
// (Play/Pause/Next/Previous/Stop/Quit/Raise/OpenUri). Skips if no session bus.
TEST_F(PlayerEngineTest, MprisDbus_DrivesCommandLambdas_NoThrow) {
    auto bus = QDBusConnection::sessionBus();
    if (!bus.isConnected()) {
        GTEST_SKIP() << "No D-Bus session bus available";
    }
    const QString svc = "org.deepin.music.ut.playerengine";
    eng->setMprisPlayer(svc, "deepin-music", "Music");
    pumpEvents(200);
    auto call = [&](const QString &iface, const QString &method) {
        QDBusInterface i(svc, "/org/mpris/MediaPlayer2", iface, bus);
        if (i.isValid()) { (void)i.call(method); }
    };
    call("org.mpris.MediaPlayer2.Player", "Play");
    call("org.mpris.MediaPlayer2.Player", "Pause");
    call("org.mpris.MediaPlayer2.Player", "Next");
    call("org.mpris.MediaPlayer2.Player", "Previous");
    call("org.mpris.MediaPlayer2.Player", "Stop");
    call("org.mpris.MediaPlayer2", "Quit");
    call("org.mpris.MediaPlayer2", "Raise");
    call("org.mpris.MediaPlayer2.Player", "OpenUri");
    pumpEvents(300);
    SUCCEED();
}
