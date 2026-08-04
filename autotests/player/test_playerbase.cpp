// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
// Graph-driven unit tests for src/libdmusic/player/playerbase.h (class PlayerBase).
// Uses the FakePlayer concrete subclass (common/fake_player.h) to exercise the
// non-pure virtual methods and inline getters/setters defined in playerbase.h.
#include <gtest/gtest.h>
#include <QList>
#include "global.h"
#include "player/playerbase.h"
#include "fake_player.h"

TEST(PlayerBaseTest, ConstructFakePlayer_NoThrow) {
    FakePlayer fp;
    SUCCEED();
}
TEST(PlayerBaseTest, SupportedSuffixList_FakePlayerAdvertisesMp3) {
    FakePlayer fp;
    EXPECT_TRUE(fp.supportedSuffixList().contains("mp3"));
}
TEST(PlayerBaseTest, GetMediaMeta_DefaultEmpty) {
    FakePlayer fp;
    DMusic::MediaMeta m = fp.getMediaMeta();
    EXPECT_TRUE(m.hash.isEmpty());
}
TEST(PlayerBaseTest, SetMediaMeta_Roundtrip) {
    FakePlayer fp;
    DMusic::MediaMeta m; m.hash = "h1";
    fp.setMediaMeta(m);
    EXPECT_EQ(fp.getMediaMeta().hash, QString("h1"));
}
TEST(PlayerBaseTest, SetGetVolume_Roundtrip) {
    FakePlayer fp;
    fp.setVolume(77);
    EXPECT_EQ(fp.getVolume(), 77);
}
TEST(PlayerBaseTest, GetMute_DefaultFalse) {
    FakePlayer fp;
    EXPECT_FALSE(fp.getMute());
}
TEST(PlayerBaseTest, State_DefaultStopped) {
    FakePlayer fp;
    EXPECT_EQ(fp.state(), DmGlobal::Stopped);
}
TEST(PlayerBaseTest, InitCddaTrack_NoThrow) {
    FakePlayer fp;
    EXPECT_NO_THROW(fp.initCddaTrack());
}
TEST(PlayerBaseTest, GetCdaMetaInfo_DefaultEmpty) {
    FakePlayer fp;
    EXPECT_TRUE(fp.getCdaMetaInfo().isEmpty());
}
TEST(PlayerBaseTest, EqualizerDefaults_NoThrow) {
    FakePlayer fp;
    EXPECT_NO_THROW(fp.setEqualizerEnabled(true));
    EXPECT_NO_THROW(fp.loadFromPreset(0));
    EXPECT_NO_THROW(fp.setPreamplification(1.5f));
    EXPECT_NO_THROW(fp.setAmplificationForBandAt(2.0f, 1));
    EXPECT_FLOAT_EQ(fp.amplificationForBandAt(1), 1.0f);
    EXPECT_FLOAT_EQ(fp.preamplification(), 1.0f);
}
