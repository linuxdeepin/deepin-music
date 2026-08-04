// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
// Graph-driven unit tests for src/libdmusic/player/vlc/sdlplayer.h (SdlPlayer).
#include <gtest/gtest.h>
#include <QStringList>
#include "global.h"
#include "vlc/Instance.h"
#include "vlc/MediaPlayer.h"
#include "vlc/sdlplayer.h"

class SdlPlayerTest : public ::testing::Test {
protected:
    void SetUp() override {
        QStringList args{"--ignore-config", "--no-video", "--no-audio"};
        inst = new VlcInstance(args);
        p = new SdlPlayer(inst);
    }
    void TearDown() override { delete p; p = nullptr; delete inst; inst = nullptr; }
    VlcInstance *inst = nullptr;
    SdlPlayer *p = nullptr;
};

TEST_F(SdlPlayerTest, Lifecycle_ConstructDestruct_NoThrow) { ASSERT_NE(p, nullptr); }
TEST_F(SdlPlayerTest, SetCurMeta_GetCurMeta_Roundtrip) {
    DMusic::MediaMeta m; m.hash = "s1";
    p->setCurMeta(m);
    EXPECT_EQ(p->getCurMeta().hash, QString("s1"));
}
TEST_F(SdlPlayerTest, SetTime_NoThrow) { EXPECT_NO_THROW(p->setTime(1000)); }
TEST_F(SdlPlayerTest, SetVolume_NoThrow) { EXPECT_NO_THROW(p->setVolume(40)); }
TEST_F(SdlPlayerTest, SetMute_NoThrow) { EXPECT_NO_THROW(p->setMute(true)); }
TEST_F(SdlPlayerTest, GetVolume_NoThrow) { EXPECT_NO_THROW((void)p->getVolume()); }
TEST_F(SdlPlayerTest, GetMute_NoThrow) { EXPECT_NO_THROW((void)p->getMute()); }
TEST_F(SdlPlayerTest, SetProgressTag_NoThrow) { EXPECT_NO_THROW(p->setProgressTag(1)); }
TEST_F(SdlPlayerTest, CheckDataZero_NoThrow) { EXPECT_NO_THROW(p->checkDataZero()); }
TEST_F(SdlPlayerTest, Play_NoMedia_NoThrow) { EXPECT_NO_THROW(p->play()); }
TEST_F(SdlPlayerTest, Pause_NoThrow) { EXPECT_NO_THROW(p->pause()); }
TEST_F(SdlPlayerTest, Resume_NoThrow) { EXPECT_NO_THROW(p->resume()); }
TEST_F(SdlPlayerTest, Stop_NoThrow) { EXPECT_NO_THROW(p->stop()); }
