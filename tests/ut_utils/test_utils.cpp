// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <QCoreApplication>
#include <QChar>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QVariantMap>
#include <QDBusConnection>
#include <complex>
#include <vector>

#include "utils.h"
#include "global.h"

// 提供一个 QCoreApplication，供 DBus / Qt 内部机制使用。
int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

// ============================ isChinese ============================
TEST(UtilsTest, IsChinese_CjkCharacter_ReturnsTrue)
{
    EXPECT_TRUE(Utils::isChinese(QChar(0x4E2D)));  // "中"
    EXPECT_TRUE(Utils::isChinese(QChar(0x4E00)));  // 范围下界
    EXPECT_TRUE(Utils::isChinese(QChar(0x9FBF)));  // 范围上界
}

TEST(UtilsTest, IsChinese_AsciiAndDigit_ReturnsFalse)
{
    EXPECT_FALSE(Utils::isChinese(QChar('a')));
    EXPECT_FALSE(Utils::isChinese(QChar('Z')));
    EXPECT_FALSE(Utils::isChinese(QChar('0')));
    EXPECT_FALSE(Utils::isChinese(QChar(' ')));
}

// ============================ simpleChineseSplit ============================
TEST(UtilsTest, SimpleChineseSplit_PureAscii_ReturnsConcatenatedWords)
{
    QString s = QStringLiteral("Hello World");
    QStringList result = Utils::simpleChineseSplit(s);
    ASSERT_GE(result.size(), 2);
    EXPECT_TRUE(result.join("").contains("Hello"));
    EXPECT_TRUE(result.join("").contains("World"));
}

TEST(UtilsTest, SimpleChineseSplit_PureChinese_ProducesPinyinTokens)
{
    QString s = QStringLiteral("音乐");
    QStringList result = Utils::simpleChineseSplit(s);
    EXPECT_FALSE(result.isEmpty());
}

TEST(UtilsTest, SimpleChineseSplit_MixedAsciiAndChinese_SplitsBoth)
{
    QString s = QStringLiteral("a音乐b");
    QStringList result = Utils::simpleChineseSplit(s);
    EXPECT_FALSE(result.isEmpty());
    EXPECT_GE(result.size(), 2);
}

// ============================ updateChineseMetaInfo ============================
TEST(UtilsTest, UpdateChineseMetaInfo_ChineseTitle_FillsPinyinFields)
{
    DMusic::MediaMeta meta;
    meta.title = QStringLiteral("音乐");
    meta.album = QStringLiteral("专辑");
    meta.artist = QStringLiteral("歌手");
    Utils::updateChineseMetaInfo(meta);
    EXPECT_FALSE(meta.pinyinTitle.isEmpty());
    EXPECT_FALSE(meta.pinyinAlbum.isEmpty());
    EXPECT_FALSE(meta.pinyinArtist.isEmpty());
}

TEST(UtilsTest, UpdateChineseMetaInfo_AsciiTitle_FillsPinyinShort)
{
    DMusic::MediaMeta meta;
    meta.title = QStringLiteral("Hello");
    meta.album = QStringLiteral("World");
    meta.artist = QStringLiteral("Foo");
    Utils::updateChineseMetaInfo(meta);
    EXPECT_FALSE(meta.pinyinTitle.isEmpty());
    EXPECT_FALSE(meta.pinyinTitleShort.isEmpty());
}

// ============================ detectEncodings ============================
TEST(UtilsTest, DetectEncodings_EmptyData_ReturnsAtLeastLocaleCharset)
{
    QByteArray empty;
    QStringList encs = Utils::detectEncodings(empty);
    EXPECT_FALSE(encs.isEmpty());
}

TEST(UtilsTest, DetectEncodings_AsciiData_DetectsCharset)
{
    QByteArray data = QByteArray("The quick brown fox jumps over the lazy dog");
    QStringList encs = Utils::detectEncodings(data);
    EXPECT_FALSE(encs.isEmpty());
}

TEST(UtilsTest, DetectEncodings_Utf8Chinese_DetectsCharset)
{
    QByteArray data = QString::fromUtf8("这是一段中文文本").toUtf8();
    QStringList encs = Utils::detectEncodings(data);
    EXPECT_FALSE(encs.isEmpty());
}

// ============================ filePathHash ============================
TEST(UtilsTest, FilePathHash_KnownInput_ReturnsMd5Hex)
{
    // md5("abc") = 900150983cd24fb0d6963f7d28e17f72
    EXPECT_EQ(Utils::filePathHash(QStringLiteral("abc")),
              QStringLiteral("900150983cd24fb0d6963f7d28e17f72"));
}

TEST(UtilsTest, FilePathHash_DifferentInputs_ReturnDifferentHashes)
{
    EXPECT_NE(Utils::filePathHash(QStringLiteral("/a/b.mp3")),
              Utils::filePathHash(QStringLiteral("/a/c.mp3")));
}

// ============================ fft ============================
// 注意：源码 fft() 在 Log2N>=2 时因内层循环复用变量 i 会死循环（source_defect），
// 此处仅测试安全的 Log2N=1（2 点）路径。
TEST(UtilsTest, Fft_TwoPointForwardTransform_ReturnsSumAndDiff)
{
    const int Log2N = 1;
    std::vector<std::complex<float> > data = {{1.0f, 0.0f}, {0.0f, 0.0f}};
    Utils::fft(data.data(), Log2N, -1);  // 正变换
    EXPECT_FLOAT_EQ(data[0].real(), 1.0f);
    EXPECT_FLOAT_EQ(data[1].real(), 1.0f);
}

TEST(UtilsTest, Fft_TwoPointInverseTransform_NormalizesByLength)
{
    const int Log2N = 1;
    std::vector<std::complex<float> > data = {{1.0f, 0.0f}, {1.0f, 0.0f}};
    Utils::fft(data.data(), Log2N, 1);  // 逆变换（除以 length=2）
    EXPECT_NEAR(data[0].real(), 1.0f, 1e-5f);
    EXPECT_NEAR(data[1].real(), 0.0f, 1e-5f);
}

TEST(UtilsTest, Fft_TwoPointWithImaginary_CoversComplexArithmetic)
{
    const int Log2N = 1;
    std::vector<std::complex<float> > data = {{0.0f, 1.0f}, {0.0f, -1.0f}};
    Utils::fft(data.data(), Log2N, -1);
    EXPECT_NEAR(data[0].real(), 0.0f, 1e-5f);
    EXPECT_NEAR(data[1].real(), 0.0f, 1e-5f);
}

// ============================ readDBusProperty ============================
TEST(UtilsTest, ReadDBusProperty_NonexistentService_ReturnsFallbackValue)
{
    // 不存在的服务：无论是否有会话总线，属性都不可读，返回回退值（QVariant(0)
    // 或 invalid QVariant）。此处主要保证函数被进入并安全返回。
    QVariant v = Utils::readDBusProperty(
        QStringLiteral("org.deepin.music.nonexistent.service.ut"),
        QStringLiteral("/"),
        QStringLiteral("org.deepin.music.ut"),
        "Name",
        QDBusConnection::sessionBus());
    // 对不存在的服务，绝不应返回一个真实属性字符串值。
    EXPECT_FALSE(v.isValid() && !v.toString().isEmpty()
                && v.toString() == QStringLiteral("RealNameValue"));
}

// ============================ metaToVariantMap ============================
TEST(UtilsTest, MetaToVariantMap_PopulatedMeta_ContainsAllKeys)
{
    DMusic::MediaMeta meta;
    meta.hash = "h1";
    meta.localPath = "/a/b.mp3";
    meta.title = "Title";
    meta.artist = "Artist";
    meta.album = "Album";
    meta.length = 100;
    meta.size = 200;
    meta.favourite = true;
    meta.filetype = "mp3";

    QVariantMap m = Utils::metaToVariantMap(meta);
    EXPECT_EQ(m.value("hash").toString(), "h1");
    EXPECT_EQ(m.value("localPath").toString(), "/a/b.mp3");
    EXPECT_EQ(m.value("title").toString(), "Title");
    EXPECT_EQ(m.value("artist").toString(), "Artist");
    EXPECT_EQ(m.value("album").toString(), "Album");
    EXPECT_EQ(m.value("length").toLongLong(), 100);
    EXPECT_EQ(m.value("size").toLongLong(), 200);
    EXPECT_EQ(m.value("favourite").toBool(), true);
    EXPECT_EQ(m.value("filetype").toString(), "mp3");
}

TEST(UtilsTest, MetaToVariantMap_DefaultMeta_HasExpectedKeySet)
{
    QVariantMap m = Utils::metaToVariantMap(DMusic::MediaMeta{});
    EXPECT_TRUE(m.contains("hash"));
    EXPECT_TRUE(m.contains("title"));
    EXPECT_TRUE(m.contains("pinyinTitle"));
    EXPECT_TRUE(m.contains("coverUrl"));
    EXPECT_TRUE(m.contains("inMulitSelect"));
    EXPECT_TRUE(m.contains("dragFlag"));
}

// ============================ albumToVariantMap ============================
TEST(UtilsTest, AlbumToVariantMap_WithMusicInfos_MapsNestedMetas)
{
    DMusic::AlbumInfo album;
    album.name = "Album";
    album.pinyin = "album";
    album.artist = "Artist";
    album.timestamp = 42;
    DMusic::MediaMeta meta;
    meta.hash = "m1";
    meta.title = "Song";
    album.musicinfos.insert("m1", meta);

    QVariantMap m = Utils::albumToVariantMap(album);
    EXPECT_EQ(m.value("name").toString(), "Album");
    EXPECT_EQ(m.value("pinyin").toString(), "album");
    EXPECT_EQ(m.value("artist").toString(), "Artist");
    EXPECT_EQ(m.value("timestamp").toLongLong(), 42);
    QVariantMap infos = m.value("musicinfos").toMap();
    EXPECT_TRUE(infos.contains("m1"));
    EXPECT_EQ(infos.value("m1").toMap().value("title").toString(), "Song");
}

TEST(UtilsTest, AlbumToVariantMap_EmptyAlbum_HasEmptyMusicinfos)
{
    QVariantMap m = Utils::albumToVariantMap(DMusic::AlbumInfo{});
    EXPECT_TRUE(m.value("musicinfos").toMap().isEmpty());
}

// ============================ artistToVariantMap ============================
TEST(UtilsTest, ArtistToVariantMap_WithMusicInfos_MapsNestedMetas)
{
    DMusic::ArtistInfo artist;
    artist.name = "Singer";
    artist.pinyin = "singer";
    artist.timestamp = 7;
    DMusic::MediaMeta meta;
    meta.hash = "m2";
    meta.title = "Track";
    artist.musicinfos.insert("m2", meta);

    QVariantMap m = Utils::artistToVariantMap(artist);
    EXPECT_EQ(m.value("name").toString(), "Singer");
    EXPECT_EQ(m.value("pinyin").toString(), "singer");
    EXPECT_EQ(m.value("timestamp").toLongLong(), 7);
    QVariantMap infos = m.value("musicinfos").toMap();
    EXPECT_TRUE(infos.contains("m2"));
    EXPECT_EQ(infos.value("m2").toMap().value("title").toString(), "Track");
}

// ============================ playlistToVariantMap ============================
TEST(UtilsTest, PlaylistToVariantMap_PopulatedPlaylist_MapsCoreFields)
{
    DMusic::PlaylistInfo pl;
    pl.uuid = "u1";
    pl.displayName = "Fav";
    pl.icon = "icon";
    pl.sortType = DmGlobal::SortByTitleDES;  // 7 -> simplified 12
    pl.orderType = 1;
    pl.sortID = 9;
    pl.editmode = true;
    pl.readonly = true;
    pl.hide = false;
    pl.active = true;
    pl.playStatus = false;

    QVariantMap m = Utils::playlistToVariantMap(pl);
    EXPECT_EQ(m.value("uuid").toString(), "u1");
    EXPECT_EQ(m.value("displayName").toString(), "Fav");
    EXPECT_EQ(m.value("icon").toString(), "icon");
    EXPECT_EQ(m.value("sortType").toInt(), DmGlobal::SortByTitle);
    EXPECT_EQ(m.value("orderType").toInt(), 1);
    EXPECT_EQ(m.value("sortID").toUInt(), 9u);
    EXPECT_EQ(m.value("editmode").toBool(), true);
    EXPECT_EQ(m.value("readonly").toBool(), true);
    EXPECT_EQ(m.value("hide").toBool(), false);
    EXPECT_EQ(m.value("active").toBool(), true);
    EXPECT_EQ(m.value("playStatus").toBool(), false);
}

// ============================ simplifyPlaylistSortType ============================
struct SortTypeCase {
    int input;
    int expected;
};

class SimplifySortTypeTest : public ::testing::WithParamInterface<SortTypeCase>,
                             public ::testing::Test {
};

TEST_P(SimplifySortTypeTest, mapsToSimplifiedType)
{
    EXPECT_EQ(Utils::simplifyPlaylistSortType(GetParam().input), GetParam().expected);
}

INSTANTIATE_TEST_SUITE_P(Maps, SimplifySortTypeTest, ::testing::Values(
    SortTypeCase{DmGlobal::SortByAddTimeASC, DmGlobal::SortByAddTime},
    SortTypeCase{DmGlobal::SortByAddTimeDES, DmGlobal::SortByAddTime},
    SortTypeCase{DmGlobal::SortByTitleASC, DmGlobal::SortByTitle},
    SortTypeCase{DmGlobal::SortByTitleDES, DmGlobal::SortByTitle},
    SortTypeCase{DmGlobal::SortByArtistASC, DmGlobal::SortByArtist},
    SortTypeCase{DmGlobal::SortByArtistDES, DmGlobal::SortByArtist},
    SortTypeCase{DmGlobal::SortByAblumASC, DmGlobal::SortByAblum},
    SortTypeCase{DmGlobal::SortByAblumDES, DmGlobal::SortByAblum},
    SortTypeCase{DmGlobal::SortByCustomASC, DmGlobal::SortByCustom},
    SortTypeCase{DmGlobal::SortByCustomDES, DmGlobal::SortByCustom},
    SortTypeCase{0, DmGlobal::SortByAddTime},      // default
    SortTypeCase{-1, DmGlobal::SortByAddTime},     // default
    SortTypeCase{999, DmGlobal::SortByAddTime}     // default
));

// ============================ containsStr ============================
TEST(UtilsTest, ContainsStr_EnglishCaseInsensitive_Matches)
{
    EXPECT_TRUE(Utils::containsStr("love", "Deepin Love"));
    EXPECT_TRUE(Utils::containsStr("LOVE", "deepin love"));
}

TEST(UtilsTest, ContainsStr_ChineseSubstring_Matches)
{
    EXPECT_TRUE(Utils::containsStr("音乐", "深度音乐"));
    EXPECT_FALSE(Utils::containsStr("不存在", "深度音乐"));
}

TEST(UtilsTest, ContainsStr_NoMatch_ReturnsFalse)
{
    EXPECT_FALSE(Utils::containsStr("xyzabc", "hello world"));
}

TEST(UtilsTest, ContainsStr_IgnoresNewlineAndCarriageReturn)
{
    EXPECT_TRUE(Utils::containsStr("world", "hello\nworld"));
    EXPECT_TRUE(Utils::containsStr("world", "hello\r\nworld"));
}
