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

