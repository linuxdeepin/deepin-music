/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#pragma once

#include <QObject>

#include <mediameta.h>

struct WordElement {
    // Sustain time.
    qint64 time;

    QString word;

public:
    bool operator<(const WordElement &other)
    {
        return this->time < other.time;
    }
};

struct LrcElement {
    // Start time of current sentence, in milliseconds.
    qint64 start = 0;

    // Set current time. Used in translations.
    // Used in krc.
    int line_number = 0;

    // Duration of current sentence, in milliseconds.
    // This value is 0 if current element is the last one.
    // Used in lrc.
    qint64 duration = 0;

    // current lrc sentence, without any time tags.
    QString content;

    // Time to keep displaying current line before switch to next one, in
    // milliseconds.
    // Used in krc.
    qint64 sustain_time = 0;

    // Each word and its time delta.
    // Used in krc.
    QList<WordElement> words;

    // Translated to Simplified Chinese. Default is empty.
    // Used in krc.
    QString zh_cn;

public:
    bool operator<(const LrcElement &other) const
    {
        return this->start < other.start;
    }
};

class Lyric
{
public:
    explicit Lyric();

    bool        hasTime;
    MediaMeta   m_meta;
    QList<LrcElement> m_lyricElements;
};

Lyric parseLrc(QString &rawLyric);

