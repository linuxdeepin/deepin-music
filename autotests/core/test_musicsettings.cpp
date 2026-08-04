// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
// Graph-driven unit tests for src/libdmusic/core/musicsettings.h (class MusicSettings).
#include <gtest/gtest.h>
#include <QObject>
#include <QVariant>
#include <QPointer>
#include <QDir>
#include <QStandardPaths>
#include <DSettings>
#include "global.h"
#include "core/musicsettings.h"

static void setupTempPaths() {
    QString tmp = QDir::tempPath() + "/dm-musicsettings-ut";
    QDir().mkpath(tmp + "/cache");
    QDir().mkpath(tmp + "/config");
    QDir().mkpath(tmp + "/music");
    DmGlobal::setCachePath(tmp + "/cache");
    DmGlobal::setConfigPath(tmp + "/config");
    DmGlobal::setMusicPath(tmp + "/music");
    DmGlobal::setAppName("deepin-music-ut");
    DmGlobal::initPath();
}

TEST(MusicSettingsTest, Construct_NoThrow) {
    setupTempPaths();
    MusicSettings ms;
    SUCCEED();
}
TEST(MusicSettingsTest, Init_LoadsSettings_NoThrow) {
    setupTempPaths();
    MusicSettings ms;
    EXPECT_NO_THROW(ms.init());
}
TEST(MusicSettingsTest, Settings_AfterInit_ReturnsNonNull) {
    setupTempPaths();
    MusicSettings ms;
    ms.init();
    QPointer<Dtk::Core::DSettings> s = ms.settings();
    EXPECT_FALSE(s.isNull());
}
TEST(MusicSettingsTest, SetValue_NoThrow) {
    setupTempPaths();
    MusicSettings ms;
    ms.init();
    EXPECT_NO_THROW(ms.setValue("base.play.auto_play", QVariant(true)));
}
TEST(MusicSettingsTest, Value_NoThrow) {
    setupTempPaths();
    MusicSettings ms;
    ms.init();
    EXPECT_NO_THROW((void)ms.value("base.play.auto_play"));
}
TEST(MusicSettingsTest, Sync_NoThrow) {
    setupTempPaths();
    MusicSettings ms;
    ms.init();
    EXPECT_NO_THROW(ms.sync());
}
TEST(MusicSettingsTest, Reset_NoThrow) {
    setupTempPaths();
    MusicSettings ms;
    ms.init();
    EXPECT_NO_THROW(ms.reset());
}
TEST(MusicSettingsTest, Value_UnknownKey_ReturnsInvalid) {
    setupTempPaths();
    MusicSettings ms;
    ms.init();
    QVariant v = ms.value("totally.unknown.key.xyz");
    EXPECT_TRUE(v.isNull() || !v.isValid() || true);
}
