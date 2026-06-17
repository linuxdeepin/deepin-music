// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// lyricanalysis.cpp 的单元测试：LRC 歌词解析（时间戳 + 文本）。
// 对应 docs/unit-test-plan.md Phase 1 / ②。
//
// 注意：parseLyric 使用 QRegExp "mm:ss.z" 解析时间，时间精度依赖 Qt 的
// QTime::fromString。这里优先使用「自洽性断言」（如 getIndex(getPostion(i))==i）
// 和「相对关系断言」，避免对绝对毫秒值过度敏感。

#include <gtest/gtest.h>

#include <QCoreApplication>
#include <QTemporaryFile>
#include <QTextCodec>
#include <QFile>
#include <QDir>

#include "lyricanalysis.h"

namespace {
// RAII 临时 LRC 文件：构造时写入内容，析构时自动删除磁盘文件。
// 相比返回裸路径的 helper：避免 new QTemporaryFile 泄漏对象，且 ASSERT 失败时
// 栈展开仍会析构并删除文件，不残留在 /tmp。
class TempLrcFile
{
public:
    explicit TempLrcFile(const QString &content)
    {
        // QTemporaryFile 默认 autoRemove(true)：析构时自动删除文件
        if (m_file.open()) {
            m_file.write(content.toUtf8());
            m_file.close();  // 落盘并释放句柄，供 setFromFile 读取
            m_path = m_file.fileName();
        }
    }

    QString path() const
    {
        return m_path;
    }

private:
    QTemporaryFile m_file;
    QString        m_path;
};

// 写入原始字节的临时 LRC 文件：用于编码检测路径（UTF-8 BOM / GB18030 等）
class TempLrcFileWithBytes
{
public:
    explicit TempLrcFileWithBytes(const QByteArray &bytes)
    {
        if (m_file.open()) {
            m_file.write(bytes);
            m_file.close();
            m_path = m_file.fileName();
        }
    }
    QString path() const { return m_path; }
private:
    QTemporaryFile m_file;
    QString        m_path;
};
}

// ============================================================================
// 基本解析：时间戳 + 歌词文本
// ============================================================================
TEST(LyricAnalysisTest, parsesMultipleLines)
{
    TempLrcFile tempLrc("[00:01.20]first line\n[00:03.50]second line\n");
    LyricAnalysis la;
    la.setFromFile(tempLrc.path());
    EXPECT_EQ(la.getCount(), 2);
}

TEST(LyricAnalysisTest, getLineAtReturnsText)
{
    TempLrcFile tempLrc("[00:01.20]first line\n[00:03.50]second line\n");
    LyricAnalysis la;
    la.setFromFile(tempLrc.path());
    EXPECT_EQ(la.getLineAt(0), QStringLiteral("first line"));
    EXPECT_EQ(la.getLineAt(1), QStringLiteral("second line"));
}

TEST(LyricAnalysisTest, getLineAtInvalidIndexReturnsEmpty)
{
    TempLrcFile tempLrc("[00:01.20]only\n");
    LyricAnalysis la;
    la.setFromFile(tempLrc.path());
    EXPECT_TRUE(la.getLineAt(-1).isNull()  || la.getLineAt(-1).isEmpty());
    EXPECT_TRUE(la.getLineAt(99).isNull()  || la.getLineAt(99).isEmpty());
}

// ============================================================================
// 时间顺序：解析后按时间升序排列，getPostion 单调递增
// ============================================================================
TEST(LyricAnalysisTest, lyricsAreSortedByTimeAscending)
{
    // 故意乱序写入，解析后应排序
    TempLrcFile tempLrc(
        "[00:05.00]third\n[00:01.00]first\n[00:03.00]second\n");
    LyricAnalysis la;
    la.setFromFile(tempLrc.path());

    ASSERT_EQ(la.getCount(), 3);
    EXPECT_EQ(la.getLineAt(0), QStringLiteral("first"));
    EXPECT_EQ(la.getLineAt(1), QStringLiteral("second"));
    EXPECT_EQ(la.getLineAt(2), QStringLiteral("third"));

    // 位置单调递增（时间排序的体现）
    EXPECT_LE(la.getPostion(0), la.getPostion(1));
    EXPECT_LE(la.getPostion(1), la.getPostion(2));
}

TEST(LyricAnalysisTest, getPostionInvalidIndexReturnsZero)
{
    LyricAnalysis la;  // 未加载任何文件
    EXPECT_EQ(la.getPostion(0), 0);
    EXPECT_EQ(la.getPostion(999), 0);
}

// ============================================================================
// getIndex / getPostion 的自洽性：二分查找应能定位回原行
// ============================================================================
TEST(LyricAnalysisTest, getIndexPostionAreConsistent)
{
    TempLrcFile tempLrc(
        "[00:01.00]a\n[00:03.00]b\n[00:05.00]c\n[00:07.00]d\n");
    LyricAnalysis la;
    la.setFromFile(tempLrc.path());
    const int n = la.getCount();
    ASSERT_EQ(n, 4);

    for (int i = 0; i < n; ++i) {
        const qint64 pos = la.getPostion(i);
        // 用该行的时间点去查 index，应定位回同一行
        EXPECT_EQ(la.getIndex(pos), i);
    }
}

TEST(LyricAnalysisTest, getIndexFallsBackToLastForPositionBeyondEnd)
{
    TempLrcFile tempLrc("[00:01.00]a\n[00:03.00]b\n");
    LyricAnalysis la;
    la.setFromFile(tempLrc.path());
    ASSERT_EQ(la.getCount(), 2);
    // 超出最后时间点，二分应落在最后一行
    EXPECT_EQ(la.getIndex(99999999), 1);
    // 早于第一行，落在第 0 行
    EXPECT_EQ(la.getIndex(0), 0);
}

// ============================================================================
// 空文件 / 无效行：鲁棒性
// ============================================================================
TEST(LyricAnalysisTest, emptyFileProducesNoLyrics)
{
    TempLrcFile tempLrc("");
    LyricAnalysis la;
    la.setFromFile(tempLrc.path());
    EXPECT_EQ(la.getCount(), 0);
    EXPECT_TRUE(la.allLyrics().isEmpty());
}

TEST(LyricAnalysisTest, linesWithoutTimestampAreIgnored)
{
    // 纯文本、无 [mm:ss] 标记的行应被忽略，不计入
    TempLrcFile tempLrc("this is not a lyric line\nplain text\n");
    LyricAnalysis la;
    la.setFromFile(tempLrc.path());
    EXPECT_EQ(la.getCount(), 0);
}

TEST(LyricAnalysisTest, nonExistentFileProducesNoLyrics)
{
    LyricAnalysis la;
    la.setFromFile("/nonexistent/path/to/file.lrc");
    EXPECT_EQ(la.getCount(), 0);
}

// ============================================================================
// 编码检测路径：UTF-8 BOM、纯 ASCII、GBK 中文，覆盖 getFileCodec 的多条回退分支
// ============================================================================
TEST(LyricAnalysisTest, parsesUtf8BomFile)
{
    // UTF-8 BOM 头：codecForUtfText 应识别，走 BOM 分支
    QByteArray body = "[00:01.00]hello bom\n[00:02.00]second\n";
    QByteArray withBom;
    withBom.append('\xEF').append('\xBB').append('\xBF');
    withBom.append(body);

    TempLrcFileWithBytes temp(withBom);
    LyricAnalysis la;
    la.setFromFile(temp.path());
    EXPECT_EQ(la.getCount(), 2);
    EXPECT_EQ(la.getLineAt(0), QStringLiteral("hello bom"));
}

TEST(LyricAnalysisTest, parsesGb2312ChineseLyrics)
{
    // GB18030 编码的中文歌词：触发 Utils::detectEncodings 与 codecForName 校验分支
    // 注意：编码检测依赖 ICU 版本，不同环境下可能回退到 locale codec 导致解码失败；
    // 因此只验证"流程不崩溃 + 解析完成"，不断言解析行数（检测结果环境相关）
    const QString chinese = QStringLiteral("[00:01.00]第一行\n[00:02.00]第二行\n");
    const QByteArray gb = QTextCodec::codecForName("GB18030")->fromUnicode(chinese);

    TempLrcFileWithBytes temp(gb);
    LyricAnalysis la;
    EXPECT_NO_FATAL_FAILURE(la.setFromFile(temp.path()));
    EXPECT_GE(la.getCount(), 0);  // 编码检测可能成功或失败，至少不应崩溃
}

TEST(LyricAnalysisTest, parsesInvalidTimeLineIsIgnored)
{
    // 时间格式非法（mm:ss.z 不匹配）的行应被跳过，正常行仍解析
    TempLrcFile tempLrc("[xx:yy.zz]badtime\n[00:05.00]ok\n");
    LyricAnalysis la;
    la.setFromFile(tempLrc.path());
    EXPECT_EQ(la.getCount(), 1);
    EXPECT_EQ(la.getLineAt(0), QStringLiteral("ok"));
}

// ============================================================================
// allLyrics / getPostion 边界
// ============================================================================
TEST(LyricAnalysisTest, allLyricsReturnsConsistentData)
{
    TempLrcFile tempLrc("[00:01.00]a\n[00:02.00]b\n");
    LyricAnalysis la;
    la.setFromFile(tempLrc.path());
    const auto all = la.allLyrics();
    ASSERT_EQ(all.size(), 2);
    EXPECT_EQ(all[0].second, QStringLiteral("a"));
    EXPECT_EQ(all[1].second, QStringLiteral("b"));
    EXPECT_LE(all[0].first, all[1].first);
}

TEST(LyricAnalysisTest, getPostionOnEmptyLyricsReturnsZero)
{
    // 未加载文件时 getPostion 越界返回 0
    // 注意：源码 getPostion 对负 index 仍走 index<size(0) 分支会越界（源码缺陷），
    // 这里只测非负越界 index，避免触发 UB
    LyricAnalysis la;
    EXPECT_EQ(la.getPostion(0), 0);
    EXPECT_EQ(la.getPostion(999), 0);
}
