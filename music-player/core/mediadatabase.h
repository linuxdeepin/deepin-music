/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef MEDIADATABASE_H
#define MEDIADATABASE_H

#include <QObject>

#include "mediadatabasewriter.h"

class MediaDatabase : public QObject
{
    Q_OBJECT
public:
    explicit MediaDatabase(QObject *parent = 0);

    static MediaDatabase* instance() {
        static auto s_instance = new MediaDatabase;
        return s_instance;
    }

    //! music meta query interface
    bool musicMetaExist(const QString &hash);
    static QList<MusicMeta> searchMusicTitle(const QString &title, int limit);
    static QList<MusicMeta> searchMusicMeta(const QString &title, int limit);
    static QList<MusicMeta> searchMusicPath(const QString &title, int limit);

    //! sync query interface
    static QStringList allPlaylistDisplayName();
    QList<PlaylistMeta> allPlaylist();
    bool playlistExist(const QString &uuid);

signals:
    void addMusicMeta(const MusicMeta &meta);
    void addMusicMetaList(const MusicMetaList &metalist);
    void updateMusicMeta(const MusicMeta &meta);
    void updateMusicMetaList(const MusicMetaList &metalist);
    void insertMusic(const MusicMeta &meta, const PlaylistMeta &playlistMeta);
    void insertMusicList(const MusicMetaList &metalist, const PlaylistMeta &playlistMeta);
    void removeMusicMetaList(const MusicMetaList &metalist);

public slots:
    static void addPlaylist(const PlaylistMeta &playlistMeta);
    static void updatePlaylist(const PlaylistMeta &playlistMeta);
    static void removePlaylist(const PlaylistMeta &playlistMeta);
    static void deleteMusic(const MusicMeta &meta, const PlaylistMeta &playlistMeta);

private:
    void bind();

    MediaDatabaseWriter *m_writer;
};

#endif // MEDIADATABASE_H
