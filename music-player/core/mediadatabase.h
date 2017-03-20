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

#include <util/singleton.h>
#include <playlistmeta.h>

#include "mediadatabasewriter.h"

class MediaDatabase : public QObject, public DMusic::DSingleton<MediaDatabase>
{
    Q_OBJECT
public:
    explicit MediaDatabase(QObject *parent = 0);
    void init();

    //! music meta query interface
    bool mediaMetaExist(const QString &hash);
    QList<MediaMeta> allmetas();
    static MetaPtrList searchMediaTitle(const QString &title, int limit);
    static MetaPtrList searchMediaMeta(const QString &title, int limit);
    static MetaPtrList searchMediaPath(const QString &title, int limit);

    //! sync query interface
    static QStringList allPlaylistDisplayName();
    QList<PlaylistMeta> allPlaylistMeta();
    bool playlistExist(const QString &uuid);

signals:
    void addMediaMeta(const MetaPtr meta);
    void addMediaMetaList(const MetaPtrList metalist);
    void updateMediaMeta(const MetaPtr meta);
    void updateMediaMetaList(const MetaPtrList metalist);
    void insertMusic(const MetaPtr meta, const PlaylistMeta &playlistMeta);
    void insertMusicList(const MetaPtrList metalist, const PlaylistMeta &playlistMeta);
    void removeMediaMetaList(const MetaPtrList metalist);

public slots:
    static void addPlaylist(const PlaylistMeta &playlistMeta);
    static void updatePlaylist(const PlaylistMeta &playlistMeta);
    static void removePlaylist(const PlaylistMeta &playlistMeta);
    static void deleteMusic(const MetaPtr meta, const PlaylistMeta &playlistMeta);

private:
    void bind();

    friend class DMusic::DSingleton<MediaDatabase>;
    MediaDatabaseWriter *m_writer;
};

