// Copyright (C) 2020 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LYRICANALYSIS_H
#define LYRICANALYSIS_H

#include <QVector>
#include <QByteArray>
#include <QPair>

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

private:
    void parseLyric(const QString &str);
    QString getFileCodec();

private:
    QString                           m_filePath;
    double                            m_offset;
    QVector<QPair<qint64, QString> >   m_allLyrics;
};

#endif
