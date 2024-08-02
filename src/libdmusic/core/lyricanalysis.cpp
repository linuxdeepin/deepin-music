// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "lyricanalysis.h"

#include <QFile>
#include <QTextStream>
#include <QTextCodec>
#include <QTime>
#include <QMimeDatabase>
#include <QDebug>
#include <QRegExp>

#include <DTextEncoding>

DCORE_USE_NAMESPACE

static float codecConfidenceForData(const QTextCodec *codec, const QByteArray &data, const QLocale::Country &country)
{
    qreal hep_count = 0;
    int non_base_latin_count = 0;
    qreal unidentification_count = 0;
    int replacement_count = 0;

    QTextDecoder decoder(codec);
    const QString &unicode_data = decoder.toUnicode(data);

    for (int i = 0; i < unicode_data.size(); ++i) {
        const QChar &ch = unicode_data.at(i);

        if (ch.unicode() > 0x7f)
            ++non_base_latin_count;

        switch (ch.script()) {
        case QChar::Script_Hiragana:
        case QChar::Script_Katakana:
            hep_count += (country == QLocale::Japan) ? 1.2 : 0.5;
            unidentification_count += (country == QLocale::Japan) ? 0 : 0.3;
            break;
        case QChar::Script_Han:
            hep_count += (country == QLocale::China || country == QLocale::Japan) ? 1.2 : 0.5;
            unidentification_count += (country == QLocale::China || country == QLocale::Japan) ? 0 : 0.3;
            break;
        case QChar::Script_Hangul:
            hep_count += (country == QLocale::NorthKorea || country == QLocale::SouthKorea) ? 1.2 : 0.5;
            unidentification_count += (country == QLocale::NorthKorea || country == QLocale::SouthKorea) ? 0 : 0.3;
            break;
        case QChar::Script_Cyrillic:
            hep_count += (country == QLocale::Russia) ? 1.2 : 0.5;
            unidentification_count += (country == QLocale::Russia) ? 0 : 0.3;
            break;
        case QChar::Script_Devanagari:
            hep_count += (country == QLocale::Nepal || country == QLocale::India) ? 1.2 : 0.5;
            unidentification_count += (country == QLocale::Nepal || country == QLocale::India) ? 0 : 0.3;
            break;
        default:
            // full-width character, emoji, 常用标点, 拉丁文补充1，天城文补充，CJK符号和标点符号（如：【】）
            if ((ch.unicode() >= 0xff00 && ch <= (QChar)0xffef)
                    || (ch.unicode() >= 0x2600 && ch.unicode() <= 0x27ff)
                    || (ch.unicode() >= 0x2000 && ch.unicode() <= 0x206f)
                    || (ch.unicode() >= 0x80 && ch.unicode() <= 0xff)
                    || (ch.unicode() >= 0xa8e0 && ch.unicode() <= 0xa8ff)
                    || (ch.unicode() >= 0x0900 && ch.unicode() <= 0x097f)
                    || (ch.unicode() >= 0x3000 && ch.unicode() <= 0x303f)) {
                ++hep_count;
            } else if (ch.isSurrogate() && ch.isHighSurrogate()) {
                ++i;

                if (i < unicode_data.size()) {
                    const QChar &next_ch = unicode_data.at(i);

                    if (!next_ch.isLowSurrogate()) {
                        --i;
                        break;
                    }

                    uint unicode = QChar::surrogateToUcs4(ch, next_ch);

                    // emoji
                    if (unicode >= 0x1f000 && unicode <= 0x1f6ff) {
                        hep_count += 2;
                    }
                }
            } else if (ch.unicode() == QChar::ReplacementCharacter) {
                ++replacement_count;
            } else if (ch.unicode() > 0x7f) {
                // 因为UTF-8编码的容错性很低，所以未识别的编码只需要判断是否为 QChar::ReplacementCharacter 就能排除
                if (codec->name() != "UTF-8")
                    ++unidentification_count;
            }
            break;
        }
    }

    float c = static_cast<float>(qreal(hep_count) / non_base_latin_count / 1.2);

    c -= static_cast<float>(qreal(replacement_count) / non_base_latin_count);
    c -= static_cast<float>(qreal(unidentification_count) / non_base_latin_count);

    return qMax(0.0f, c);
}

LyricAnalysis::LyricAnalysis(): m_offset(0.00)
{

}

int LyricAnalysis::getIndex(qint64 pos)
{
    //采用二分查找
    //时间复杂度O(logn)
    int lt, rt;
    lt = 0;
    rt = m_allLyrics.count();
    while (lt < rt - 1) {
        int mid = (lt + rt) >> 1;
        if (m_allLyrics[mid].first > pos) rt = mid;
        else lt = mid;
    }
    return lt;
}

qint64 LyricAnalysis::getPostion(int index)
{
    if (index < m_allLyrics.size())
        return m_allLyrics[index].first;
    else
        return 0;
}

void LyricAnalysis::parseLyric(const QString &str)
{
    auto lines = str.split("\n");
    QRegExp rx("\\[([^\\]]*)\\]\\s*(\\S.*\\S|\\S)\\s*$");
    QVector<QPair<qint64, QString>> tmp;
    for (auto line : lines) {
        if (rx.indexIn(line) != -1) {
            auto timeStr = rx.capturedTexts()[1];
            auto lyricStr = rx.capturedTexts()[2];
            QTime t = QTime::fromString(timeStr, "mm:ss.z");
            qint64 time = t.msecsSinceStartOfDay();
            if (t.isValid())
                tmp.push_back({time, lyricStr});
        }
    }
    std::sort(tmp.begin(), tmp.end());
    m_allLyrics.clear();
    for (auto item : tmp) {
        m_allLyrics.push_back(item);
    }
}

QString LyricAnalysis::getFileCodec()
{
    QFile fin(m_filePath);
    QString code;
    if (!fin.open(QIODevice::ReadOnly))
        return code;

    QByteArray data = fin.readAll();
    fin.close();

    if (data.isEmpty()) {
        return QTextCodec::codecForLocale()->name();
    }

    if (QTextCodec *c = QTextCodec::codecForUtfText(data, nullptr)) {
        return c->name();
    }

    bool isOK = false;
    QByteArray encode = DTextEncoding::detectFileEncoding(m_filePath, &isOK);
    return encode;
}

void LyricAnalysis::setFromFile(const QString &filePath)
{
    m_filePath = filePath;
    m_allLyrics.clear();
    QString codecStr = getFileCodec();
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;
    QByteArray array = file.readAll();
    QTextCodec *codec = QTextCodec::codecForName(codecStr.toLatin1());
    parseLyric(codec->toUnicode(array));
}

QVector<QPair<qint64, QString> > LyricAnalysis::allLyrics()
{
    return m_allLyrics;
}

QString LyricAnalysis::getLineAt(int index)
{
    return m_allLyrics[index].second;
}

int LyricAnalysis::getCount() const
{
    return m_allLyrics.count();
}
