// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LYRICANALYSIS_H
#define LYRICANALYSIS_H

#include <QVector>
#include <QByteArray>
#include <QPair>

struct LyricWord {
    qint64 time;    // 时间戳（毫秒）
    QString text;   // 该时间点后的文本片段
};

class LyricAnalysis
{
public:
    LyricAnalysis();

    void setFromFile(const QString &filePath);
    QVector<QPair<qint64, QString> > allLyrics();
    QString getLineAt(int index);
    int getCount() const;
    int getIndex(qint64 pos);
    qint64 getPostion(int index);

    // 逐字歌词支持
    bool hasWordTiming(int index) const;
    QVector<LyricWord> getWordTiming(int index) const;

private:
    void parseLyric(const QString &str);
    QString getFileCodec();

private:
    QString                           m_filePath;
    QVector<QPair<qint64, QString> >   m_allLyrics;
    QVector<QVector<LyricWord>>        m_wordLyrics;   // 逐字歌词时间轴
};

#endif
