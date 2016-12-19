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

struct NeteaseAlbum {
    int     id;
    QString name;
    QString coverUrl;
};

struct NeteaseArtist {
    int     id;
    QString name;
    QString avatarUrl;
};

struct NeteaseSong {
    int                     id;
    QString                 name;
    NeteaseAlbum            album;
    QList<NeteaseArtist>    artists;
};
