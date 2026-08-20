// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
// Graph-driven unit tests for src/libdmusic/player/vlc/cda.h (CdaThread).
#include <gtest/gtest.h>
#include "global.h"
#include "vlc/cda.h"

TEST(CdaThreadTest, Construct_NoThrow) {
    EXPECT_NO_THROW({ CdaThread t; });
}
TEST(CdaThreadTest, SetMediaPlayerPointer_Nullptr_NoThrow) {
    CdaThread t;
    EXPECT_NO_THROW(t.setMediaPlayerPointer(nullptr));
}
TEST(CdaThreadTest, GetCdaState_NoThrow) {
    CdaThread t;
    EXPECT_NO_THROW((void)t.getCdaState());
}
TEST(CdaThreadTest, CloseThread_NoThrow) {
    CdaThread t;
    EXPECT_NO_THROW(t.closeThread());
}
TEST(CdaThreadTest, GetCdaMetaInfo_NoThrow) {
    CdaThread t;
    EXPECT_NO_THROW((void)t.getCdaMetaInfo());
}
TEST(CdaThreadTest, DISABLED_DoQuery_NoCdDrive_Skipped) {
    // doQuery() performs CD-ROM probing which aborts with no drive in CI.
    CdaThread t;
    EXPECT_NO_THROW(t.doQuery());
}
