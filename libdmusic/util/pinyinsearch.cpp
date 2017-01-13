/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "pinyinsearch.h"

#include "../../../vendor/src/chinese2pinyin/chinese2pinyin.h"

namespace DMusic
{
namespace PinyinSearch
{

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

inline bool isChinese(const QChar &c)
{
    return c.unicode() < 0x9FBF && c.unicode() > 0x4E00;
}

inline QString toChinese(const QString &c)
{
    QString pinyin = Pinyin::Chinese2Pinyin(c);
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
        if (isNumber(c)) {
            wordList << c;
            continue;
        }
        if (isChinese(c)) {
            wordList << toChinese(c);
            continue;
        }
    }
    return wordList;
}

}
}
