// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// global.cpp 的单元测试：DmGlobal 的 setter/getter 对与 Wayland 检测。
// 这些 set/get 用文件级静态变量存状态，跨用例共享——因此每个用例内
// set 后立即 get 验证，不依赖其他用例的执行顺序（GTest 不保证顺序）。

#include <gtest/gtest.h>

#include <QString>
#include <QVariant>
#include <QMetaType>

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

// ============================================================================
// libPath : 在 Qt 库目录搜索动态库；不存在的库名应回退到原始名（默认分支）
// ============================================================================
TEST(DmGlobalLibPathTest, unknownLibFallsBackToName)
{
    // 一个肯定不存在的库名：list 为空 → libPath 保持为入参 strlib
    const QString result = DmGlobal::libPath("lib_not_exist_xyz123.so");
    EXPECT_TRUE(result.contains("lib_not_exist_xyz123"));
}

TEST(DmGlobalLibPathTest, returnsNonEmptyForAnyInput)
{
    EXPECT_FALSE(DmGlobal::libPath("libc.so.6").isEmpty());
}

// ============================================================================
// libExist : 检查动态库是否可加载
// 系统 libc 几乎一定可加载（true）；不存在的库为 false
// ============================================================================
TEST(DmGlobalLibExistTest, libcExistsReturnsTrue)
{
    // libc 是系统基础库，QLibrary("libc") 在 Linux 应可加载
    EXPECT_TRUE(DmGlobal::libExist("libc.so.6"));
}

TEST(DmGlobalLibExistTest, nonExistentLibReturnsFalse)
{
    EXPECT_FALSE(DmGlobal::libExist("lib_totally_nonexistent_zzz.so"));
}

// ============================================================================
// initPlaybackEngineType : libvlc/libavcodec 同时存在 → engineType=1，否则 0
// 测试环境通常不装这两个库 → 结果为 0；只断言"调用后 playbackEngineType 一致"
// ============================================================================
TEST(DmGlobalInitPlaybackEngineTest, setsEngineTypeConsistently)
{
    DmGlobal::initPlaybackEngineType();
    const int t = DmGlobal::playbackEngineType();
    // initPlaybackEngineType 内部只在 vlc+avcodec 都在时置 1，否则 0
    EXPECT_TRUE(t == 0 || t == 1);
}

// ============================================================================
// DmGlobal 实例化：QObject 派生，构造不崩溃；moc 元对象接口可调用
// 覆盖 DmGlobal 构造函数 + metaObject/qt_metacast/qt_metacall（moc 生成）
// ============================================================================
TEST(DmGlobalInstanceTest, constructsWithoutParent)
{
    DmGlobal g;
    EXPECT_EQ(g.parent(), nullptr);
}

TEST(DmGlobalInstanceTest, metaObjectIsValid)
{
    DmGlobal g;
    const QMetaObject *mo = g.metaObject();
    ASSERT_NE(mo, nullptr);
    EXPECT_STREQ(mo->className(), "DmGlobal");
}

TEST(DmGlobalInstanceTest, qtMetacastOnUnknownReturnsNull)
{
    DmGlobal g;
    // 未知类名 qt_metacast 返回 nullptr
    EXPECT_EQ(g.qt_metacast("NonExistentClass"), nullptr);
}

TEST(DmGlobalInstanceTest, qtMetacastOnQObjectReturnsNonNull)
{
    DmGlobal g;
    // DmGlobal 派生自 QObject，qt_metacast("QObject") 应返回非空
    void *p = g.qt_metacast("QObject");
    EXPECT_NE(p, nullptr);
}

TEST(DmGlobalInstanceTest, invokesStaticMetaCallEnumRead)
{
    // 通过 metaObject 调用静态 metaCall 读取枚举信息，触发 qt_static_metacall
    DmGlobal g;
    const QMetaObject *mo = g.metaObject();
    ASSERT_NE(mo, nullptr);
    // PlaybackStatus 枚举应能查到（index >= 0）
    const int idx = mo->indexOfEnumerator("PlaybackStatus");
    EXPECT_GE(idx, 0);
}

// ============================================================================
// Q_DECLARE_METATYPE 元类型注册：触发 global.h 中 static 元类型对象的初始化
// （DA:114/116/117 三行未覆盖 → 覆盖这些宏展开的 static 变量）
// ============================================================================
TEST(DmGlobalMetaTypeTest, registersAndUsesAllDeclaredMetaTypes)
{
    // qRegisterMetaType + QVariant::fromValue 触发 Q_DECLARE_METATYPE 的 static 初始化
    qRegisterMetaType<DmGlobal::PlaybackStatus>("DmGlobal::PlaybackStatus");
    qRegisterMetaType<DmGlobal::PlayerEngineType>("DmGlobal::PlayerEngineType");
    qRegisterMetaType<DmGlobal::PlaybackMode>("DmGlobal::PlaybackMode");
    qRegisterMetaType<DmGlobal::MimeType>("DmGlobal::MimeType");
    qRegisterMetaType<DmGlobal::PlaylistSortType>("DmGlobal::PlaylistSortType");

    // QVariant::fromValue 触发 static metatype 对象访问（global.h 第 114/116/117 行）
    QVariant v1 = QVariant::fromValue(DmGlobal::Playing);
    QVariant v2 = QVariant::fromValue(DmGlobal::VLC);
    QVariant v3 = QVariant::fromValue(DmGlobal::RepeatAll);
    QVariant v4 = QVariant::fromValue(DmGlobal::MimeTypeLocal);
    QVariant v5 = QVariant::fromValue(DmGlobal::SortByAddTime);

    EXPECT_TRUE(v1.isValid());
    EXPECT_TRUE(v2.isValid());
    EXPECT_TRUE(v3.isValid());
    EXPECT_TRUE(v4.isValid());
    EXPECT_TRUE(v5.isValid());
    // 取回值应与原值相等
    EXPECT_EQ(v1.value<DmGlobal::PlaybackStatus>(), DmGlobal::Playing);
    EXPECT_EQ(v2.value<DmGlobal::PlayerEngineType>(), DmGlobal::VLC);
    EXPECT_EQ(v3.value<DmGlobal::PlaybackMode>(), DmGlobal::RepeatAll);
    EXPECT_EQ(v4.value<DmGlobal::MimeType>(), DmGlobal::MimeTypeLocal);
    EXPECT_EQ(v5.value<DmGlobal::PlaylistSortType>(), DmGlobal::SortByAddTime);
}
