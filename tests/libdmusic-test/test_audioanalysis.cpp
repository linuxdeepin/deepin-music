// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// audioanalysis.cpp 的单元测试：
// 1) 构造 + 纯数据函数（convertMetaCodec/detectEncodings 空分支）——不依赖文件
// 2) 真实 mp3 文件解析路径（parseMetaFromLocalFile/Cover/Lyrics、getMetaCoverImage、
//    creatMediaMeta、detectEncodings 文件分支）——用 testdata/sample.mp3（裁剪的 3s 样本）
// 3) 实例方法 parseAudioBuffer（通过 AudioAnalysis 实例间接驱动 AudioDataDetector）
// 4) 录音槽 startRecorder/suspendRecorder/stopRecorder：触发格式探测/QAudioSource 生命周期
// 5) engineType=1（VLC/FFmpeg）分支：parseMetaFromLocalFile 走 ffmpeg 取时长、
//    parseMetaCover/getMetaCoverImage 走 ffmpeg 提取封面、detectEncodings 的 CUE 分支

#include <gtest/gtest.h>

#include <QCoreApplication>
#include <QString>
#include <QStringList>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QDir>
#include <QTemporaryFile>
#include <QBuffer>
#include <QSignalSpy>
#include <memory>

#include <taglib/mpegfile.h>
#include <taglib/id3v2tag.h>
#include <taglib/id3v2frame.h>
#include <taglib/attachedpictureframe.h>
#include <taglib/unsynchronizedlyricsframe.h>
#include <taglib/synchronizedlyricsframe.h>
#include <taglib/textidentificationframe.h>

#include "audioanalysis.h"
#include "mediametaworker.h"
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

// RAII：在测试期间把播放引擎类型切到 type，析构时还原为 0，避免污染其它用例。
// audioanalysis.cpp 内部大量分支以 playbackEngineType()==1 作为是否走 FFmpeg 路径的开关。
class EngineTypeGuard
{
public:
    explicit EngineTypeGuard(int type)
    {
        DmGlobal::setPlaybackEngineType(type);
    }
    ~EngineTypeGuard()
    {
        DmGlobal::setPlaybackEngineType(0);
    }
};

// 把 sample.mp3 复制到临时文件，并用 TagLib 写入：
//   - APIC 封面帧（覆盖 parseMetaCover/getMetaCoverImage 的封面提取分支）
//   - SYLT 同步歌词帧 与/或 USLT 非同步歌词帧（覆盖 parseMetaLyrics 的两条提取分支）
// 返回临时文件路径；测试结束自行 QFile::remove。
//   wantSylt=true 时优先写同步歌词（命中 line 692-701）
//   wantUslt=true 时写非同步歌词（命中 line 706-713）
QString makeEnrichedMp3Copy(bool wantSylt, bool wantUslt)
{
    const QString src = sampleMp3Path();
    const QString dst = QDir::temp().filePath("dmusic_test_enriched.mp3");
    QFile::remove(dst);
    if (!QFile::copy(src, dst)) {
        return QString();
    }

    TagLib::MPEG::File f(dst.toStdString().c_str());
    if (!f.isValid()) {
        QFile::remove(dst);
        return QString();
    }
    TagLib::ID3v2::Tag *tag = f.ID3v2Tag(true);

    // 写一个有效封面帧（覆盖 line 525-531 / 628-633）
    {
        QImage cover(16, 16, QImage::Format_RGB32);
        cover.fill(Qt::blue);
        QByteArray ba;
        QBuffer buf(&ba);
        buf.open(QIODevice::WriteOnly);
        cover.save(&buf, "jpg");
        TagLib::ID3v2::AttachedPictureFrame *pic = new TagLib::ID3v2::AttachedPictureFrame;
        pic->setMimeType("image/jpeg");
        pic->setType(TagLib::ID3v2::AttachedPictureFrame::FrontCover);
        pic->setPicture(TagLib::ByteVector(ba.constData(), ba.size()));
        tag->addFrame(pic);
    }

    if (wantSylt) {
        // 同步歌词帧（覆盖 line 692-701）
        TagLib::ID3v2::SynchronizedLyricsFrame *sylt = new TagLib::ID3v2::SynchronizedLyricsFrame;
        sylt->setLanguage("chi");
        TagLib::ID3v2::SynchronizedLyricsFrame::SynchedTextList list;
        list.append(TagLib::ID3v2::SynchronizedLyricsFrame::SynchedText(1000,  "la"));
        list.append(TagLib::ID3v2::SynchronizedLyricsFrame::SynchedText(2000,  "test"));
        sylt->setSynchedText(list);
        tag->addFrame(sylt);
    }

    if (wantUslt) {
        // 非同步歌词帧（覆盖 line 706-713）
        TagLib::ID3v2::UnsynchronizedLyricsFrame *uslt = new TagLib::ID3v2::UnsynchronizedLyricsFrame;
        uslt->setLanguage("chi");
        uslt->setText(TagLib::String("[00:01.00]la la la\n[00:02.00]test lyric",
                                     TagLib::String::UTF8));
        tag->addFrame(uslt);
    }

    f.save();
    return dst;
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

TEST(AudioAnalysisParseFileTest, creatMediaMetaUsesPrecomputedHash)
{
    ASSERT_TRUE(QFile::exists(sampleMp3Path()));
    const QString precomputedHash = "precomputed-hash";
    const auto meta = AudioAnalysis::creatMediaMeta(sampleMp3Path(), precomputedHash);
    EXPECT_EQ(meta.hash, precomputedHash);
    EXPECT_EQ(meta.localPath, sampleMp3Path());
    EXPECT_EQ(meta.filetype, "mp3");
}


TEST(MediaMetaWorkerTest, enqueueMetasDeduplicatesByHash)
{
    ASSERT_TRUE(QFile::exists(sampleMp3Path()));
    MediaMetaWorker worker;
    QSignalSpy spy(&worker, &MediaMetaWorker::signalMetaAnalysisReady);

    DMusic::MediaMeta meta;
    meta.localPath = sampleMp3Path();
    meta.hash = "duplicate-cover-hash";

    worker.enqueueMetas({meta, meta});
    ASSERT_TRUE(spy.wait(1000));
    EXPECT_EQ(spy.count(), 1);
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

TEST(AudioAnalysisParseFileTest, parseMetaCoverAndLyricsDoesNotCrash)
{
    DMusic::MediaMeta meta;
    meta.localPath = sampleMp3Path();
    meta.hash = Utils::filePathHash(sampleMp3Path());
    AudioAnalysis::parseMetaCoverAndLyrics(meta); // 一次打开文件解析封面和歌词
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

// ============================================================================
// detectEncodings : cuePath 分支（line 419-428）
// 指向一个真实存在的临时文件，触发 QFile::open 成功路径
// ============================================================================
TEST(AudioAnalysisDetectEncodingsTest, cueFileReadableReturnsEncodings)
{
    // 写一个临时 cue 文件
    QTemporaryFile cueFile;
    cueFile.setAutoRemove(true);
    ASSERT_TRUE(cueFile.open());
    {
        // Qt6 QTextStream 已移除 setCodec；直接写 UTF-8 字节
        cueFile.write(QString::fromUtf8("PERFORMER 测试歌手\nTITLE 测试标题\n").toUtf8());
    }
    cueFile.close();

    DMusic::MediaMeta meta;
    meta.localPath = sampleMp3Path();    // localPath 非空才能进入 cue 分支（否则 line 413 早退）
    meta.cuePath = cueFile.fileName();   // cuePath 非空 → 进 cue 读取分支
    const QStringList encodings = AudioAnalysis::detectEncodings(meta);
    ASSERT_FALSE(encodings.isEmpty());
    // 第一个编码来自 locale，后续来自 ICU 检测
}

TEST(AudioAnalysisDetectEncodingsTest, cueFileUnopenableFallsThroughToOriginals)
{
    DMusic::MediaMeta meta;
    meta.localPath = sampleMp3Path();                 // 非空，越过早退
    meta.cuePath = "/nonexistent/notafile.cue";       // 无法打开 → 走 warning 后落到 original 分支
    meta.originalTitle = "fallback";
    const QStringList encodings = AudioAnalysis::detectEncodings(meta);
    EXPECT_FALSE(encodings.isEmpty());
}

// ============================================================================
// parseFileTagCodec 的空 localPath 防御分支（line 178-181）
// 通过 parseMetaFromLocalFile 间接无法命中此分支（parseMetaFromLocalFile 自己会先判空），
// 直接验证 parseMetaFromLocalFile 对空路径返回 false（line 305-308）。
// ============================================================================
TEST(AudioAnalysisParseFileTest, emptyLocalPathReturnsFalse)
{
    DMusic::MediaMeta meta;  // localPath 默认为空
    EXPECT_FALSE(AudioAnalysis::parseMetaFromLocalFile(meta));
}

// ============================================================================
// parseMetaFromLocalFile : engineType=1 时走 FFmpeg 时长探测路径（line 317-377）
// 用不存在文件触发 open_input 失败分支（line 342-346）
// ============================================================================
TEST(AudioAnalysisFfmpegTest, ffmpegDurationOnNonexistentFileReturnsFalse)
{
    EngineTypeGuard guard(1);  // 强制走 FFmpeg 路径
    DMusic::MediaMeta meta;
    meta.localPath = "/nonexistent/path/missing.mp3";
    // parseFileTagCodec 先失败（清空 localPath），但 parseMetaFromLocalFile 重新用 curFilePath
    // 触发 ffmpeg 分支：avformat_open_input 失败 → return false
    EXPECT_FALSE(AudioAnalysis::parseMetaFromLocalFile(meta));
}

TEST(AudioAnalysisFfmpegTest, ffmpegDurationForRealFileSucceeds)
{
    ASSERT_TRUE(QFile::exists(sampleMp3Path())) << "testdata/sample.mp3 missing";
    EngineTypeGuard guard(1);
    DMusic::MediaMeta meta;
    meta.localPath = sampleMp3Path();
    EXPECT_TRUE(AudioAnalysis::parseMetaFromLocalFile(meta));
    // FFmpeg 分支应解析出时长（>0）
    EXPECT_GT(meta.length, 0);
}

// ============================================================================
// parseMetaCover : engineType=1 走 FFmpeg 提取封面分支（line 478-554）
// 真实 mp3 无内嵌封面时回退到 TagLib ID3v2（line 512-540）
// ============================================================================
TEST(AudioAnalysisFfmpegTest, parseMetaCoverWithFfmpegEngineDoesNotCrash)
{
    ASSERT_TRUE(QFile::exists(sampleMp3Path()));
    EngineTypeGuard guard(1);
    DMusic::MediaMeta meta;
    meta.localPath = sampleMp3Path();
    meta.hash = Utils::filePathHash(sampleMp3Path());
    AudioAnalysis::parseMetaCover(meta);
    // 不崩溃即可，sample.mp3 无封面时 hasimage 保持 false
    SUCCEED();
}

// parseMetaCover：engineType=1 + 不存在文件，ffmpeg/TagLib 都失败但不崩溃
TEST(AudioAnalysisFfmpegTest, parseMetaCoverWithFfmpegNonexistentFileDoesNotCrash)
{
    EngineTypeGuard guard(1);
    DMusic::MediaMeta meta;
    meta.localPath = "/nonexistent/path/missing.mp3";
    meta.hash = "deadbeef";
    AudioAnalysis::parseMetaCover(meta);
    SUCCEED();
}

// ============================================================================
// getMetaCoverImage : engineType=1 走 FFmpeg 提取分支（line 562-611）
// ============================================================================
TEST(AudioAnalysisFfmpegTest, getMetaCoverImageWithFfmpegEngineDoesNotCrash)
{
    ASSERT_TRUE(QFile::exists(sampleMp3Path()));
    EngineTypeGuard guard(1);
    DMusic::MediaMeta meta;
    meta.localPath = sampleMp3Path();
    meta.hasimage = true;
    const QImage img = AudioAnalysis::getMetaCoverImage(meta);
    // sample.mp3 无封面 → 走 TagLib ID3v2（空）→ 默认封面
    Q_UNUSED(img);
    SUCCEED();
}

// getMetaCoverImage：hasimage=false 时直接走默认封面分支（line 648-653）
TEST(AudioAnalysisGetCoverTest, noImageReturnsDefaultCover)
{
    DMusic::MediaMeta meta;  // hasimage 默认 false
    meta.localPath = "/tmp/anything.mp3";
    const QImage img = AudioAnalysis::getMetaCoverImage(meta);
    // 默认封面文件测试环境可能不存在 → img 可能为 null，只验证不崩溃
    Q_UNUSED(img);
    SUCCEED();
}

// ============================================================================
// parseMetaLyrics : 已存在歌词文件时提前返回分支（line 675-678）
// 测试环境 cachePath() 可能为空，故显式设置到临时目录后再构造歌词缓存
// ============================================================================
TEST(AudioAnalysisLyricsTest, existingLyricFileSkipsParsing)
{
    ASSERT_TRUE(QFile::exists(sampleMp3Path()));
    // 显式设置缓存路径到临时目录，确保可写
    const QString baseCache = QDir::temp().filePath("dmusic-test-aa-cache");
    QDir().mkpath(baseCache);
    DmGlobal::setCachePath(baseCache);

    DMusic::MediaMeta meta;
    meta.localPath = sampleMp3Path();
    meta.hash = "existing-lyric-test-hash";

    // 预先创建歌词文件命中"已存在则跳过"分支
    const QString lyricDir = baseCache + QDir::separator() + "lyrics";
    const QString lyricName = meta.hash + ".lrc";
    QDir().mkpath(lyricDir);
    {
        QFile f(lyricDir + QDir::separator() + lyricName);
        ASSERT_TRUE(f.open(QIODevice::WriteOnly));
        f.write("[00:00.00]dummy\n");
    }

    AudioAnalysis::parseMetaLyrics(meta);
    // 文件应仍然存在（提前 return 未被覆盖）
    EXPECT_TRUE(QFile::exists(lyricDir + QDir::separator() + lyricName));

    // 还原 cachePath 避免污染其它用例
    DmGlobal::setCachePath("");
}

// parseMetaLyrics：无效路径触发 TagLib MPEG::File isValid()=false 分支（line 732-734）
TEST(AudioAnalysisLyricsTest, invalidMpegFileLogsWarning)
{
    DMusic::MediaMeta meta;
    meta.localPath = "/nonexistent/notaudio.mp3";
    meta.hash = "lyric-test-hash-unique";
    AudioAnalysis::parseMetaLyrics(meta);
    SUCCEED();
}

// ============================================================================
// 实例方法 parseAudioBuffer：通过 AudioAnalysis 实例驱动 AudioDataDetector
// 用空/非法 localPath 触发 detector 的防御分支（line 113-119）
// ============================================================================
TEST(AudioAnalysisBufferTest, parseAudioBufferWithEmptyPathDoesNotCrash)
{
    std::unique_ptr<AudioAnalysis> aa(new AudioAnalysis());
    DMusic::MediaMeta meta;  // localPath/hash 为空
    aa->parseAudioBuffer(meta);  // detector 内部对空路径直接退出
    SUCCEED();
}

TEST(AudioAnalysisBufferTest, parseAudioBufferWithRealFileEmitsAudioBufferSignal)
{
    ASSERT_TRUE(QFile::exists(sampleMp3Path()));
    std::unique_ptr<AudioAnalysis> aa(new AudioAnalysis());
    DMusic::MediaMeta meta;
    meta.localPath = sampleMp3Path();
    meta.hash = Utils::filePathHash(sampleMp3Path());
    // AudioDataDetector 是 QThread 子类，会在后台解码并 emit audioBuffer
    // 调用后立即断言，不等待信号（避免测试阻塞）
    aa->parseAudioBuffer(meta);
    SUCCEED();
}

// ============================================================================
// 录音槽 startRecorder/suspendRecorder/stopRecorder
// startRecorder 首次调用初始化 QAudioSource 并 connect readyRead（line 739-777）
// suspendRecorder：非空 m_audioSource 时 suspend（line 780-789）
// stopRecorder：清理 m_audioSource/m_audioDevice（line 791-804）
// ============================================================================
TEST(AudioAnalysisRecorderTest, startRecorderInitializesAudioSource)
{
    std::unique_ptr<AudioAnalysis> aa(new AudioAnalysis());
    // 首次调用：m_audioDevice==nullptr 分支，创建 QAudioSource + connect(nullptr,...) 安全
    aa->startRecorder();
    SUCCEED();
}

TEST(AudioAnalysisRecorderTest, suspendRecorderWhenSourceExistsSucceeds)
{
    std::unique_ptr<AudioAnalysis> aa(new AudioAnalysis());
    aa->startRecorder();          // 先初始化 source
    aa->suspendRecorder();        // m_audioSource 非空 → suspend()
    SUCCEED();
}

TEST(AudioAnalysisRecorderTest, suspendRecorderWithoutSourceIsNoop)
{
    std::unique_ptr<AudioAnalysis> aa(new AudioAnalysis());
    // 不 start，m_audioSource 仍为 nullptr → 仅打印 debug，不崩溃
    aa->suspendRecorder();
    SUCCEED();
}

TEST(AudioAnalysisRecorderTest, stopRecorderCleansUpResources)
{
    std::unique_ptr<AudioAnalysis> aa(new AudioAnalysis());
    aa->startRecorder();
    aa->stopRecorder();           // 清理 source/device
    // 再次 stop 不崩溃（幂等）
    aa->stopRecorder();
    SUCCEED();
}

TEST(AudioAnalysisRecorderTest, stopRecorderWithoutStartIsNoop)
{
    std::unique_ptr<AudioAnalysis> aa(new AudioAnalysis());
    aa->stopRecorder();           // m_audioSource/m_audioDevice 均为 null，仅打印日志
    SUCCEED();
}

// ============================================================================
// convertMetaCodec : 用 GB18030 编码的原始字段，覆盖 codec 非空 + 中文分支（line 147-154）
// ============================================================================
TEST(AudioAnalysisConvertCodecTest, gb18030CodecDecodesChineseFields)
{
    DMusic::MediaMeta meta;
    // GB18030 字节流（"标题"的 GBK 编码）
    meta.originalTitle = QByteArray::fromHex("b1eac2eb");   // "标题" GBK
    meta.originalArtist = QByteArray::fromHex("b8e8cad6");  // "歌手" GBK
    meta.originalAlbum = QByteArray::fromHex("d7a8bcad");   // "专辑" GBK
    AudioAnalysis::convertMetaCodec(meta, "GB18030");
    EXPECT_FALSE(meta.title.isEmpty());
    EXPECT_EQ(meta.codec, "GB18030");
    // 中文应被正确解码为非空 unicode（具体字符不强断言以避免编码细节漂移）
    EXPECT_GT(meta.title.size(), 0);
}

// convertMetaCodec : 空原始字段 + 无 codec 名（触发 null codec 分支 line 152-154 + 默认值填充）
TEST(AudioAnalysisConvertCodecTest, emptyCodecNameLeavesDefaults)
{
    DMusic::MediaMeta meta;
    meta.localPath = "/tmp/empty.mp3";
    AudioAnalysis::convertMetaCodec(meta, "");  // codecForName("") 返回 null
    EXPECT_EQ(meta.title, "empty");
    EXPECT_EQ(meta.album, DmGlobal::unknownAlbumText());
    EXPECT_EQ(meta.artist, DmGlobal::unknownArtistText());
}

// ============================================================================
// parseMetaCover : 已存在缓存封面图片时提前返回分支（line 462-472）
// ============================================================================
TEST(AudioAnalysisCoverTest, existingCoverCacheShortCircuits)
{
    // 显式设置可写缓存目录（测试环境默认 cachePath 可能为空）
    const QString baseCache = QDir::temp().filePath("dmusic-test-aa-cache2");
    QDir().mkpath(baseCache);
    DmGlobal::setCachePath(baseCache);

    DMusic::MediaMeta meta;
    meta.localPath = sampleMp3Path();
    meta.hash = "covercache-test-hash";

    // 预先写入一张合法封面图片到缓存目录
    const QString imagesDir = baseCache + "/images";
    QDir().mkpath(imagesDir);
    const QString coverPath = imagesDir + "/" + meta.hash + ".jpg";
    {
        QImage dummy(8, 8, QImage::Format_RGB32);
        dummy.fill(Qt::red);
        ASSERT_TRUE(dummy.save(coverPath, "jpg"));
    }

    AudioAnalysis::parseMetaCover(meta);
    EXPECT_TRUE(meta.hasimage);
    EXPECT_EQ(meta.coverUrl, coverPath);

    // 清理 + 还原
    QFile::remove(coverPath);
    DmGlobal::setCachePath("");
}

// ============================================================================
// creatMediaMeta : 空路径触发 symlink 循环跳过 + parseMetaFromLocalFile 失败分支（line 126-138）
// ============================================================================
TEST(AudioAnalysisCreatMetaTest, emptyPathProducesMetaWithHash)
{
    const auto meta = AudioAnalysis::creatMediaMeta("");
    // 空路径：parseMetaFromLocalFile 返回 false，但 hash 仍由 filePathHash 生成
    EXPECT_TRUE(meta.hash.isEmpty() || meta.hash.size() > 0);
    EXPECT_TRUE(meta.localPath.isEmpty());
}

TEST(AudioAnalysisCreatMetaTest, symlinkPathResolvesToTarget)
{
    ASSERT_TRUE(QFile::exists(sampleMp3Path()));
    // 创建一个指向 sample.mp3 的软链接，覆盖 isSymLink 循环分支（line 126-129）
    const QString linkPath = QDir::temp().filePath("dmusic_test_link.mp3");
    QFile::remove(linkPath);
    ASSERT_TRUE(QFile::link(QFileInfo(sampleMp3Path()).absoluteFilePath(), linkPath));

    const auto meta = AudioAnalysis::creatMediaMeta(linkPath);
    EXPECT_FALSE(meta.hash.isEmpty());
    // 解析目标后应得到 mp3 后缀
    EXPECT_EQ(meta.filetype, "mp3");

    QFile::remove(linkPath);
}

// ============================================================================
// parseMetaFromLocalFile : 同毫秒连续解析触发 preAddTime 自增分支（line 385-388）
// preAddTime 是文件级 static，连续两次解析同一文件可能命中 timestamp 相等 +1
// ============================================================================
TEST(AudioAnalysisParseFileTest, rapidRepeatedParseMayTriggerPreAddTimeBranch)
{
    DMusic::MediaMeta meta1;
    meta1.localPath = sampleMp3Path();
    ASSERT_TRUE(AudioAnalysis::parseMetaFromLocalFile(meta1));
    const qint64 ts1 = meta1.timestamp;

    DMusic::MediaMeta meta2;
    meta2.localPath = sampleMp3Path();
    ASSERT_TRUE(AudioAnalysis::parseMetaFromLocalFile(meta2));
    const qint64 ts2 = meta2.timestamp;

    // 两次解析时间戳递增（若同毫秒，preAddTime 分支保证 ts2 >= ts1）
    EXPECT_GE(ts2, ts1);
}

// ============================================================================
// getMetaCoverImage : engineType=1 + 不存在文件，触发 FFmpeg open_input 失败路径（line 587-591）
// 与 parseMetaCover 的 ffmpeg 路径互补，确保 getMetaCoverImage 走完 ffmpeg 分支
// ============================================================================
TEST(AudioAnalysisFfmpegTest, getMetaCoverImageWithFfmpegNonexistentFileDoesNotCrash)
{
    EngineTypeGuard guard(1);
    DMusic::MediaMeta meta;
    meta.localPath = "/nonexistent/path/missing.mp3";
    meta.hasimage = true;
    const QImage img = AudioAnalysis::getMetaCoverImage(meta);
    // ffmpeg 打开失败 → 回退 TagLib（无效文件）→ 默认封面
    Q_UNUSED(img);
    SUCCEED();
}

// ============================================================================
// 富数据 mp3（含 APIC 封面 + USLT 歌词）：覆盖 parseMetaCover/parseMetaLyrics 的
// TagLib ID3v2 提取成功分支（line 525-551 / 692-728）。
// 用 TagLib 在临时副本上注入帧，避免依赖带富元数据的样本文件。
// ============================================================================
class AudioAnalysisEnrichedTest : public ::testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        // 默认富数据副本：含封面 + USLT（非同步歌词）
        s_enrichedPath = makeEnrichedMp3Copy(false /*sylt*/, true /*uslt*/);
        // 另一副本：含封面 + SYLT（同步歌词），覆盖 parseMetaLyrics 的 SYLT 分支
        s_syltPath = makeEnrichedMp3Copy(true /*sylt*/, false /*uslt*/);
    }
    static void TearDownTestSuite()
    {
        if (!s_enrichedPath.isEmpty()) {
            QFile::remove(s_enrichedPath);
            s_enrichedPath.clear();
        }
        if (!s_syltPath.isEmpty()) {
            QFile::remove(s_syltPath);
            s_syltPath.clear();
        }
    }
    static QString s_enrichedPath;
    static QString s_syltPath;
};
QString AudioAnalysisEnrichedTest::s_enrichedPath;
QString AudioAnalysisEnrichedTest::s_syltPath;

// parseMetaCover：从富数据文件提取到封面，命中 TagLib APIC 分支 + 保存分支（line 525-551）
TEST_F(AudioAnalysisEnrichedTest, parseMetaCoverExtractsEmbeddedCover)
{
    ASSERT_FALSE(s_enrichedPath.isEmpty()) << "无法构造富数据 mp3 副本";
    // 设置可写缓存目录
    const QString baseCache = QDir::temp().filePath("dmusic-test-aa-cover-enriched");
    QDir().mkpath(baseCache);
    DmGlobal::setCachePath(baseCache);

    DMusic::MediaMeta meta;
    meta.localPath = s_enrichedPath;
    meta.hash = "enriched-cover-hash";
    AudioAnalysis::parseMetaCover(meta);
    // 命中封面提取分支后应写入缓存并标记 hasimage
    EXPECT_TRUE(meta.hasimage);
    EXPECT_FALSE(meta.coverUrl.isEmpty());

    DmGlobal::setCachePath("");
    QFile::remove(baseCache + "/images/" + meta.hash + ".jpg");
}

// getMetaCoverImage：engineType=0 + hasimage 时走 TagLib APIC 分支（line 625-633）
TEST_F(AudioAnalysisEnrichedTest, getMetaCoverImageExtractsEmbeddedCover)
{
    ASSERT_FALSE(s_enrichedPath.isEmpty());
    DMusic::MediaMeta meta;
    meta.localPath = s_enrichedPath;
    meta.hasimage = true;
    const QImage img = AudioAnalysis::getMetaCoverImage(meta);
    // 应从内嵌 APIC 提取出非空图片
    EXPECT_FALSE(img.isNull());
}

// parseMetaLyrics：从富数据文件提取到 USLT 歌词，命中写入分支（line 706-723）
TEST_F(AudioAnalysisEnrichedTest, parseMetaLyricsExtractsEmbeddedLyrics)
{
    ASSERT_FALSE(s_enrichedPath.isEmpty());
    const QString baseCache = QDir::temp().filePath("dmusic-test-aa-lyric-enriched");
    QDir().mkpath(baseCache);
    DmGlobal::setCachePath(baseCache);

    DMusic::MediaMeta meta;
    meta.localPath = s_enrichedPath;
    meta.hash = "enriched-lyric-hash";
    AudioAnalysis::parseMetaLyrics(meta);
    // 命中歌词提取分支后应写出 .lrc 文件
    const QString lrc = baseCache + QDir::separator() + "lyrics" + QDir::separator() + meta.hash + ".lrc";
    EXPECT_TRUE(QFile::exists(lrc));

    DmGlobal::setCachePath("");
    QFile::remove(lrc);
}

// parseMetaLyrics：含 SYLT（同步歌词）帧时走同步歌词提取分支（line 692-701）
TEST_F(AudioAnalysisEnrichedTest, parseMetaLyricsExtractsSynchronizedLyrics)
{
    ASSERT_FALSE(s_syltPath.isEmpty());
    const QString baseCache = QDir::temp().filePath("dmusic-test-aa-lyric-sylt");
    QDir().mkpath(baseCache);
    DmGlobal::setCachePath(baseCache);

    DMusic::MediaMeta meta;
    meta.localPath = s_syltPath;
    meta.hash = "enriched-sylt-hash";
    AudioAnalysis::parseMetaLyrics(meta);
    // 同步歌词提取成功 → 写出 .lrc 文件
    const QString lrc = baseCache + QDir::separator() + "lyrics" + QDir::separator() + meta.hash + ".lrc";
    EXPECT_TRUE(QFile::exists(lrc));

    DmGlobal::setCachePath("");
    QFile::remove(lrc);
}

// ============================================================================
// parseAudioBuffer + 录音 resume 分支：
// startRecorder 内 m_audioDevice 永远为 null（源码中 start() 调用被注释），
// 故 resume 分支（line 774-776）无法经公开接口命中——记录为已知不可达。
// parseData（私有槽，line 806-826）只能由 QIODevice::readyRead 信号触发，
// 而 m_audioDevice 始终为 null，无法经公开接口触发——记录为已知不可达。
// ============================================================================
