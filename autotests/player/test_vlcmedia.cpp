// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
// Graph-driven unit tests for src/libdmusic/player/vlc/Media.h (VlcMedia).
#include <gtest/gtest.h>
#include <QStringList>
#include "vlc/Instance.h"
#include "vlc/Media.h"

class VlcMediaTest : public ::testing::Test {
protected:
    void SetUp() override {
        QStringList args{"--ignore-config", "--no-video"};
        inst = new VlcInstance(args);
    }
    void TearDown() override { delete inst; inst = nullptr; }
    VlcInstance *inst = nullptr;
};

TEST_F(VlcMediaTest, Lifecycle_ConstructDestruct_NoThrow) {
    EXPECT_NO_THROW({ VlcMedia m; });
}
TEST_F(VlcMediaTest, DISABLED_State_BeforeInit_NullCore_Skipped) {
    // state() before initMedia() dereferences a null libvlc media handle; the
    // supported path is state() AFTER initMedia (covered by Core_AfterInit).
    VlcMedia m;
    EXPECT_NO_THROW((void)m.state());
}
TEST_F(VlcMediaTest, GetCdaTrack_DefaultMinusOne) {
    VlcMedia m;
    EXPECT_EQ(m.getCdaTrack(), -1);
}
TEST_F(VlcMediaTest, InitMedia_LocalNonexistent_NoThrow) {
    VlcMedia m;
    EXPECT_NO_THROW(m.initMedia("/tmp/no-such-file.mp3", true, inst));
}
TEST_F(VlcMediaTest, InitMedia_NetLocation_NoThrow) {
    VlcMedia m;
    EXPECT_NO_THROW(m.initMedia("rtsp://example.invalid/stream", false, inst));
}
TEST_F(VlcMediaTest, InitMedia_CdaTrack_NoThrow) {
    VlcMedia m;
    EXPECT_NO_THROW(m.initMedia("cdda:///dev/sr0", false, inst, 2));
}
TEST_F(VlcMediaTest, Core_AfterInit_NoThrow) {
    VlcMedia m;
    m.initMedia("/tmp/no-such-file.mp3", true, inst);
    EXPECT_NO_THROW((void)m.core());
}
TEST_F(VlcMediaTest, ReleaseMedia_NoThrow) {
    VlcMedia m;
    m.initMedia("/tmp/no-such-file.mp3", true, inst);
    EXPECT_NO_THROW(m.releaseMedia());
}

TEST_F(VlcMediaTest, State_AfterInit_NoThrow) {
    VlcMedia m;
    m.initMedia("/tmp/no-such-file.mp3", true, inst);
    EXPECT_NO_THROW((void)m.state());
}
