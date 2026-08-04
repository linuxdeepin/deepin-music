// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
// Shared Google Test main entry: creates a QCoreApplication so that Qt
// SQL/threads/multimedia used by the dmusic backend operate under a headless
// event-loop owner. This file is compiled into every test_* executable
// (see autotests/cmake/UnitTestUtils.cmake); it replaces GTest::gtest_main.
#include <gtest/gtest.h>
#include <QCoreApplication>

int main(int argc, char **argv) {
    // Headless Qt: avoid any platform windowing/audio-backend assumptions.
    qputenv("QT_QPA_PLATFORM", "offscreen");
    QCoreApplication app(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
