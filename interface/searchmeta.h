/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#pragma once

#include <QString>
#include <QList>
#include <QObject>

namespace DMusic
{

struct SearchAlbum {
    int     id = 0;
    QString name;
    QString coverUrl;
};

struct SearchArtist {
    int     id = 0;
    QString name;
    QString avatarUrl;
};

struct SearchMeta {
    QString                 id;
    QString                 name;
    SearchAlbum             album;
    QList<SearchArtist>     artists;
    int                     length = 0;

    SearchMeta(){}
    SearchMeta(const QString &id): id(id) {}
};

}

Q_DECLARE_METATYPE(DMusic::SearchMeta)
Q_DECLARE_METATYPE(QList<DMusic::SearchMeta>)
