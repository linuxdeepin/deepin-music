// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
// Graph-driven unit tests for src/libdmusic/player/vlc/MediaPlayer.h (VlcMediaPlayer).
#include <gtest/gtest.h>
#include <QStringList>
#include "vlc/Instance.h"
#include "vlc/MediaPlayer.h"
#include "vlc/Media.h"

class VlcMediaPlayerTest : public ::testing::Test {
protected:
    void SetUp() override {
        QStringList args{"--ignore-config", "--no-video"};
        inst = new VlcInstance(args);
        mp = new VlcMediaPlayer(inst);
    }
    void TearDown() override { delete mp; mp = nullptr; delete inst; inst = nullptr; }
    VlcInstance *inst = nullptr;
    VlcMediaPlayer *mp = nullptr;
};

TEST_F(VlcMediaPlayerTest, Lifecycle_ConstructDestruct_NoThrow) { ASSERT_NE(mp, nullptr); }
TEST_F(VlcMediaPlayerTest, Core_ReturnsNonNull) { EXPECT_NE(mp->core(), nullptr); }
TEST_F(VlcMediaPlayerTest, Equalizer_ReturnsNonNull) { EXPECT_NE(mp->equalizer(), nullptr); }
TEST_F(VlcMediaPlayerTest, Length_NoMedia_NoThrow) { EXPECT_NO_THROW((void)mp->length()); }
TEST_F(VlcMediaPlayerTest, Time_NoMedia_NoThrow) { EXPECT_NO_THROW((void)mp->time()); }
TEST_F(VlcMediaPlayerTest, State_NoMedia_NoThrow) { EXPECT_NO_THROW((void)mp->state()); }
TEST_F(VlcMediaPlayerTest, Position_NoMedia_NoThrow) { EXPECT_NO_THROW((void)mp->position()); }
TEST_F(VlcMediaPlayerTest, GetVolume_NoThrow) { EXPECT_NO_THROW((void)mp->getVolume()); }
TEST_F(VlcMediaPlayerTest, GetMute_NoThrow) { EXPECT_NO_THROW((void)mp->getMute()); }
TEST_F(VlcMediaPlayerTest, SetTime_NoThrow) { EXPECT_NO_THROW(mp->setTime(1000)); }
TEST_F(VlcMediaPlayerTest, SetVolume_NoThrow) { EXPECT_NO_THROW(mp->setVolume(50)); }
TEST_F(VlcMediaPlayerTest, SetMute_NoThrow) { EXPECT_NO_THROW(mp->setMute(true)); }
TEST_F(VlcMediaPlayerTest, SetPosition_NoThrow) { EXPECT_NO_THROW(mp->setPosition(0.5f)); }
TEST_F(VlcMediaPlayerTest, InitCddaTrack_NoThrow) { EXPECT_NO_THROW(mp->initCddaTrack()); }
TEST_F(VlcMediaPlayerTest, Play_NoMedia_NoThrow) { EXPECT_NO_THROW(mp->play()); }
TEST_F(VlcMediaPlayerTest, Pause_NoThrow) { EXPECT_NO_THROW(mp->pause()); }
TEST_F(VlcMediaPlayerTest, Resume_NoThrow) { EXPECT_NO_THROW(mp->resume()); }
TEST_F(VlcMediaPlayerTest, Stop_NoThrow) { EXPECT_NO_THROW(mp->stop()); }
TEST_F(VlcMediaPlayerTest, Open_Media_NoThrow) {
    VlcMedia media;
    media.initMedia("/tmp/no-such-file.mp3", true, inst);
    EXPECT_NO_THROW(mp->open(&media));
}
