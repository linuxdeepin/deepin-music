// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
// Graph-driven unit tests for src/libdmusic/player/vlc/vlcdynamicinstance.h.
#include <gtest/gtest.h>
#include "vlc/vlcdynamicinstance.h"

TEST(VlcDynamicInstanceTest, VlcFunctionInstance_ReturnsNonNull) {
    auto *p = VlcDynamicInstance::VlcFunctionInstance();
    ASSERT_NE(p, nullptr);
}
TEST(VlcDynamicInstanceTest, VlcFunctionInstance_ReturnsSameInstance) {
    auto *a = VlcDynamicInstance::VlcFunctionInstance();
    auto *b = VlcDynamicInstance::VlcFunctionInstance();
    EXPECT_EQ(a, b);
}
TEST(VlcDynamicInstanceTest, ResolveSymbol_VlcNew_NoThrow) {
    auto *p = VlcDynamicInstance::VlcFunctionInstance();
    EXPECT_NO_THROW((void)p->resolveSymbol("libvlc_new"));
}
TEST(VlcDynamicInstanceTest, ResolveSymbol_Ffmpeg_NoThrow) {
    auto *p = VlcDynamicInstance::VlcFunctionInstance();
    EXPECT_NO_THROW((void)p->resolveSymbol("avcodec_open2", true));
}
TEST(VlcDynamicInstanceTest, ResolveSdlSymbol_NoThrow) {
    auto *p = VlcDynamicInstance::VlcFunctionInstance();
    EXPECT_NO_THROW((void)p->resolveSdlSymbol("SDL_Init"));
}
TEST(VlcDynamicInstanceTest, LoadSdlLibrary_NoThrow) {
    auto *p = VlcDynamicInstance::VlcFunctionInstance();
    EXPECT_NO_THROW((void)p->loadSdlLibrary());
}
TEST(VlcDynamicInstanceTest, ResolveSymbol_Nonexistent_NoThrow) {
    auto *p = VlcDynamicInstance::VlcFunctionInstance();
    EXPECT_NO_THROW((void)p->resolveSymbol("no_such_symbol_xyz"));
}
