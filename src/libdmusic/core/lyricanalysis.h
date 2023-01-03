/*
 * Copyright (C) 2020 ~ 2020 Deepin Technology Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
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
