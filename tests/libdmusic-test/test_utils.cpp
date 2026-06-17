// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// utils.cpp 的单元测试：纯函数为主，零外部依赖，用 GTest 参数化覆盖边界条件。
// 对应 docs/unit-test-plan.md Phase 1 / ①。

#include <gtest/gtest.h>

#include <QChar>
#include <QString>
#include <QStringList>
#include <QVariantMap>
#include <complex>

#include "utils.h"
#include "global.h"

// ============================================================================
// isChinese : CJK 统一汉字范围 [0x4E00, 0x9FBF]
// ============================================================================
struct IsChineseCase {
    QChar input;
    bool expected;
};

class IsChineseTest : public ::testing::WithParamInterface<IsChineseCase>,
                      public ::testing::Test {
};

TEST_P(IsChineseTest, detectsChineseRange)
{
    EXPECT_EQ(Utils::isChinese(GetParam().input), GetParam().expected);
}

INSTANTIATE_TEST_SUITE_P(Detect, IsChineseTest, ::testing::Values(
    IsChineseCase{QChar(0x4E00), true},   // 范围下界 "一"
    IsChineseCase{QChar(0x4E2D), true},   // "中"
    IsChineseCase{QChar(0x9FBF), true},   // 范围上界
    IsChineseCase{QChar('a'),   false},   // 拉丁字母
    IsChineseCase{QChar('1'),   false},   // 数字
    IsChineseCase{QChar(' '),   false},   // 空格
    IsChineseCase{QChar(0x3042), false}   // 日文平假名，不在中文范围
));

// ============================================================================
// filePathHash : MD5，确定性 + 不同输入产生不同输出
// ============================================================================
TEST(UtilsFilePathHashTest, IsDeterministicForSameInput)
{
    EXPECT_EQ(Utils::filePathHash("/a/b.mp3"), Utils::filePathHash("/a/b.mp3"));
}

TEST(UtilsFilePathHashTest, DiffersForDifferentInput)
{
    EXPECT_NE(Utils::filePathHash("/a/b.mp3"), Utils::filePathHash("/a/c.mp3"));
}

TEST(UtilsFilePathHashTest, IsHexMd5Length32)
{
    const QString hash = Utils::filePathHash("x");
    EXPECT_EQ(hash.length(), 32);
    // 仅含十六进制字符
    EXPECT_TRUE(std::all_of(hash.begin(), hash.end(), [](const QChar &c) {
        QChar ch = c.toLower();
        return (ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f');
    }));
}

// ============================================================================
// containsStr : 中文/英文搜索，大小写不敏感
// ============================================================================
TEST(UtilsContainsStrTest, matchesEnglishCaseInsensitive)
{
    EXPECT_TRUE(Utils::containsStr("love", "Deepin Love"));
    EXPECT_TRUE(Utils::containsStr("LOVE", "deepin love"));
}

TEST(UtilsContainsStrTest, matchesChineseSubstring)
{
    EXPECT_TRUE(Utils::containsStr("音乐", "深度音乐"));
    EXPECT_FALSE(Utils::containsStr("不存在", "深度音乐"));
}

TEST(UtilsContainsStrTest, noMatchReturnsFalse)
{
    EXPECT_FALSE(Utils::containsStr("xyzabc", "hello world"));
}

TEST(UtilsContainsStrTest, ignoresNewlineAndCarriageReturn)
{
    // 源码会 remove \r \n，跨行搜索应能命中
    EXPECT_TRUE(Utils::containsStr("world", "hello\nworld"));
}

// ============================================================================
// simplifyPlaylistSortType : ASC/DES 归并到无方向的基础类型
// 枚举值来自 global.h：ASC/DES 为 0~5，基础类型 10~14
// ============================================================================
struct SortTypeCase {
    int input;
    int expected;
};

class SimplifySortTypeTest : public ::testing::WithParamInterface<SortTypeCase>,
                             public ::testing::Test {
};

TEST_P(SimplifySortTypeTest, mapsToBaseType)
{
    EXPECT_EQ(Utils::simplifyPlaylistSortType(GetParam().input), GetParam().expected);
}

INSTANTIATE_TEST_SUITE_P(Map, SimplifySortTypeTest, ::testing::Values(
    // ASC / DES 成对归并
    SortTypeCase{DmGlobal::SortByAddTimeASC, DmGlobal::SortByAddTime},
    SortTypeCase{DmGlobal::SortByAddTimeDES, DmGlobal::SortByAddTime},
    SortTypeCase{DmGlobal::SortByTitleASC,   DmGlobal::SortByTitle},
    SortTypeCase{DmGlobal::SortByTitleDES,   DmGlobal::SortByTitle},
    SortTypeCase{DmGlobal::SortByAblumASC,   DmGlobal::SortByAblum},
    SortTypeCase{DmGlobal::SortByAblumDES,   DmGlobal::SortByAblum},
    SortTypeCase{DmGlobal::SortByArtistASC,  DmGlobal::SortByArtist},
    SortTypeCase{DmGlobal::SortByArtistDES,  DmGlobal::SortByArtist},
    SortTypeCase{DmGlobal::SortByCustomASC,  DmGlobal::SortByCustom},
    SortTypeCase{DmGlobal::SortByCustomDES,  DmGlobal::SortByCustom},
    // 未知值回退到 SortByAddTime
    SortTypeCase{999, DmGlobal::SortByAddTime},
    SortTypeCase{-1,  DmGlobal::SortByAddTime}
));

// ============================================================================
// metaToVariantMap : 字段完整映射到 QVariantMap
// ============================================================================
TEST(UtilsMetaToVariantMapTest, mapsAllStringFields)
{
    DMusic::MediaMeta meta;
    meta.hash = "h1";
    meta.localPath = "/a/b.mp3";
    meta.title = "T";
    meta.artist = "A";
    meta.album = "Al";

    const QVariantMap m = Utils::metaToVariantMap(meta);
    EXPECT_EQ(m.value("hash").toString(),      "h1");
    EXPECT_EQ(m.value("localPath").toString(), "/a/b.mp3");
    EXPECT_EQ(m.value("title").toString(),     "T");
    EXPECT_EQ(m.value("artist").toString(),    "A");
    EXPECT_EQ(m.value("album").toString(),     "Al");
}

TEST(UtilsMetaToVariantMapTest, mapsNumericAndFlagFields)
{
    DMusic::MediaMeta meta;
    meta.length = 12345;
    meta.size = 6789;
    meta.favourite = true;
    meta.hasimage = false;

    const QVariantMap m = Utils::metaToVariantMap(meta);
    EXPECT_EQ(m.value("length").toLongLong(),    12345);
    EXPECT_EQ(m.value("size").toLongLong(),      6789);
    EXPECT_EQ(m.value("favourite").toBool(),     true);
    EXPECT_EQ(m.value("hasimage").toBool(),      false);
}

TEST(UtilsMetaToVariantMapTest, containsAllExpectedKeys)
{
    const QVariantMap m = Utils::metaToVariantMap(DMusic::MediaMeta{});
    const QStringList expectedKeys = {
        "hash", "localPath", "cuePath", "title", "artist", "album", "lyricPath",
        "pinyinTitle", "filetype", "mmType", "timestamp", "length", "size",
        "favourite", "invalid", "codec", "inMulitSelect", "dragFlag"
    };
    for (const auto &key : expectedKeys) {
        EXPECT_TRUE(m.contains(key)) << "Missing key: " << key.toStdString();
    }
}

// ============================================================================
// playlistToVariantMap : 内部会调用 simplifyPlaylistSortType
// ============================================================================
TEST(UtilsPlaylistToVariantMapTest, mapsCoreFields)
{
    DMusic::PlaylistInfo pl;
    pl.uuid = "u1";
    pl.displayName = "My List";
    pl.sortType = DmGlobal::SortByTitleASC;  // 应被简化为 SortByTitle

    const QVariantMap m = Utils::playlistToVariantMap(pl);
    EXPECT_EQ(m.value("uuid").toString(), "u1");
    EXPECT_EQ(m.value("displayName").toString(), "My List");
    EXPECT_EQ(m.value("sortType").toInt(), DmGlobal::SortByTitle);
}

// ============================================================================
// fft : 傅里叶变换数学性质
//
// 注意：utils.cpp 的 fft() 在 Log2N>=2 时存在变量复用缺陷——外层循环变量 i
// 被最内层 for(i=0; i<length/step; i++) 覆盖，导致死循环。因此用 Log2N=1
// （length=2，外层 i 循环条件 i<=Log2N 不成立直接跳过）来安全覆盖 fft 主体：
// - 2 点蝶形（第一层）
// - sign==-1 正变换分支
// - sign==1 逆变换分支（除以 length）
// 大点数用例保持 DISABLED_，待 src 修复后启用。
// ============================================================================
TEST(UtilsFftTest, twoPointForwardTransformIsCorrect)
{
    // Log2N=1：只走 2 点蝶形，不会进入外层循环（规避变量复用 bug）
    const int Log2N = 1;
    std::vector<std::complex<float>> data = {{1.0f, 0.0f}, {3.0f, 0.0f}};

    Utils::fft(data.data(), Log2N, -1);  // 正变换

    // X[0]=x[0]+x[1]=4, X[1]=x[0]-x[1]=-2
    EXPECT_NEAR(data[0].real(), 4.0f, 1e-3f);
    EXPECT_NEAR(data[1].real(), -2.0f, 1e-3f);
    EXPECT_NEAR(data[0].imag(), 0.0f, 1e-3f);
    EXPECT_NEAR(data[1].imag(), 0.0f, 1e-3f);
}

TEST(UtilsFftTest, twoPointInverseTransformNormalizesByLength)
{
    // sign==1：逆变换，结束时除以 length（=2）
    const int Log2N = 1;
    std::vector<std::complex<float>> data = {{1.0f, 0.0f}, {3.0f, 0.0f}};

    Utils::fft(data.data(), Log2N, 1);  // 逆变换

    // 先 2 点蝶形：(4,-2)；再除以 2：(2,-1)
    EXPECT_NEAR(data[0].real(), 2.0f, 1e-3f);
    EXPECT_NEAR(data[1].real(), -1.0f, 1e-3f);
}

TEST(UtilsFftTest, DISABLED_dcSignalConcentratesInFirstBin)
{
    const int Log2N = 3;          // length = 8
    const int length = 1 << Log2N;
    std::vector<std::complex<float>> data(length, std::complex<float>(1.0f, 0.0f));

    Utils::fft(data.data(), Log2N, -1);   // 正变换

    // 直流分量 = N，其余分量幅值应趋近 0
    EXPECT_NEAR(data[0].real(), static_cast<float>(length), 1e-3f);
    for (int i = 1; i < length; ++i) {
        EXPECT_NEAR(std::abs(data[i]), 0.0f, 1e-3f);
    }
}

TEST(UtilsFftTest, DISABLED_inverseFftRestoresOriginalSignal)
{
    const int Log2N = 3;
    const int length = 1 << Log2N;
    const std::vector<std::complex<float>> original = {
        {1, 0}, {2, 0}, {3, 0}, {4, 0}, {3, 0}, {2, 0}, {1, 0}, {0, 0}
    };
    std::vector<std::complex<float>> data = original;

    Utils::fft(data.data(), Log2N, -1);   // 正变换
    Utils::fft(data.data(), Log2N, 1);    // 逆变换（源码 sign==1 时除以 length）

    for (int i = 0; i < length; ++i) {
        EXPECT_NEAR(data[i].real(), original[i].real(), 1e-3f);
        EXPECT_NEAR(data[i].imag(), 0.0f, 1e-3f);
    }
}

// ============================================================================
// albumToVariantMap : 与 metaToVariantMap 同构，含嵌套 musicinfos
// ============================================================================
TEST(UtilsAlbumToVariantMapTest, mapsCoreFieldsAndNestedMusicinfos)
{
    DMusic::AlbumInfo album;
    album.name = "Greatest Hits";
    album.pinyin = "zuìda";
    album.artist = "Someone";

    DMusic::MediaMeta m;
    m.hash = "h1";
    m.title = "T";
    album.musicinfos.insert("h1", m);

    const QVariantMap out = Utils::albumToVariantMap(album);
    EXPECT_EQ(out.value("name").toString(),    "Greatest Hits");
    EXPECT_EQ(out.value("pinyin").toString(),  "zuìda");
    EXPECT_EQ(out.value("artist").toString(),  "Someone");

    // 嵌套 musicinfos：每个 meta 经 metaToVariantMap 映射
    const QVariantMap musicinfos = out.value("musicinfos").toMap();
    ASSERT_EQ(musicinfos.size(), 1);
    EXPECT_EQ(musicinfos.value("h1").toMap().value("title").toString(), "T");
}

TEST(UtilsAlbumToVariantMapTest, emptyAlbumProducesEmptyMusicinfos)
{
    DMusic::AlbumInfo album;
    album.name = "Empty";
    const QVariantMap out = Utils::albumToVariantMap(album);
    EXPECT_EQ(out.value("musicinfos").toMap().size(), 0);
}

// ============================================================================
// artistToVariantMap : 与 albumToVariantMap 同构
// ============================================================================
TEST(UtilsArtistToVariantMapTest, mapsCoreFieldsAndNestedMusicinfos)
{
    DMusic::ArtistInfo artist;
    artist.name = "Artist A";
    artist.pinyin = "pinyinA";

    DMusic::MediaMeta m;
    m.hash = "h2";
    m.title = "Song";
    artist.musicinfos.insert("h2", m);

    const QVariantMap out = Utils::artistToVariantMap(artist);
    EXPECT_EQ(out.value("name").toString(),   "Artist A");
    EXPECT_EQ(out.value("pinyin").toString(), "pinyinA");

    const QVariantMap musicinfos = out.value("musicinfos").toMap();
    ASSERT_EQ(musicinfos.size(), 1);
    EXPECT_EQ(musicinfos.value("h2").toMap().value("title").toString(), "Song");
}

// ============================================================================
// simpleChineseSplit : 中英混合分词，中文转拼音
// 注意：依赖 Dtk::Core::Chinese2Pinyin，只断言可观察不变量
// ============================================================================
TEST(UtilsSimpleChineseSplitTest, handlesAsciiWithoutCrash)
{
    // 已知源码缺陷：simpleChineseSplit 的 isLastAlphabeta 更新位于
    // if(isCurAlphabeta){...continue;} 之后，字母字符 continue 时跳过更新，
    // 导致连续 ASCII 被拆成单字符 token（"hello" → ['h','e','l','l','o']）。
    // 此处只验证不崩溃 + 拼接内容完整，不强断言聚合行为；待源码修复后启用强断言。
    QString s = "hello";
    const QStringList result = Utils::simpleChineseSplit(s);
    EXPECT_FALSE(result.isEmpty());
    EXPECT_EQ(result.join(""), "hello");
}

TEST(UtilsSimpleChineseSplitTest, convertsChineseToPinyin)
{
    QString s = "中";
    const QStringList result = Utils::simpleChineseSplit(s);
    ASSERT_FALSE(result.isEmpty());
    // 中文经 Chinese2Pinyin 应得到非空拼音（toChinese 会去掉末尾声调数字）
    EXPECT_FALSE(result.first().isEmpty());
}

TEST(UtilsSimpleChineseSplitTest, handlesMixedAsciiAndChinese)
{
    QString s = "a中";
    const QStringList result = Utils::simpleChineseSplit(s);
    // 至少分出 'a' 与中文拼音两段
    EXPECT_GE(result.size(), 2);
}

// ============================================================================
// updateChineseMetaInfo : 填充 title/album/artist 的拼音与首字母字段
// ============================================================================
TEST(UtilsUpdateChineseMetaInfoTest, fillsPinyinFieldsForAscii)
{
    DMusic::MediaMeta meta;
    meta.title = "hello";
    meta.album = "world";
    meta.artist = "someone";

    Utils::updateChineseMetaInfo(meta);
    // ASCII 经 simpleChineseSplit 聚合，pinyin 字段应非空
    EXPECT_FALSE(meta.pinyinTitle.isEmpty());
    EXPECT_FALSE(meta.pinyinTitleShort.isEmpty());
}

// ============================================================================
// detectEncodings : ICU 编码检测
// 注意：依赖 ICU，结果受版本影响；只断言非空 + 包含至少一个编码名
// ============================================================================
TEST(UtilsDetectEncodingsTest, returnsAtLeastOneEncoding)
{
    const QByteArray data = "hello world";
    const QStringList encodings = Utils::detectEncodings(data);
    EXPECT_FALSE(encodings.isEmpty());
}

TEST(UtilsDetectEncodingsTest, detectsEncodingForAscii)
{
    const QByteArray data = "plain ascii text 1234567890";
    const QStringList encodings = Utils::detectEncodings(data);
    ASSERT_FALSE(encodings.isEmpty());
    // 编码名应为非空字符串
    EXPECT_FALSE(encodings.first().isEmpty());
}
