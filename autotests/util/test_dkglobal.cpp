// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
// Graph-driven unit tests for src/libdmusic/global.h (class DmGlobal + DMusic structs).
#include <gtest/gtest.h>
#include <QObject>
#include <QString>
#include <QVariant>
#include "global.h"

TEST(DmGlobalTest, Construct_WithParent_NoThrow) {
    QObject parent;
    DmGlobal g(&parent);
    SUCCEED();
}
TEST(DmGlobalTest, SetGetAppName_Roundtrip_Matches) {
    DmGlobal::setAppName("deepin-music-test");
    EXPECT_EQ(DmGlobal::getAppName(), QString("deepin-music-test"));
}
TEST(DmGlobalTest, InitPath_NoThrow) {
    EXPECT_NO_THROW(DmGlobal::initPath());
}
TEST(DmGlobalTest, SetGetConfigPath_Roundtrip_Matches) {
    DmGlobal::setConfigPath("/tmp/dm-cfg-test");
    EXPECT_EQ(DmGlobal::configPath(), QString("/tmp/dm-cfg-test"));
}
TEST(DmGlobalTest, SetGetCachePath_Roundtrip_Matches) {
    DmGlobal::setCachePath("/tmp/dm-cache-test");
    EXPECT_EQ(DmGlobal::cachePath(), QString("/tmp/dm-cache-test"));
}
TEST(DmGlobalTest, SetGetMusicPath_Roundtrip_Matches) {
    DmGlobal::setMusicPath("/tmp/dm-music-test");
    EXPECT_EQ(DmGlobal::musicPath(), QString("/tmp/dm-music-test"));
}
TEST(DmGlobalTest, SetGetUnknownAlbumText_Roundtrip_Matches) {
    DmGlobal::setUnknownAlbumText("UnknownAlbum");
    EXPECT_EQ(DmGlobal::unknownAlbumText(), QString("UnknownAlbum"));
}
TEST(DmGlobalTest, SetGetUnknownArtistText_Roundtrip_Matches) {
    DmGlobal::setUnknownArtistText("UnknownArtist");
    EXPECT_EQ(DmGlobal::unknownArtistText(), QString("UnknownArtist"));
}
TEST(DmGlobalTest, CheckWaylandMode_NoThrow) {
    EXPECT_NO_THROW((void)DmGlobal::checkWaylandMode());
}
TEST(DmGlobalTest, SetIsWaylandMode_Roundtrip_Matches) {
    DmGlobal::setWaylandMode(true);
    EXPECT_TRUE(DmGlobal::isWaylandMode());
    DmGlobal::setWaylandMode(false);
    EXPECT_FALSE(DmGlobal::isWaylandMode());
}
TEST(DmGlobalTest, LibPath_NonEmptyString) {
    QString p = DmGlobal::libPath("libvlc");
    EXPECT_FALSE(p.isEmpty());
}
TEST(DmGlobalTest, LibExist_NonexistentLib_ReturnsFalse) {
    EXPECT_FALSE(DmGlobal::libExist("lib_totally_does_not_exist_xyz"));
}
TEST(DmGlobalTest, InitPlaybackEngineType_NoThrow) {
    EXPECT_NO_THROW(DmGlobal::initPlaybackEngineType());
}
TEST(DmGlobalTest, SetGetPlaybackEngineType_Roundtrip_Matches) {
    DmGlobal::setPlaybackEngineType(DmGlobal::QtMEDIAPLAYER);
    EXPECT_EQ(DmGlobal::playbackEngineType(), DmGlobal::QtMEDIAPLAYER);
}
TEST(DmGlobalTest, MediaMeta_Defaults_HaveExpectedInit) {
    DMusic::MediaMeta m;
    EXPECT_EQ(m.mmType, DmGlobal::MimeTypeLocal);
    EXPECT_EQ(m.length, qint64(1));
    EXPECT_FALSE(m.favourite);
}
TEST(DmGlobalTest, PlaylistInfo_Defaults_HaveExpectedInit) {
    DMusic::PlaylistInfo p;
    EXPECT_FALSE(p.readonly);
    EXPECT_TRUE(p.saveFalg);
}

TEST(DmGlobalTest, EnumMetatypes_Register_NoThrow) {
    EXPECT_NO_THROW((void)QVariant::fromValue(DmGlobal::PlaybackStatus(DmGlobal::Playing)));
    EXPECT_NO_THROW((void)QVariant::fromValue(DmGlobal::PlayerEngineType(DmGlobal::VLC)));
    EXPECT_NO_THROW((void)QVariant::fromValue(DmGlobal::MimeType(DmGlobal::MimeTypeCDA)));
    EXPECT_NO_THROW((void)QVariant::fromValue(DmGlobal::PlaylistSortType(DmGlobal::SortByTitle)));
}
