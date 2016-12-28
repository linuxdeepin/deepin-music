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
#include <QString>
#include <QStringList>
#include <QMap>
#include <QTime>

class MusicMeta
{
public:

    QString hash;
    QString localPath;
    QString cuePath;
    QString title;
    QString artist;
    QString album;

    QString pinyinTitle;
    QString pinyinTitleShort;
    QString pinyinArtist;
    QString pinyinArtistShort;
    QString pinyinAlbum;
    QString pinyinAlbumShort;

    QString filetype;

    qint64  timestamp;
    qint64  length      = 1;//msec
    qint64  track;
    qint64  filesize        = 1;

    QString editor;
    QString composer;
    QString creator;
    qint64  offset      = 0;//msec

    bool    favourite   = false;
    bool    invalid     = false;

public:
    MusicMeta()
    {
        timestamp = 0;
        length = 0;
        offset = 0;
        filesize = 0;
        track = 0;
        favourite = false;
    }
};

typedef QList<MusicMeta>  MusicMetaList;

inline QString lengthString(qint64 length)
{
    length = length / 1000;
    QTime t(static_cast<int>(length / 3600), length % 3600 / 60, length % 60);
    return t.toString("mm:ss");
}

inline QString sizeString(qint64 sizeByte)
{
    QString text;
    if (sizeByte < 1024) {
        text.sprintf("%.1fB", sizeByte / 1.0);
        return text;
    }
    if (sizeByte < 1024 * 1024) {
        text.sprintf("%.1fK", sizeByte / 1024.0);
        return text;
    }
    if (sizeByte < 1024 * 1024 * 1024) {
        text.sprintf("%.1fM", sizeByte / 1024.0 / 1024.0);
        return text;
    }
    text.sprintf("%.1fG", sizeByte / 1024.0 / 1024.0 / 1024.0);
    return text;
}

Q_DECLARE_METATYPE(MusicMeta);
Q_DECLARE_METATYPE(MusicMetaList);
