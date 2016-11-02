/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "lyric.h"

#include <QString>

// Check square element exists in |lrc| or not.
bool HasNoTimeTag(const QString &lrc)
{
    return lrc.indexOf("[00:") == -1;
}

Lyric parseLrc(QString &rawLyric)
{
    Lyric lyric;
    QString lrc(rawLyric.replace(QString("\\n"), QChar('\n')));

    if (HasNoTimeTag(lrc)) {
        lyric.hasTime = false;
//        lrc_type_ = LrcType::kLrcTypeNoTimeLrc;
        return lyric;
    }
    lyric.hasTime = true;

    // Parse each line of content.
    foreach(const QString & line, lrc.split(QChar('\n'))) {
        if (line.length() == 0) {
            continue;
        }

        // Match [*] tag.
        QRegExp square_reg("\\[([^\\]]*)\\]");

        // To store time tags of current line.
        QList<int> lrc_time_tags;

        bool ok = true;

        // Match and capture tags.
        int pos = 0;
        while (square_reg.indexIn(line, pos) > -1) {

            // Removes left square bracket.
            int tag_start = pos + 1;
            // Removes right square bracket.
            int tag_len = square_reg.matchedLength() - 2;

            QString tag(line.mid(tag_start, tag_len));

            if (tag.startsWith("al:", Qt::CaseInsensitive)) {
                lyric.m_meta.album = tag.mid(3);
            } else if (tag.startsWith("ar:", Qt::CaseInsensitive)) {
                lyric.m_meta.artist = tag.mid(3);
            } else if (tag.startsWith("by:", Qt::CaseInsensitive)) {
                lyric.m_meta.editor = tag.mid(3);
            } else if (tag.startsWith("co:", Qt::CaseInsensitive)) {
                lyric.m_meta.composer = tag.mid(3);
            } else if (tag.startsWith("cr:", Qt::CaseInsensitive)) {
                lyric.m_meta.creator = tag.mid(3);
            } else if (tag.startsWith("offset:", Qt::CaseInsensitive)) {
                lyric.m_meta.offset = tag.mid(7).toLongLong();
            } else if (tag.startsWith("ti:", Qt::CaseInsensitive)) {
                lyric.m_meta.title = tag.mid(3);
            } else {
                // Parse time tag. Like [03:21.428]
                int colon_index = tag.indexOf(QChar(':'));

                if (colon_index > 0) {
                    auto minutes = tag.left(colon_index).toLongLong(&ok);
                    if (ok) {
                        double seconds = tag.mid(colon_index + 1).toFloat(&ok);
                        if (ok) {
                            auto time_tag = static_cast<int>(minutes * 60 * 1000 + seconds * 1000);
                            lrc_time_tags.append(time_tag);
                        }
                    }
                }
            }

            // Go forward.
            pos += square_reg.matchedLength();
        }

        // If end-square-bracket is not the end of line, assumes that
        // remaining content is words of song.
        if (pos < line.length() && ok && lrc_time_tags.length() > 0) {
            // Add lrc_element.
            QString words(line.mid(pos));

            // Store all time tags.
            foreach(int time_tag, lrc_time_tags) {
                LrcElement lrc_element;
                lrc_element.start = time_tag;
                lrc_element.content = words;
                lyric.m_lyricElements.append(lrc_element);
            }
        }
    }

    // Sort lrc_elements.
    std::sort(lyric.m_lyricElements.begin(), lyric.m_lyricElements.end());

    // TODO(xushaohua):Add durations to lrc_element.
    for (auto iter = lyric.m_lyricElements.begin(); iter != lyric.m_lyricElements.end();
            ++iter) {
        if ((iter + 1) != lyric.m_lyricElements.end()) {
            (*iter).duration = (* (iter + 1)).start - (*iter).start;
        } else {
            (*iter).duration = 0;
        }
    }

    // Add offset to each lrc_element.
    if (lyric.m_meta.offset != 0) {
        for (auto iter = lyric.m_lyricElements.begin(); iter != lyric.m_lyricElements.end();
                ++iter) {
            (*iter).start += lyric.m_meta.offset;
        }
    }

    // FIXME(xushaohua): time tag is incorrect.
    LrcElement title_lrc_element;
    title_lrc_element.start = 0;

    QString title_and_artist(lyric.m_meta.title);
    if (lyric.m_meta.artist.length() > 0) {
        title_and_artist.append(" - ");
        title_and_artist.append(lyric.m_meta.artist);
    }
    title_lrc_element.content = title_and_artist;

    lyric.m_lyricElements.prepend(title_lrc_element);
    return lyric;
}

Lyric::Lyric()
{

}
