// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
// Graph-driven unit tests for src/libdmusic/core/dynamiclibraries.h (class DynamicLibraries).
#include <gtest/gtest.h>
#include "core/dynamiclibraries.h"

TEST(DynamicLibrariesTest, Instance_ReturnsNonNull) {
    auto *p = DynamicLibraries::instance();
    ASSERT_NE(p, nullptr);
}
TEST(DynamicLibrariesTest, Instance_ReturnsSameInstance) {
    auto *a = DynamicLibraries::instance();
    auto *b = DynamicLibraries::instance();
    EXPECT_EQ(a, b);
}
TEST(DynamicLibrariesTest, Resolve_VlcSymbol_NoThrow) {
    auto *p = DynamicLibraries::instance();
    EXPECT_NO_THROW((void)p->resolve("libvlc_new"));
}
TEST(DynamicLibrariesTest, Resolve_FfmpegSymbol_NoThrow) {
    auto *p = DynamicLibraries::instance();
    EXPECT_NO_THROW((void)p->resolve("avcodec_open2", true));
}
TEST(DynamicLibrariesTest, Resolve_NonexistentSymbol_NoThrow) {
    auto *p = DynamicLibraries::instance();
    EXPECT_NO_THROW((void)p->resolve("symbol_does_not_exist_xyz"));
}
TEST(DynamicLibrariesTest, Resolve_NonexistentFfmpegSymbol_NoThrow) {
    auto *p = DynamicLibraries::instance();
    EXPECT_NO_THROW((void)p->resolve("ffmpeg_no_such_symbol", true));
}
