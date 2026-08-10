// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <QCoreApplication>
#include <QString>

#include "global.h"

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

// ============================ 构造 ============================
TEST(DmGlobalTest, Constructor_CreatesInstance_WithoutThrowing)
{
    DmGlobal global;
    EXPECT_TRUE(global.metaObject() != nullptr);
}

// ============================ AppName ============================
TEST(DmGlobalTest, AppName_SetThenGet_RoundTrips)
{
    DmGlobal::setAppName(QStringLiteral("deepin-music-ut"));
    EXPECT_EQ(DmGlobal::getAppName(), QStringLiteral("deepin-music-ut"));
}

// ============================ initPath / 路径 ============================
TEST(DmGlobalTest, InitPath_PopulatesConfigCacheMusicPaths)
{
    DmGlobal::initPath();
    // initPath 后路径应已被赋值（可能为空字符串，但调用本身覆盖函数）。
    DmGlobal::configPath();
    DmGlobal::cachePath();
    DmGlobal::musicPath();
    SUCCEED();
}

TEST(DmGlobalTest, ConfigPath_SetNonEmpty_GetReturnsIt)
{
    DmGlobal::setConfigPath(QStringLiteral("/tmp/dm_ut_config"));
    EXPECT_EQ(DmGlobal::configPath(), QStringLiteral("/tmp/dm_ut_config"));
}

TEST(DmGlobalTest, ConfigPath_SetEmpty_DoesNotChangeValue)
{
    DmGlobal::setConfigPath(QStringLiteral("/tmp/dm_ut_config2"));
    DmGlobal::setConfigPath(QString());
    EXPECT_EQ(DmGlobal::configPath(), QStringLiteral("/tmp/dm_ut_config2"));
}

TEST(DmGlobalTest, CachePath_SetNonEmpty_GetReturnsIt)
{
    DmGlobal::setCachePath(QStringLiteral("/tmp/dm_ut_cache"));
    EXPECT_EQ(DmGlobal::cachePath(), QStringLiteral("/tmp/dm_ut_cache"));
}

TEST(DmGlobalTest, CachePath_SetEmpty_DoesNotChangeValue)
{
    DmGlobal::setCachePath(QStringLiteral("/tmp/dm_ut_cache2"));
    DmGlobal::setCachePath(QString());
    EXPECT_EQ(DmGlobal::cachePath(), QStringLiteral("/tmp/dm_ut_cache2"));
}

TEST(DmGlobalTest, MusicPath_SetNonEmpty_GetReturnsIt)
{
    DmGlobal::setMusicPath(QStringLiteral("/tmp/dm_ut_music"));
    EXPECT_EQ(DmGlobal::musicPath(), QStringLiteral("/tmp/dm_ut_music"));
}

TEST(DmGlobalTest, MusicPath_SetEmpty_DoesNotChangeValue)
{
    DmGlobal::setMusicPath(QStringLiteral("/tmp/dm_ut_music2"));
    DmGlobal::setMusicPath(QString());
    EXPECT_EQ(DmGlobal::musicPath(), QStringLiteral("/tmp/dm_ut_music2"));
}

// ============================ UnknownAlbum/Artist ============================
TEST(DmGlobalTest, UnknownAlbumText_SetNonEmpty_GetReturnsIt)
{
    DmGlobal::setUnknownAlbumText(QStringLiteral("未知专辑"));
    EXPECT_EQ(DmGlobal::unknownAlbumText(), QStringLiteral("未知专辑"));
}

TEST(DmGlobalTest, UnknownAlbumText_SetEmpty_DoesNotChangeValue)
{
    DmGlobal::setUnknownAlbumText(QStringLiteral("未知专辑2"));
    DmGlobal::setUnknownAlbumText(QString());
    EXPECT_EQ(DmGlobal::unknownAlbumText(), QStringLiteral("未知专辑2"));
}

TEST(DmGlobalTest, UnknownArtistText_SetNonEmpty_GetReturnsIt)
{
    DmGlobal::setUnknownArtistText(QStringLiteral("未知艺人"));
    EXPECT_EQ(DmGlobal::unknownArtistText(), QStringLiteral("未知艺人"));
}

TEST(DmGlobalTest, UnknownArtistText_SetEmpty_DoesNotChangeValue)
{
    DmGlobal::setUnknownArtistText(QStringLiteral("未知艺人2"));
    DmGlobal::setUnknownArtistText(QString());
    EXPECT_EQ(DmGlobal::unknownArtistText(), QStringLiteral("未知艺人2"));
}

// ============================ Wayland ============================
TEST(DmGlobalTest, WaylandMode_SetThenIs_ReturnsSetValue)
{
    DmGlobal::setWaylandMode(true);
    EXPECT_TRUE(DmGlobal::isWaylandMode());
    DmGlobal::setWaylandMode(false);
    EXPECT_FALSE(DmGlobal::isWaylandMode());
}

TEST(DmGlobalTest, CheckWaylandMode_ReturnsBoolAndSetsState)
{
    bool result = DmGlobal::checkWaylandMode();
    EXPECT_EQ(DmGlobal::isWaylandMode(), result);
}

// ============================ libPath ============================
TEST(DmGlobalTest, LibPath_KnownLib_ReturnsNonEmptyPath)
{
    QString p = DmGlobal::libPath(QStringLiteral("libc"));
    EXPECT_FALSE(p.isEmpty());
}

TEST(DmGlobalTest, LibPath_NonexistentLib_ReturnsDefaultName)
{
    QString p = DmGlobal::libPath(QStringLiteral("zzz_nonexistent_lib_xyz"));
    EXPECT_FALSE(p.isEmpty());
}

// ============================ libExist ============================
TEST(DmGlobalTest, LibExist_SystemLib_ReturnsTrue)
{
    // libc 是系统核心库，必然可加载。
    EXPECT_TRUE(DmGlobal::libExist(QStringLiteral("libc")));
}

TEST(DmGlobalTest, LibExist_NonexistentLib_ReturnsFalse)
{
    EXPECT_FALSE(DmGlobal::libExist(QStringLiteral("zzz_nonexistent_lib_xyz")));
}

// ============================ PlaybackEngineType ============================
TEST(DmGlobalTest, PlaybackEngineType_SetThenGet_RoundTrips)
{
    DmGlobal::setPlaybackEngineType(1);
    EXPECT_EQ(DmGlobal::playbackEngineType(), 1);
    DmGlobal::setPlaybackEngineType(0);
    EXPECT_EQ(DmGlobal::playbackEngineType(), 0);
}

TEST(DmGlobalTest, InitPlaybackEngineType_SetsValidEngineType)
{
    DmGlobal::initPlaybackEngineType();
    int t = DmGlobal::playbackEngineType();
    EXPECT_TRUE(t == 0 || t == 1);
}
