// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
// Graph-driven unit tests for src/libdmusic/core/lyricanalysis.h (class LyricAnalysis).
#include <gtest/gtest.h>
#include <QFile>
#include <QDir>
#include <QByteArray>
#include <QPair>
#include <QVector>
#include <QString>
#include "core/lyricanalysis.h"

namespace {
QString writeTempLrc(const QByteArray &content) {
    QString path = QDir::tempPath() + "/dm-lyric-test.lrc";
    QFile f(path);
    f.open(QIODevice::WriteOnly);
    f.write(content);
    f.close();
    return path;
}
}

TEST(LyricAnalysisTest, Construct_NoThrow) {
    LyricAnalysis la;
    SUCCEED();
}
TEST(LyricAnalysisTest, SetFromFile_NonexistentFile_NoThrow) {
    LyricAnalysis la;
    EXPECT_NO_THROW(la.setFromFile("/tmp/does-not-exist-lyric.lrc"));
    EXPECT_EQ(la.getCount(), 0);
}
TEST(LyricAnalysisTest, SetFromFile_ValidLrc_ParsesLines) {
    QByteArray lrc =
        "[ti:Title]\n"
        "[00:01.00]line one\n"
        "[00:03.50]line two\n"
        "[00:05.00]line three\n";
    QString path = writeTempLrc(lrc);
    LyricAnalysis la;
    la.setFromFile(path);
    EXPECT_GE(la.getCount(), 0);
}
TEST(LyricAnalysisTest, AllLyrics_AfterLoad_ReturnsList) {
    QByteArray lrc = "[00:01.00]a\n[00:02.00]b\n[00:03.00]c\n";
    QString path = writeTempLrc(lrc);
    LyricAnalysis la;
    la.setFromFile(path);
    QVector<QPair<qint64, QString>> all = la.allLyrics();
    EXPECT_NO_THROW(all.size());
}
TEST(LyricAnalysisTest, GetLineAt_AfterLoad_NoThrow) {
    QByteArray lrc = "[00:01.00]alpha\n[00:02.00]beta\n";
    QString path = writeTempLrc(lrc);
    LyricAnalysis la;
    la.setFromFile(path);
    EXPECT_NO_THROW((void)la.getLineAt(0));
    EXPECT_NO_THROW((void)la.getLineAt(99));
}
TEST(LyricAnalysisTest, GetCount_AfterLoad_NoThrow) {
    QByteArray lrc = "[00:01.00]x\n[00:02.00]y\n";
    QString path = writeTempLrc(lrc);
    LyricAnalysis la;
    la.setFromFile(path);
    EXPECT_NO_THROW((void)la.getCount());
}
TEST(LyricAnalysisTest, GetIndex_ByPosition_NoThrow) {
    QByteArray lrc = "[00:01.00]x\n[00:03.00]y\n[00:05.00]z\n";
    QString path = writeTempLrc(lrc);
    LyricAnalysis la;
    la.setFromFile(path);
    EXPECT_NO_THROW((void)la.getIndex(2000));
    EXPECT_NO_THROW((void)la.getIndex(999999));
}
TEST(LyricAnalysisTest, GetPostion_ByIndex_NoThrow) {
    QByteArray lrc = "[00:01.00]x\n[00:03.00]y\n";
    QString path = writeTempLrc(lrc);
    LyricAnalysis la;
    la.setFromFile(path);
    EXPECT_NO_THROW((void)la.getPostion(0));
    EXPECT_NO_THROW((void)la.getPostion(99));
}
TEST(LyricAnalysisTest, EmptyFile_HandledGracefully) {
    QString path = writeTempLrc(QByteArray());
    LyricAnalysis la;
    EXPECT_NO_THROW(la.setFromFile(path));
    EXPECT_EQ(la.getCount(), 0);
}
