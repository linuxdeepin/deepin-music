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

#include "musiclyric.h"
#include <QFile>
#include <QTextStream>
#include <QFileDialog>
#include <QTextCodec>
#include <QTime>
#include <QDebug>

#include "util/basetool.h"

MusicLyric::MusicLyric(): offset(0.00)
{

}

int MusicLyric::getIndex(qint64 pos)
{
    //采用二分查找
    //时间复杂度O(logn)
    int lt, rt;
    lt = 0;
    rt = position.count();
    while (lt < rt - 1) {
        int mid = (lt + rt) >> 1;
        if (position[mid] > pos) rt = mid;
        else lt = mid;
    }
    return lt;
}

qint64 MusicLyric::getPosition(int index)
{
    if (index < position.size())
        return position[index];
    else
        return 0;
}

static QString getFileCodex(QString dir)
{
    QFile fin(dir);
    QString code;
    if (!fin.open(QIODevice::ReadOnly))
        return code;

//    unsigned char  s2;
//    fin.read((char *)&s2, sizeof(s2));
//    int p = s2 << 8;
//    fin.read((char *)&s2, sizeof(s2));
//    p += s2;

//    switch (p) {
//    case 0xfffe:  //65534
//        code = "Unicode";
//        break;
//    case 0xfeff://65279
//        code = "Unicode big endian";
//        break;
//    case 0xefbb://61371
//        code = "UTF-8";
//        break;
//    default:
//        code = "GB18030";
//    }

    QByteArray data = fin.readAll();
    fin.close();
    code = BaseTool::detectEncode(data, dir);

    return code;
}

void MusicLyric::parseLyric(const QString &str)
{
    auto lines = str.split("\n");
    QRegExp rx("\\[([^\\]]*)\\]\\s*(\\S.*)");
    QVector<QPair<qint64,QString>> tmp;
    for(auto line : lines) {
        rx.indexIn(line);
        auto timeStr = rx.capturedTexts()[1];
        auto lyricStr = rx.capturedTexts()[2];
        QTime t = QTime::fromString(timeStr,"mm:ss.z");
        qint64 time = t.msecsSinceStartOfDay();
        if(t.isValid())
            tmp.push_back({time,lyricStr});
    }
    std::sort(tmp.begin(),tmp.end());
    this->position.clear();
    this->line.clear();
    for(auto item:tmp) {
        this->position.push_back(item.first);
        this->line.push_back(item.second);
    }
}

void MusicLyric::getFromFile(QString dir)
{
    qDebug() << "Lyric dir:" << dir << endl;
    this->filedir = dir;
    //this->offset
    this->line.clear();
    this->position.clear();
    //先使用暴力的字符串匹配，还不会正则表达式
    //时间复杂度O(n)
    QString codeStr = getFileCodex(dir);
    QFile file(dir);
    if (!file.exists()) return;
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;
    QTextStream read(&file);
    if (!codeStr.isEmpty()) {
        read.setCodec(QTextCodec::codecForName(codeStr.toStdString().c_str()));
    }
    parseLyric(read.readAll());
}

/*
#标识标签(ID-tags)
格式："[标识名:值]"。大小写等价。
[ar:艺人名]
[ti:曲名]
[al:专辑名]
[by:编者（指编辑LRC歌词的人）]
[offset:时间补偿值]其单位是秒，正值表示整体提前，负值相反。这是用于总体调整显示快慢的。

#时间标签(Time-tag)
格式："[mm:ss]"或"[mm:ss.fff]"（分钟：秒）。数字必须为非负整数。
时间标签需位于某行歌词中的句首部分，一行歌词可以包含多个时间标签。
*/
//void MusicLyric::getFromFileOld(QString dir)
//{
//    qDebug() << "Lyric dir:" << dir << endl;
//    this->filedir = dir;
//    //this->offset
//    this->line.clear();
//    this->postion.clear();
//    //先使用暴力的字符串匹配，还不会正则表达式
//    //时间复杂度O(n)
//    QFile file(dir);
//    if (!file.exists()) return;
//    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
//        return;
//    QTextStream read(&file);
//    QChar ch;
//    bool flag;
//    qint64 mm;
//    double ss;
//    QVector<qint64> muli;
//    QMap<qint64, QString> ans;
//    QMap<qint64, QString>::iterator it;
//    QString sign;
//    QString val;
//    QString str;
//    while (!read.atEnd()) {
//        for (;;) {
//            read >> ch;
//            if (ch == '[' || read.atEnd()) {
//                if (muli.count()) {
//                    for (int i = 0; i < muli.count(); ++i)
//                        ans.insert(muli[i], str);
//                }
//                str = "";
//                muli.clear();
//                break;
//            }
//            str = str + ch;
//        }
//        if (read.atEnd()) break;
//        for (;;) {
//            read >> ch;
//            if (ch == ':' || read.atEnd()) break;
//            sign = sign + ch;
//        }
//        if (read.atEnd()) break;
//        for (;;) {
//            read >> ch;
//            if (ch == ']' || read.atEnd()) {
//                mm = sign.toLongLong(&flag, 10);
//                //判断sign是否是整数
//                if (flag) {
//                    ss = val.toDouble(&flag);
//                    if (flag) {
//                        qint64  curtime = (qint64)(ss * 1000) + mm * 60 * 1000;
//                        muli.push_back(curtime);
//                    }
//                }
//                break;
//            }
//            val = val + ch;
//        }
//        sign = "";
//        val = "";
//    }
//    for (it = ans.begin(); it != ans.end(); ++it) {
//        this->postion.push_back(it.key());
//        this->line.push_back(it.value());
//    }
//}

//bool MusicLyric::getHeadFromFile(QString dir)
//{
//    QString codeStr = getFileCodex(dir);
//    QFile file(dir);
//    if (!file.open(QIODevice::ReadOnly))
//        return false;
//    QTextCodec *codec = QTextCodec::codecForName(codeStr.toStdString().c_str());
//    while (!file.atEnd()) {
//        QByteArray arr = file.readLine();
//        QString str = codec->toUnicode(arr);
//        this->line.push_back(str);
//    }
//    file.close();
//    return true;
//}

QString MusicLyric::getLineAt(int index)
{
    return line[index];
}

int MusicLyric::getCount() const
{
    return line.count();
}
