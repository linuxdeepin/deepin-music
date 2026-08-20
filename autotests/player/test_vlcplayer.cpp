// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
// Graph-driven unit tests for src/libdmusic/player/vlcplayer.h (VlcPlayer).
#include <gtest/gtest.h>
#include "global.h"
#include "player/vlcplayer.h"

class VlcPlayerTest : public ::testing::Test {
protected:
    void SetUp() override { p = new VlcPlayer; }
    void TearDown() override { if (p) { p->release(); delete p; } p = nullptr; }
    VlcPlayer *p = nullptr;
};

TEST_F(VlcPlayerTest, Lifecycle_Construct_NoThrow) { ASSERT_NE(p, nullptr); }
TEST_F(VlcPlayerTest, Init_NoThrow) { EXPECT_NO_THROW(p->init()); }
TEST_F(VlcPlayerTest, State_AfterInit_NoThrow) { p->init(); EXPECT_NO_THROW((void)p->state()); }
TEST_F(VlcPlayerTest, Length_AfterInit_NoThrow) { p->init(); EXPECT_NO_THROW((void)p->length()); }
TEST_F(VlcPlayerTest, Time_AfterInit_NoThrow) { p->init(); EXPECT_NO_THROW((void)p->time()); }
TEST_F(VlcPlayerTest, SetTime_NoThrow) { p->init(); EXPECT_NO_THROW(p->setTime(1000)); }
TEST_F(VlcPlayerTest, SetVolume_NoThrow) { p->init(); EXPECT_NO_THROW(p->setVolume(50)); }
TEST_F(VlcPlayerTest, GetVolume_AfterInit_NoThrow) { p->init(); EXPECT_NO_THROW((void)p->getVolume()); }
TEST_F(VlcPlayerTest, SetMute_NoThrow) { p->init(); EXPECT_NO_THROW(p->setMute(true)); }
TEST_F(VlcPlayerTest, GetMute_AfterInit_NoThrow) { p->init(); EXPECT_NO_THROW((void)p->getMute()); }
TEST_F(VlcPlayerTest, SetMediaMeta_NoThrow) { p->init(); DMusic::MediaMeta m; EXPECT_NO_THROW(p->setMediaMeta(m)); }
TEST_F(VlcPlayerTest, SetFadeInOutFactor_NoThrow) { p->init(); EXPECT_NO_THROW(p->setFadeInOutFactor(0.5)); }
TEST_F(VlcPlayerTest, EqualizerMethods_NoThrow) {
    p->init();
    EXPECT_NO_THROW(p->setEqualizerEnabled(true));
    EXPECT_NO_THROW(p->loadFromPreset(0));
    EXPECT_NO_THROW(p->setPreamplification(1.0f));
    EXPECT_NO_THROW(p->setAmplificationForBandAt(2.0f, 1));
    EXPECT_NO_THROW((void)p->amplificationForBandAt(1));
    EXPECT_NO_THROW((void)p->preamplification());
}
TEST_F(VlcPlayerTest, InitCddaTrack_NoThrow) { p->init(); EXPECT_NO_THROW(p->initCddaTrack()); }
TEST_F(VlcPlayerTest, GetCdaMetaInfo_NoThrow) { p->init(); EXPECT_NO_THROW((void)p->getCdaMetaInfo()); }
TEST_F(VlcPlayerTest, Play_NoMedia_NoThrow) { p->init(); EXPECT_NO_THROW(p->play()); }
TEST_F(VlcPlayerTest, Pause_NoThrow) { p->init(); EXPECT_NO_THROW(p->pause()); }
TEST_F(VlcPlayerTest, Stop_NoThrow) { p->init(); EXPECT_NO_THROW(p->stop()); }
TEST_F(VlcPlayerTest, Release_NoThrow) { p->init(); EXPECT_NO_THROW(p->release()); }
