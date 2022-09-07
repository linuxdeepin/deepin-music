// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
