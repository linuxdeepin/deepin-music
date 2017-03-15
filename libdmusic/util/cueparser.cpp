/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "cueparser.h"

#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QTextCodec>

extern "C" {
#include <libcue/libcue.h>
}

#include "encodingdetector.h"

namespace Libcue {

void time_frame_to_msf(long frame, int *m, int *s, int *f)
{
    *f = frame % 75;           /* 0 <= frames <= 74 */
    frame /= 75;
    *s = frame % 60;          /* 0 <= seconds <= 59 */
    frame /= 60;
    *m = frame;               /* 0 <= minutes */
}

}

qint64 timeframe2mtime(long frame)
{
    int m, s, f;
    Libcue::time_frame_to_msf(frame, &m, &s, &f);
    auto a =  m * 60 * 1000 + s * 1000 + f * 1000 / 75;
    return a;
}

#define CHECK_RETURN(msg, condition) do{ \
        if (! (condition)) { \
            qCritical() << (msg); \
            return; \
        } \
    }while(0)


namespace DMusic
{

class CueParserPrivate
{
public:
    CueParserPrivate(CueParser *parent) : q_ptr(parent) {}

    void parseCue(const QByteArray &data, QByteArray codeName);

    MetaPtrList    metalist;
    QString        mediaFilepath;
    QString        cueFilepath;

    CueParser *q_ptr;
    Q_DECLARE_PUBLIC(CueParser)
};

void CueParserPrivate::parseCue(const QByteArray &data,  QByteArray codeName)
{
    if (codeName.isEmpty()) {
        codeName = EncodingDetector::detectEncodings(data).value(0);
    }

    auto codec = QTextCodec::codecForName(codeName);
    if (!codec) {
        codec = QTextCodec::codecForLocale();
    }
    Q_ASSERT(codec != nullptr);

    QString cue = codec->toUnicode(data.toStdString().c_str());

    // WARNING: libcue need empty line at last
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

    QMap<QString, MediaMeta> fileMetaCache;
    QMap<QString, bool> fileExist;

    QFileInfo cueFi(cueFilepath);
    for (int i = 0; i < ival; ++i) {
        auto meta = MetaPtr(new MediaMeta);
        meta->track = i + 1;
        Track *track;
        track = cd_get_track(cd, int(meta->track));
        if (nullptr == track) {
            continue;
        }

        val = track_get_filename(track);
        if (nullptr == val) {
            continue;
        }
        meta->localPath = cueFi.path() + "/" + val;
        meta->cuePath = cueFilepath;

        if (!fileMetaCache.contains(meta->localPath)) {
            QFileInfo media(meta->localPath);
//            if (media.exists()) {
                auto mediaMeta = MediaMeta::fromLocalFile(media);
                fileMetaCache.insert(meta->localPath, mediaMeta);
                fileExist.insert(meta->localPath, media.exists());
//            }
        }

        if (!fileExist.value(meta->localPath)) {
            continue;
        }

        auto trackPath = QString("%1.%2").arg(cueFilepath).arg(i + 1);
        auto hash = DMusic::filepathHash(trackPath);
        meta->hash = hash;

        // TODO: maybe multi
        mediaFilepath = meta->localPath;

        cdtext = track_get_cdtext(track);
        if (nullptr == cdtext) {
            continue;
        }

        val = cdtext_get(PTI_TITLE, cdtext);
        meta->title = val;

        val = cdtext_get(PTI_PERFORMER, cdtext);
        meta->artist = val;
        meta->album = album;

        meta->offset = timeframe2mtime(track_get_start(track));
        meta->length = timeframe2mtime(track_get_length(track));

        // TODO: hack track must < 1000
        auto cueMediaMeta = fileMetaCache.value(meta->localPath);
        meta->timestamp = cueMediaMeta.timestamp + meta->track;
        meta->filetype = cueMediaMeta.filetype;
        meta->size = cueMediaMeta.size;

        meta->updateSearchIndex();

        // TODO: fix last len
        if (meta->track == ival && meta->length <= 0) {
            auto total = cueMediaMeta.length;
            if (total > meta->offset) {
                meta->length = total - meta->offset;
            }
        }

        meta->filetype = cueMediaMeta.filetype;
        metalist << meta;
    }

    cd_delete(cd);
}

CueParser::CueParser(const QString &filepath) : CueParser(filepath, "") {}

CueParser::CueParser(const QString &filepath, QByteArray codeName) : d_ptr(new CueParserPrivate(this))
{
    // TODO: check cue invaild
    Q_D(CueParser);

    QFile cueFile(filepath);
    if (!cueFile.open(QIODevice::ReadOnly)) { return; }

    QByteArray cueByte = cueFile.readAll();
    cueFile.close();

    d->cueFilepath = filepath;
    d->parseCue(cueByte, codeName);
}

CueParser::~CueParser()
{

}

MetaPtrList CueParser::metalist() const
{
    Q_D(const CueParser);
    return d->metalist;
}

QString CueParser::mediaFilepath() const
{
    Q_D(const CueParser);
    return d->mediaFilepath;
}

QString CueParser::cueFilepath() const
{
    Q_D(const CueParser);
    return d->cueFilepath;
}

}
