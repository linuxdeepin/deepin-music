/*
 * Copyright (C) 2016 ~ 2018 Wuhan Deepin Technology Co., Ltd.
 *
 * Author:     Iceyer <me@iceyer.net>
 *
 * Maintainer: Iceyer <me@iceyer.net>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <QObject>

#include <mediameta.h>
#include <playlistmeta.h>

class MediaDatabaseWriter : public QObject
{
    Q_OBJECT
public:
    explicit MediaDatabaseWriter(QObject *parent = 0);

public slots:
    void addMediaMeta(const MetaPtr meta);
    void addMediaMetaList(const MetaPtrList metalist);
    void updateMediaMeta(const MetaPtr meta);
    void updateMediaMetaList(const MetaPtrList metalist);
    void removeMediaMeta(const MetaPtr meta);
    void removeMediaMetaList(const MetaPtrList metalist);

    void insertMusic(const MetaPtr meta, const PlaylistMeta &playlistMeta);
    void insertMusicList(const MetaPtrList metalist, const PlaylistMeta &playlistMeta);
};

