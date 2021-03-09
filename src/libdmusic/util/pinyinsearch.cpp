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

#include "pinyinsearch.h"

#include <QFile>
#include <QTextStream>
#include <DPinyin>

namespace DMusic {
namespace PinyinSearch {

inline bool isAlphabeta(const QChar &c)
{
    QRegExp re("[A-Za-z]*");
    return re.exactMatch(c);
}

inline bool isNumber(const QChar &c)
{
    QRegExp re("[0-9]*");
    return re.exactMatch(c);
}

bool isChinese(const QChar &c)
{
    return c.unicode() <= 0x9FBF && c.unicode() >= 0x4E00;
}

inline QString toChinese(const QString &c)
{
    QString pinyin = Dtk::Core::Chinese2Pinyin(c);
    if (pinyin.length() >= 2
            && isNumber(pinyin.at(pinyin.length() - 1))) {
        return pinyin.left(pinyin.length() - 1);
    }
    return pinyin;
}

QStringList simpleChineseSplit(QString &pinyin)
{
    QStringList wordList;
    bool isLastAlphabeta = false;
    for (auto &c : pinyin) {
        bool isCurAlphabeta = isAlphabeta(c);
        if (isCurAlphabeta) {
            if (!isLastAlphabeta) {
                wordList << c;
            } else {
                wordList.last().append(c);
            }
            continue;
        }
        isLastAlphabeta = isCurAlphabeta;
//        if (isNumber(c)) {
//            wordList << c;
//            continue;
//        }

        //除了中文外，其它字符不作特殊处理
        if (isChinese(c)) {
            wordList << toChinese(c);
            continue;
        } else {
            wordList << c;
            continue;
        }
    }
    return wordList;
}

}
}
