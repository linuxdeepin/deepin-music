/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef MEDIADATABASEWRITER_H
#define MEDIADATABASEWRITER_H

#include <QObject>

#include "music.h"
#include "playlist.h"

class MediaDatabaseWriter : public QObject
{
    Q_OBJECT
public:
    explicit MediaDatabaseWriter(QObject *parent = 0);

public slots:
    void addMusicMeta(const MusicMeta &meta);
    void addMusicMetaList(const MusicMetaList &metalist);
    void removeMusicMeta(const MusicMeta &meta);
    void removeMusicMetaList(const MusicMetaList &metalist);

    void insertMusic(const MusicMeta &meta, const PlaylistMeta &playlistMeta);
    void insertMusicList(const MusicMetaList &metalist, const PlaylistMeta &playlistMeta);
//    static void addPlaylist(const PlaylistMeta &playlistMeta);
//    static void updatePlaylist(const PlaylistMeta &playlistMeta);
//    static void removePlaylist(const PlaylistMeta &playlistMeta);
//    static void deleteMusic(const MusicMeta &meta, const PlaylistMeta &playlistMeta);
};


#endif // MEDIADATABASEWRITER_H
