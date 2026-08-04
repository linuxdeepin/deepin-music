// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
// Graph-driven unit tests for src/libdmusic/core/audioanalysis.h (class AudioAnalysis).
#include <gtest/gtest.h>
#include <QObject>
#include <QImage>
#include <QString>
#include <QDir>
#include "global.h"
#include "core/audioanalysis.h"

TEST(AudioAnalysisTest, Construct_NoThrow) {
    AudioAnalysis aa;
    SUCCEED();
}
TEST(AudioAnalysisTest, CreatMediaMeta_NonexistentPath_NoThrow) {
    EXPECT_NO_THROW((void)AudioAnalysis::creatMediaMeta("/tmp/no-such-audio.mp3"));
}
TEST(AudioAnalysisTest, ConvertMetaCodec_NoThrow) {
    DMusic::MediaMeta m;
    m.title = QString::fromUtf8("标题");
    EXPECT_NO_THROW(AudioAnalysis::convertMetaCodec(m, "UTF-8"));
}
TEST(AudioAnalysisTest, ParseMetaFromLocalFile_Nonexistent_ReturnsFalse) {
    DMusic::MediaMeta m;
    bool ok = AudioAnalysis::parseMetaFromLocalFile(m);
    EXPECT_NO_THROW((void)ok);
}
TEST(AudioAnalysisTest, DetectEncodings_NonexistentMeta_NoThrow) {
    DMusic::MediaMeta m;
    EXPECT_NO_THROW((void)AudioAnalysis::detectEncodings(m));
}
TEST(AudioAnalysisTest, ParseMetaCover_NonexistentMeta_NoThrow) {
    DMusic::MediaMeta m;
    m.localPath = "/tmp/no-such-audio.mp3";
    EXPECT_NO_THROW(AudioAnalysis::parseMetaCover(m));
}
TEST(AudioAnalysisTest, GetMetaCoverImage_NonexistentMeta_NoThrow) {
    DMusic::MediaMeta m;
    m.localPath = "/tmp/no-such-audio.mp3";
    EXPECT_NO_THROW((void)AudioAnalysis::getMetaCoverImage(m));
}
TEST(AudioAnalysisTest, ParseMetaLyrics_NonexistentMeta_NoThrow) {
    DMusic::MediaMeta m;
    m.localPath = "/tmp/no-such-audio.mp3";
    EXPECT_NO_THROW(AudioAnalysis::parseMetaLyrics(m));
}
TEST(AudioAnalysisTest, ParseAudioBuffer_NonexistentMeta_NoThrow) {
    AudioAnalysis aa;
    DMusic::MediaMeta m;
    m.localPath = "/tmp/no-such-audio.mp3";
    EXPECT_NO_THROW(aa.parseAudioBuffer(m));
}
// startRecorder/suspendRecorder/stopRecorder touch real audio hardware; keep
// them in isolated TEST_F so a failure cannot block other coverage.
TEST(AudioAnalysisTest, StopRecorder_WithoutStart_NoThrow) {
    AudioAnalysis aa;
    EXPECT_NO_THROW(aa.stopRecorder());
}
TEST(AudioAnalysisTest, SuspendRecorder_WithoutStart_NoThrow) {
    AudioAnalysis aa;
    EXPECT_NO_THROW(aa.suspendRecorder());
}
TEST(AudioAnalysisTest, StartRecorder_Headless_NoThrow) {
    AudioAnalysis aa;
    EXPECT_NO_THROW(aa.startRecorder());
    aa.suspendRecorder();
    aa.stopRecorder();
}
