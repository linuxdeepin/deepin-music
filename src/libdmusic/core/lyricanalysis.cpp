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
#include "util/log.h"
#include "util/utils.h"

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
    qCDebug(dmMusic) << "LyricAnalysis constructor called";
}

int LyricAnalysis::getIndex(qint64 pos)
{
    qCDebug(dmMusic) << "Getting index for position:" << pos;
    int lt = 0;
    int rt = m_allLyrics.count();
    while (lt < rt - 1) {
        int mid = (lt + rt) >> 1;
        if (m_allLyrics[mid].first > pos) rt = mid;
        else lt = mid;
    }
    qCDebug(dmMusic) << "Found index:" << lt << "for position:" << pos;
    return lt;
}

qint64 LyricAnalysis::getPostion(int index)
{
    qCDebug(dmMusic) << "Getting position for index:" << index;
    if (index < m_allLyrics.size()) {
        qCDebug(dmMusic) << "Valid index, returning position:" << m_allLyrics[index].first;
        return m_allLyrics[index].first;
    } else {
        qCDebug(dmMusic) << "Invalid index, returning 0";
        return 0;
    }
}

void LyricAnalysis::parseLyric(const QString &str)
{
    qCDebug(dmMusic) << "Parsing lyrics with length:" << str.length();
    auto lines = str.split("\n");
    QRegExp rx("\\[([^\\]]*)\\]\\s*(\\S.*\\S|\\S)\\s*$");
    QVector<QPair<qint64, QString>> tmp;

    for (auto line : lines) {
        if (rx.indexIn(line) != -1) {
            auto timeStr = rx.capturedTexts()[1];
            auto lyricStr = rx.capturedTexts()[2];
            QTime t = QTime::fromString(timeStr, "mm:ss.z");
            qint64 time = t.msecsSinceStartOfDay();
            if (t.isValid()) {
                tmp.push_back({time, lyricStr});
                qCDebug(dmMusic) << "Parsed lyric - Time:" << timeStr << "Text:" << lyricStr;
            } else {
                qCWarning(dmMusic) << "Invalid time format in lyric line:" << line;
            }
        } else {
            qCDebug(dmMusic) << "Skipping non-matching line:" << line;
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
    qCDebug(dmMusic) << "Detecting file codec for:" << m_filePath;
    QFile fin(m_filePath);
    QString code;
    if (!fin.open(QIODevice::ReadOnly)) {
        qCWarning(dmMusic) << "Failed to open file for codec detection:" << m_filePath;
        return QTextCodec::codecForLocale()->name();
    }

    QByteArray data = fin.readAll();
    fin.close();

    if (data.isEmpty()) {
        qCDebug(dmMusic) << "File is empty, using locale codec";
        return QTextCodec::codecForLocale()->name();
    }

    // 1. 首先检测UTF BOM标记
    if (QTextCodec *c = QTextCodec::codecForUtfText(data, nullptr)) {
        qCDebug(dmMusic) << "Detected UTF codec with BOM:" << c->name();
        return c->name();
    }

    bool isOK = false;
    QByteArray encode = DTextEncoding::detectFileEncoding(m_filePath, &isOK);
    qCDebug(dmMusic) << "DTK detected file encoding:" << encode << "success:" << isOK;
    
    // 使用更准确的Utils检测作为主要方法
    QStringList detectedEncodings = Utils::detectEncodings(data);
    if (!detectedEncodings.isEmpty()) {
        // 验证检测到的编码是否被QTextCodec支持
        for (const QString &encoding : detectedEncodings) {
            QTextCodec *codec = QTextCodec::codecForName(encoding.toLatin1());
            if (codec != nullptr) {
                qCDebug(dmMusic) << "Utils detected encodings:" << detectedEncodings << "using validated:" << encoding;
                return encoding;
            } else {
                qCWarning(dmMusic) << "Detected encoding not supported by QTextCodec:" << encoding;
            }
        }
        qCWarning(dmMusic) << "No supported encoding found in Utils detection results:" << detectedEncodings;
    }
    
    // 如果Utils检测失败或编码不支持，回退到DTK结果
    if (isOK && !encode.isEmpty()) {
        return encode;
    }
    
    // 最后回退到本地编码
    return QTextCodec::codecForLocale()->name();
}

void LyricAnalysis::setFromFile(const QString &filePath)
{
    qCDebug(dmMusic) << "Setting lyrics from file:" << filePath;
    m_filePath = filePath;
    m_allLyrics.clear();

    QString codecStr = getFileCodec();
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qCWarning(dmMusic) << "Failed to open lyric file:" << filePath;
        return;
    }

    QByteArray array = file.readAll();
    QTextCodec *codec = QTextCodec::codecForName(codecStr.toLatin1());
    parseLyric(codec->toUnicode(array));
    qCDebug(dmMusic) << "Successfully loaded lyrics from file with codec:" << codecStr;
}

QVector<QPair<qint64, QString> > LyricAnalysis::allLyrics()
{
    qCDebug(dmMusic) << "Returning all lyrics, count:" << m_allLyrics.size();
    if (m_allLyrics.isEmpty()) {
        qCDebug(dmMusic) << "Lyrics list is empty";
    }
    return m_allLyrics;
}

QString LyricAnalysis::getLineAt(int index)
{
    qCDebug(dmMusic) << "Getting line at index:" << index;
    if (index < 0 || index >= m_allLyrics.size()) {
        qCDebug(dmMusic) << "Invalid index, returning empty string";
        return QString();
    }
    return m_allLyrics[index].second;
}

int LyricAnalysis::getCount() const
{
    qCDebug(dmMusic) << "Getting lyrics count:" << m_allLyrics.size();
    return m_allLyrics.count();
}
