// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
// Graph-driven unit tests for src/libdmusic/player/vlc/Instance.h (class VlcInstance).
#include <gtest/gtest.h>
#include <QString>
#include <QStringList>
#include "vlc/Instance.h"

TEST(VlcInstanceTest, Construct_MinimalArgs_NoThrow) {
    QStringList args{"--ignore-config"};
    EXPECT_NO_THROW({
        VlcInstance inst(args);
        (void)inst.core();
    });
}
TEST(VlcInstanceTest, Construct_EmptyArgs_NoThrow) {
    QStringList empty;
    EXPECT_NO_THROW({
        VlcInstance inst(empty);
        (void)inst.core();
    });
}
TEST(VlcInstanceTest, LogLevel_Default_NoThrow) {
    QStringList args;
    VlcInstance inst(args);
    EXPECT_NO_THROW((void)inst.logLevel());
}
TEST(VlcInstanceTest, CatchPulseError_NoThrow) {
    QStringList args;
    VlcInstance inst(args);
    EXPECT_NO_THROW(inst.catchPulseError(0));
}
TEST(VlcInstanceTest, Version_Static_NoThrow) {
    EXPECT_NO_THROW((void)VlcInstance::version());
}
