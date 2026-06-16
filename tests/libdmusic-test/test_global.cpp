// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// global.cpp 的单元测试：DmGlobal 的 setter/getter 对与 Wayland 检测。
// 这些 set/get 用文件级静态变量存状态，跨用例共享——因此每个用例内
// set 后立即 get 验证，不依赖其他用例的执行顺序（GTest 不保证顺序）。

#include <gtest/gtest.h>

#include <QString>

#include "global.h"

// ============================================================================
// setAppName / getAppName
// ============================================================================
TEST(DmGlobalAppNameTest, setGetRoundTrip)
{
    DmGlobal::setAppName("test-app");
    EXPECT_EQ(DmGlobal::getAppName(), "test-app");
}

// ============================================================================
// setCachePath / cachePath（空路径应被忽略）
// ============================================================================
TEST(DmGlobalCachePathTest, setGetRoundTrip)
{
    DmGlobal::setCachePath("/tmp/dmusic-test-cache");
    EXPECT_EQ(DmGlobal::cachePath(), "/tmp/dmusic-test-cache");
}

TEST(DmGlobalCachePathTest, emptyPathIsIgnored)
{
    DmGlobal::setCachePath("/tmp/dmusic-keep");
    DmGlobal::setCachePath("");  // 空路径不应覆盖已有值
    EXPECT_EQ(DmGlobal::cachePath(), "/tmp/dmusic-keep");
}

// ============================================================================
// setConfigPath / configPath
// ============================================================================
TEST(DmGlobalConfigPathTest, setGetRoundTrip)
{
    DmGlobal::setConfigPath("/tmp/dmusic-test-config");
    EXPECT_EQ(DmGlobal::configPath(), "/tmp/dmusic-test-config");
}

// ============================================================================
// setMusicPath / musicPath
// ============================================================================
TEST(DmGlobalMusicPathTest, setGetRoundTrip)
{
    DmGlobal::setMusicPath("/tmp/dmusic-test-music");
    EXPECT_EQ(DmGlobal::musicPath(), "/tmp/dmusic-test-music");
}

// ============================================================================
// setUnknownAlbumText / unknownAlbumText
// ============================================================================
TEST(DmGlobalUnknownAlbumTest, setGetRoundTrip)
{
    DmGlobal::setUnknownAlbumText("未知专辑");
    EXPECT_EQ(DmGlobal::unknownAlbumText(), "未知专辑");
}

// ============================================================================
// setUnknownArtistText / unknownArtistText
// ============================================================================
TEST(DmGlobalUnknownArtistTest, setGetRoundTrip)
{
    DmGlobal::setUnknownArtistText("未知艺人");
    EXPECT_EQ(DmGlobal::unknownArtistText(), "未知艺人");
}

// ============================================================================
// setWaylandMode / isWaylandMode
// ============================================================================
TEST(DmGlobalWaylandModeTest, setGetRoundTrip)
{
    DmGlobal::setWaylandMode(true);
    EXPECT_TRUE(DmGlobal::isWaylandMode());
    DmGlobal::setWaylandMode(false);
    EXPECT_FALSE(DmGlobal::isWaylandMode());
}

// ============================================================================
// setPlaybackEngineType / playbackEngineType
// ============================================================================
TEST(DmGlobalPlaybackEngineTypeTest, setGetRoundTrip)
{
    DmGlobal::setPlaybackEngineType(1);
    EXPECT_EQ(DmGlobal::playbackEngineType(), 1);
    DmGlobal::setPlaybackEngineType(0);
    EXPECT_EQ(DmGlobal::playbackEngineType(), 0);
}

// ============================================================================
// checkWaylandMode : 读 XDG_SESSION_TYPE / WAYLAND_DISPLAY 环境变量
// 测试环境通常非 Wayland（offscreen/X11），结果一般为 false；
// 只断言"不崩溃 + 返回 bool + 与 isWaylandMode 一致"，不强断言具体值
// （CI 环境可能不同，避免环境依赖导致用例脆弱）。
// ============================================================================
TEST(DmGlobalCheckWaylandTest, returnsBoolAndConsistentWithIsWaylandMode)
{
    const bool result = DmGlobal::checkWaylandMode();
    // 调用后 isWaylandMode 应与返回值一致（checkWaylandMode 内部会设置 waylandMode）
    EXPECT_EQ(DmGlobal::isWaylandMode(), result);
}
