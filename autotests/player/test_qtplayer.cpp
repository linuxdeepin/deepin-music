// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
// Graph-driven unit tests for src/libdmusic/player/qtplayer.h (class QtPlayer).
#include <gtest/gtest.h>
#include "global.h"
#include "player/qtplayer.h"

class QtPlayerTest : public ::testing::Test {
protected:
    void SetUp() override { p = new QtPlayer; }
    void TearDown() override { if (p) { p->release(); delete p; } p = nullptr; }
    QtPlayer *p = nullptr;
};

TEST_F(QtPlayerTest, Lifecycle_Construct_NoThrow) { ASSERT_NE(p, nullptr); }
TEST_F(QtPlayerTest, Init_NoThrow) { EXPECT_NO_THROW(p->init()); }
TEST_F(QtPlayerTest, State_AfterInit_NoThrow) { p->init(); EXPECT_NO_THROW((void)p->state()); }
TEST_F(QtPlayerTest, Length_AfterInit_NoThrow) { p->init(); EXPECT_NO_THROW((void)p->length()); }
TEST_F(QtPlayerTest, Time_AfterInit_NoThrow) { p->init(); EXPECT_NO_THROW((void)p->time()); }
TEST_F(QtPlayerTest, SetTime_NoThrow) { p->init(); EXPECT_NO_THROW(p->setTime(1000)); }
TEST_F(QtPlayerTest, SetGetVolume_NoThrow) {
    p->init();
    EXPECT_NO_THROW(p->setVolume(50));
    EXPECT_NO_THROW((void)p->getVolume());
}
TEST_F(QtPlayerTest, SetMute_GetMute_NoThrow) {
    p->init();
    EXPECT_NO_THROW(p->setMute(true));
    EXPECT_NO_THROW((void)p->getMute());
}
TEST_F(QtPlayerTest, SetMediaMeta_NoThrow) { p->init(); DMusic::MediaMeta m; EXPECT_NO_THROW(p->setMediaMeta(m)); }
TEST_F(QtPlayerTest, SetFadeInOutFactor_NoThrow) { p->init(); EXPECT_NO_THROW(p->setFadeInOutFactor(0.5)); }
TEST_F(QtPlayerTest, PlayPauseStop_NoThrow) {
    p->init();
    EXPECT_NO_THROW(p->play());
    EXPECT_NO_THROW(p->pause());
    EXPECT_NO_THROW(p->stop());
}
TEST_F(QtPlayerTest, Release_NoThrow) { p->init(); EXPECT_NO_THROW(p->release()); }
