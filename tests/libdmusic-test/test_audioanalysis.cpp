// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// audioanalysis.cpp 的单元测试：
// 1) 构造 + 纯数据函数（convertMetaCodec/detectEncodings 空分支）——不依赖文件
// 2) 真实 mp3 文件解析路径（parseMetaFromLocalFile/Cover/Lyrics、getMetaCoverImage、
//    creatMediaMeta、detectEncodings 文件分支）——用 testdata/sample.mp3（裁剪的 3s 样本）

#include <gtest/gtest.h>

#include <QCoreApplication>
#include <QString>
#include <QStringList>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <memory>

#include "audioanalysis.h"
#include "global.h"
#include "utils.h"

#ifndef TEST_DATA_DIR
#define TEST_DATA_DIR "."
#endif

namespace {
QString sampleMp3Path()
{
    return QString(TEST_DATA_DIR) + "/sample.mp3";
}
}

// ============================================================================
// 构造/析构：不崩溃
// ============================================================================
TEST(AudioAnalysisConstructionTest, constructsAndDestroysWithoutCrash)
{
    std::unique_ptr<AudioAnalysis> aa(new AudioAnalysis());
    EXPECT_NE(aa.get(), nullptr);
}

// ============================================================================
// convertMetaCodec : 用指定 codec 解码 original 字段，填充 title/artist/album
// ============================================================================
TEST(AudioAnalysisConvertCodecTest, decodesOriginalFieldsWithUtf8)
{
    DMusic::MediaMeta meta;
    meta.originalTitle = "Title";
    meta.originalArtist = "Artist";
    meta.originalAlbum = "Album";
    AudioAnalysis::convertMetaCodec(meta, "UTF-8");
    EXPECT_EQ(meta.title, "Title");
    EXPECT_EQ(meta.artist, "Artist");
    EXPECT_EQ(meta.album, "Album");
    EXPECT_EQ(meta.codec, "UTF-8");
}

TEST(AudioAnalysisConvertCodecTest, fillsDefaultsForEmptyFields)
{
    DMusic::MediaMeta meta;
    meta.localPath = "/tmp/test.mp3";
    AudioAnalysis::convertMetaCodec(meta, "UTF-8");
    EXPECT_EQ(meta.title, "test");
    EXPECT_EQ(meta.album, DmGlobal::unknownAlbumText());
    EXPECT_EQ(meta.artist, DmGlobal::unknownArtistText());
}

TEST(AudioAnalysisConvertCodecTest, invalidCodecFallsBackGracefully)
{
    DMusic::MediaMeta meta;
    meta.localPath = "/tmp/x.mp3";
    AudioAnalysis::convertMetaCodec(meta, "NONEXISTENT-CODEC-XYZ");
    EXPECT_EQ(meta.title, "x");
}

// ============================================================================
// detectEncodings : 空路径返回 UTF-8
// ============================================================================
TEST(AudioAnalysisDetectEncodingsTest, emptyPathReturnsUtf8)
{
    DMusic::MediaMeta meta;  // localPath 空
    const QStringList encodings = AudioAnalysis::detectEncodings(meta);
    ASSERT_EQ(encodings.size(), 1);
    EXPECT_EQ(encodings.first(), "UTF-8");
}

// ============================================================================
// 以下用真实 mp3（testdata/sample.mp3）测试文件解析路径
// 覆盖 parseMetaFromLocalFile（taglib+ffmpeg）/parseMetaCover/parseMetaLyrics/
// getMetaCoverImage/creatMediaMeta/detectEncodings 的文件分支
// ============================================================================
TEST(AudioAnalysisParseFileTest, parseMetaFromLocalFileExtractsMetadata)
{
    ASSERT_TRUE(QFile::exists(sampleMp3Path())) << "testdata/sample.mp3 missing";
    DMusic::MediaMeta meta;
    meta.localPath = sampleMp3Path();
    EXPECT_TRUE(AudioAnalysis::parseMetaFromLocalFile(meta));
    EXPECT_GT(meta.length, 0);          // 解析出时长
    EXPECT_EQ(meta.filetype, "mp3");
}

TEST(AudioAnalysisParseFileTest, creatMediaMetaBuildsHashAndFiletype)
{
    ASSERT_TRUE(QFile::exists(sampleMp3Path()));
    const auto meta = AudioAnalysis::creatMediaMeta(sampleMp3Path());
    EXPECT_FALSE(meta.hash.isEmpty());          // filePathHash 生成
    EXPECT_EQ(meta.localPath, sampleMp3Path());
    EXPECT_EQ(meta.filetype, "mp3");
}

TEST(AudioAnalysisParseFileTest, parseMetaCoverDoesNotCrash)
{
    DMusic::MediaMeta meta;
    meta.localPath = sampleMp3Path();
    meta.hash = Utils::filePathHash(sampleMp3Path());
    AudioAnalysis::parseMetaCover(meta);        // 提取/生成封面，不崩溃即可
    SUCCEED();
}

TEST(AudioAnalysisParseFileTest, parseMetaLyricsDoesNotCrash)
{
    DMusic::MediaMeta meta;
    meta.localPath = sampleMp3Path();
    meta.hash = Utils::filePathHash(sampleMp3Path());
    AudioAnalysis::parseMetaLyrics(meta);       // 提取歌词（可能无），不崩溃即可
    SUCCEED();
}

TEST(AudioAnalysisParseFileTest, getMetaCoverImageDoesNotCrash)
{
    DMusic::MediaMeta meta;
    meta.localPath = sampleMp3Path();
    const QImage img = AudioAnalysis::getMetaCoverImage(meta);
    // 无封面时回退 cachePath/images/default_cover.png；测试环境该图可能不存在
    // → img 可能为 null，只验证不崩溃，不强断言 non-null
    Q_UNUSED(img);
    SUCCEED();
}

TEST(AudioAnalysisParseFileTest, detectEncodingsForRealFile)
{
    DMusic::MediaMeta meta;
    meta.localPath = sampleMp3Path();
    meta.originalTitle = "test title";
    const QStringList encodings = AudioAnalysis::detectEncodings(meta);
    EXPECT_FALSE(encodings.isEmpty());
}

TEST(AudioAnalysisParseFileTest, nonExistentFileHandledGracefully)
{
    DMusic::MediaMeta meta;
    meta.localPath = "/nonexistent/path/file.mp3";
    // 源码设计：parseFileTagCodec 失败只 warning 不终止，parseMetaFromLocalFile 仍返回 true
    // （meta 用文件名/默认值填充）。验证不崩溃 + 后缀仍解析。
    EXPECT_TRUE(AudioAnalysis::parseMetaFromLocalFile(meta));
    EXPECT_EQ(meta.filetype, "mp3");
}

// ============================================================================
// detectEncodings : 有 originalTitle 的 meta（走 Utils::detectEncodings 路径）
// 覆盖 detectEncodings 的非空 original 字段分支（line 431-437）
// ============================================================================
TEST(AudioAnalysisDetectEncodingsTest, detectsEncodingsForMetaWithOriginalFields)
{
    DMusic::MediaMeta meta;
    meta.originalTitle = "测试标题";
    meta.originalArtist = "artist";
    meta.originalAlbum = "album";
    const QStringList encodings = AudioAnalysis::detectEncodings(meta);
    EXPECT_FALSE(encodings.isEmpty());
}
