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

class MusicInfo
{
public:
    QString id;
    QString url;
    QString title;
    QString artist;
    QString album;
    qint64  lenght;
    qint64  track;
};

class MusicListInfo
{
public:
    QString id;
    QString displayName;
    QString url;
    QString icon;

    bool    editmode;
    bool    readonly;

    QList<MusicInfo>    list;
};

Q_DECLARE_METATYPE(MusicInfo);
Q_DECLARE_METATYPE(MusicListInfo);

#endif // MUSICLISTMODEL_H
