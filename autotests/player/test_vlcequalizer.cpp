// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
// Graph-driven unit tests for src/libdmusic/player/vlc/Equalizer.h (VlcEqualizer).
#include <gtest/gtest.h>
#include <QStringList>
#include "vlc/Instance.h"
#include "vlc/MediaPlayer.h"
#include "vlc/Equalizer.h"

class VlcEqualizerTest : public ::testing::Test {
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

TEST_F(VlcEqualizerTest, EqualizerFromPlayer_ReturnsNonNull) {
    EXPECT_NE(mp->equalizer(), nullptr);
}
TEST_F(VlcEqualizerTest, AmplificationForBandAt_NoThrow) {
    EXPECT_NO_THROW((void)mp->equalizer()->amplificationForBandAt(0));
}
TEST_F(VlcEqualizerTest, Preamplification_NoThrow) {
    EXPECT_NO_THROW((void)mp->equalizer()->preamplification());
}
TEST_F(VlcEqualizerTest, LoadFromPreset_NoThrow) {
    EXPECT_NO_THROW(mp->equalizer()->loadFromPreset(0));
}
TEST_F(VlcEqualizerTest, SetAmplificationForBandAt_NoThrow) {
    EXPECT_NO_THROW(mp->equalizer()->setAmplificationForBandAt(2.5f, 1));
}
TEST_F(VlcEqualizerTest, SetEnabled_NoThrow) {
    EXPECT_NO_THROW(mp->equalizer()->setEnabled(true));
    EXPECT_NO_THROW(mp->equalizer()->setEnabled(false));
}
TEST_F(VlcEqualizerTest, SetPreamplification_NoThrow) {
    EXPECT_NO_THROW(mp->equalizer()->setPreamplification(5.0f));
}
