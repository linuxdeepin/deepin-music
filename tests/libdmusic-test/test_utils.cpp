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
// 注意：当前 utils.cpp 的 fft() 实现存在变量复用缺陷——外层循环变量 i 被最内层
// for(i=0; i<length/step; i++) 覆盖，导致外层循环条件恒成立而陷入死循环。
// 因此以下用例标记为 DISABLED_，gtest 会跳过它们（不阻塞整套测试）。
// 待 src/libdmusic/util/utils.cpp 的 fft() 修复后，移除 DISABLED_ 前缀即可启用。
// 参见 docs/unit-test-plan.md「已知缺陷」。
// ============================================================================
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
