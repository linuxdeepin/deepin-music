// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MusicLyric_H
#define MusicLyric_H

#include <QVector>

class MusicLyric
{
public:
    MusicLyric();
    void getFromFile(QString dir);
    //void getFromFileOld(QString dir);
    QString getLineAt(int index);
    int getCount() const;
    int getIndex(qint64 pos);
    qint64 getPostion(int index);
    //bool getHeadFromFile(QString dir);

private:
    QString filedir;
    double offset;
    QVector<qint64> postion;
    QVector<QString> line;

    void parseLyric(const QString& str);
};

#endif // LYRIC_H
