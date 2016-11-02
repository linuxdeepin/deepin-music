/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "cueparser.h"

extern "C" {
//#include "../../vendor/src/libcue/libcue.h"
//#include "../../vendor/src/libcue/cue_time.h"
#include <libcue/libcue.h>
}

#include <QFile>
#include <QFileInfo>
#include <QTextCodec>
#include <QCryptographicHash>

#include <QDebug>

#include "musicmeta.h"

void time_frame_to_msf(long frame, int *m, int *s, int *f)
{
    *f = frame % 75;           /* 0 <= frames <= 74 */
    frame /= 75;
    *s = frame % 60;          /* 0 <= seconds <= 59 */
    frame /= 60;
    *m = frame;               /* 0 <= minutes */
}

qint64 timeframe2mtime(long frame)
{
    int m, s, f;
    time_frame_to_msf(frame, &m, &s, &f);
    auto a =  m * 60 * 1000 + s * 1000 + f * 1000 / 75;

    qDebug() << frame << a << lengthString(a);
    return a;
}

#define mu_assert(msg, condition) \
    do{ \
        if (! (condition)) { \
            /*qDebug() << msg*/; \
            /*return*/; \
        } \
    }while(0)


CueParser::CueParser(const QString &filepath)
{
    QFile cueFile(filepath);
    if (!cueFile.open(QIODevice::ReadOnly)) { return; }

    QByteArray cueByte = cueFile.readAll();
    cueFile.close();

    QTextCodec *codec = QTextCodec::codecForName("GB18030");
    QString cue = codec->toUnicode(cueByte.toStdString().c_str());

    QString album;
    Cd *cd = cue_parse_string(cue.toStdString().c_str());
    if (nullptr == cd) {
        qCritical() << "parse cue failed" << cue;
        return;
    }

    Cdtext *cdtext = cd_get_cdtext(cd);
    mu_assert("error getting CDTEXT", cdtext != NULL);

    const char *val;
    val = cdtext_get(PTI_TITLE, cdtext);
    mu_assert("error getting CD title", val != NULL);
    album = val;

    int ival = cd_get_ntrack(cd);

    QFileInfo cueFileInfo(filepath);
    QMap<QString, MusicMeta> fileMetaCache;
    QMap<QString, bool> fileExist;

    for (int i = 0; i < ival; ++i) {
        MusicMeta meta;
        meta.album = album;
        meta.track = i + 1;
        Track *track;
        track = cd_get_track(cd, int(meta.track));
        mu_assert("error getting track", track != NULL);

        val = track_get_filename(track);
        mu_assert("error getting track filename", val != NULL);
        meta.localpath = cueFileInfo.absolutePath() + "/" + val;

        if (!fileMetaCache.contains(meta.localpath)) {
            QFileInfo media(meta.localpath);
            auto mediaHash = MusicMetaName::hash(media.absoluteFilePath());
            auto mediaMeta = MusicMetaName::fromLocalFile(media, mediaHash);
            fileMetaCache.insert(meta.localpath, mediaMeta);
            fileExist.insert(meta.localpath, media.exists());
        }

        if (!fileExist.value(meta.localpath)) {
            continue;
        }

        auto trackPath = QString("%1.%2").arg(cueFileInfo.absoluteFilePath()).arg(i + 1);
        auto hash = MusicMetaName::hash(trackPath);
        meta.hash = hash;

        // TODO: maybe multi
        musicFilePath = meta.localpath;

        cdtext = track_get_cdtext(track);
        mu_assert("error getting track CDTEXT", cdtext != NULL);

        val = cdtext_get(PTI_PERFORMER, cdtext);
        mu_assert("error getting track performer", val != NULL);
        meta.artist = val;

        val = cdtext_get(PTI_TITLE, cdtext);
        mu_assert("error getting track title", val != NULL);
        meta.title = val;

        meta.offset = timeframe2mtime(track_get_start(track));
        meta.length = timeframe2mtime(track_get_length(track));

        // TODO: track must < 1000
        meta.timestamp = fileMetaCache.value(meta.localpath).timestamp + meta.track;
        meta.filetype = fileMetaCache.value(meta.localpath).filetype;

        MusicMetaName::pinyinIndex(meta);

        // TODO: fix last len
        if (meta.track == ival && meta.length <= 0) {
            auto total = fileMetaCache.value(meta.localpath).length;
            if (total > meta.offset) {
                meta.length = total - meta.offset;
            }
        }

        metalist << meta;
    }

    cd_delete(cd);
}
