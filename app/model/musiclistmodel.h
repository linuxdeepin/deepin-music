/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef MUSICLISTMODEL_H
#define MUSICLISTMODEL_H

#include <QObject>
#include <QStringList>
#include <QMap>

class MusicInfo
{
public:
    QString id;
    QString url;
    QString title;
    QString artist;
    QString album;
    qint64  length;
    qint64  track;
};

typedef QList<MusicInfo>    MusicList;

class MusicListInfo
{
public:
    QString id;
    QString displayName;
    QString url;
    QString icon;

    bool    editmode;
    bool    readonly;

    QStringList                 musicIds;
    QMap<QString, MusicInfo>    musicMap;
};

Q_DECLARE_METATYPE(MusicInfo);
Q_DECLARE_METATYPE(MusicListInfo);

#endif // MUSICLISTMODEL_H
