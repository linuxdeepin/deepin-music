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
#include "../../../vendor/src/libcue/libcue.h"
#include "../../../vendor/src/libcue/time.h"
}

#include <QFile>
#include <QFileInfo>
#include <QTextCodec>

#include <QDebug>

#include "musicmeta.h"
#include "icu.h"

qint64 timeframe2mtime(long frame)
{
    int m, s, f;
    time_frame_to_msf(frame, &m, &s, &f);
    auto a =  m * 60 * 1000 + s * 1000 + f * 1000 / 75;

    return a;
}

#define CHECK_RETURN(msg, condition) do{ \
        if (! (condition)) { \
            qCritical() << (msg); \
            return; \
        } \
    }while(0)

CueParser::CueParser(const QString &filepath)
{
    QFile cueFile(filepath);
    if (!cueFile.open(QIODevice::ReadOnly)) { return; }

    QByteArray cueByte = cueFile.readAll();
    cueFile.close();

    QByteArray codeName = ICU::codeName(cueByte);
    QTextCodec *codec = QTextCodec::codecForName(codeName);
    if (!codec) {
        codec = QTextCodec::codecForLocale();
    }
    QString cue = codec->toUnicode(cueByte.toStdString().c_str());

    // TODO: libcue need empty line at last
    cue.append("\r\n");

    QString album;
    Cd *cd = cue_parse_string(cue.toStdString().c_str());
    CHECK_RETURN("parse cue failed", nullptr != cd);

    Cdtext *cdtext = cd_get_cdtext(cd);
    CHECK_RETURN("error getting CDTEXT", cdtext != NULL);

    const char *val;
    val = cdtext_get(PTI_TITLE, cdtext);
    CHECK_RETURN("error getting CD title", val != NULL);

    album = val;

    int ival = cd_get_ntrack(cd);

//    qDebug() << "cd_get_ntrack" << filepath << ival;

    QFileInfo cueFileInfo(filepath);
    QMap<QString, MusicMeta> fileMetaCache;
    QMap<QString, bool> fileExist;

    for (int i = 0; i < ival; ++i) {
        MusicMeta meta;
        meta.track = i + 1;
        Track *track;
        track = cd_get_track(cd, int(meta.track));
        if (nullptr == track) {
            continue;
        }

        val = track_get_filename(track);
        if (nullptr == val) {
            continue;
        }
        meta.localPath = cueFileInfo.absolutePath() + "/" + val;
        meta.cuePath = cueFileInfo.absoluteFilePath();

        if (!fileMetaCache.contains(meta.localPath)) {
            QFileInfo media(meta.localPath);
            auto mediaHash = MusicMetaName::hash(media.absoluteFilePath());
            auto mediaMeta = MusicMetaName::fromLocalFile(media, mediaHash);
            fileMetaCache.insert(meta.localPath, mediaMeta);
            fileExist.insert(meta.localPath, media.exists());
        }

        if (!fileExist.value(meta.localPath)) {
            continue;
        }

        auto trackPath = QString("%1.%2").arg(cueFileInfo.absoluteFilePath()).arg(i + 1);
        auto hash = MusicMetaName::hash(trackPath);
        meta.hash = hash;

        // TODO: maybe multi
        musicFilePath = meta.localPath;

        cdtext = track_get_cdtext(track);
        if (nullptr == cdtext) {
            continue;
        }

        val = cdtext_get(PTI_TITLE, cdtext);
        meta.title = val;

        val = cdtext_get(PTI_PERFORMER, cdtext);
        meta.artist = val;
        meta.album = album;

        meta.offset = timeframe2mtime(track_get_start(track));
        meta.length = timeframe2mtime(track_get_length(track));

        // TODO: hack track must < 1000
        meta.timestamp = fileMetaCache.value(meta.localPath).timestamp + meta.track;
        meta.filetype = fileMetaCache.value(meta.localPath).filetype;
        meta.size = fileMetaCache.value(meta.localPath).size;

        MusicMetaName::pinyinIndex(meta);

        // TODO: fix last len
        if (meta.track == ival && meta.length <= 0) {
            auto total = fileMetaCache.value(meta.localPath).length;
            if (total > meta.offset) {
                meta.length = total - meta.offset;
            }
        }

        meta.filetype = cueFileInfo.fileName();
        metalist << meta;
    }

    cd_delete(cd);
}
