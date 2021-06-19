/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     ZouYa <zouya@uniontech.com>
 *
 * Maintainer: WangYu <wangyu@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MusicLyric_H
#define MusicLyric_H

#include <QVector>

class MusicLyric
{
public:
    MusicLyric();
    void parseLyric(const QString& str);
    void getFromFile(QString dir);
    //void getFromFileOld(QString dir);
    QString getLineAt(int index);
    int getCount() const;
    int getIndex(qint64 pos);
    qint64 getPosition(int index);
    //bool getHeadFromFile(QString dir);

private:
    QString filedir;
    double offset;
    QVector<qint64> position;
    QVector<QString> line;
};

#endif // LYRIC_H
