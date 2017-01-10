/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef PLAYLISTMANAGER_H
#define PLAYLISTMANAGER_H

#include <QMap>
#include <QObject>
#include <QSettings>
#include <QSharedPointer>

#include "playlist.h"

class PlaylistManager : public QObject
{
    Q_OBJECT

    Q_PROPERTY(PlaylistPtr playingPlaylist READ playingPlaylist WRITE setPlayingPlaylist NOTIFY playingPlaylistChanged)
    Q_PROPERTY(PlaylistPtr selectedPlaylist READ selectedPlaylist WRITE setSelectedPlaylist NOTIFY selectedPlaylistChanged)
public:
    explicit PlaylistManager(QObject *parent = 0);
    ~PlaylistManager();

    QString newID();
    QString newDisplayName();

    QList<PlaylistPtr > allplaylist();
    PlaylistPtr playlist(const QString &id);
    PlaylistPtr playingPlaylist() const;
    PlaylistPtr selectedPlaylist() const;

    PlaylistPtr addPlaylist(const PlaylistMeta &listinfo);

    void load();
    void sync();

signals:
    void playingPlaylistChanged(PlaylistPtr playingPlaylist);
    void selectedPlaylistChanged(PlaylistPtr selectedPlaylist);
    void musicAdded(PlaylistPtr playlist, const MusicMeta &info);
    void musiclistAdded(PlaylistPtr playlist, const MusicMetaList &metalist);
    void musicRemoved(PlaylistPtr playlist, const MusicMeta &info);
    void playlistRemove(PlaylistPtr playlist);

public slots:
    void setPlayingPlaylist(PlaylistPtr playingPlaylist);
    void setSelectedPlaylist(PlaylistPtr selectedPlaylist);

private:
    QString getPlaylistPath(const QString &id);
    void insertPlaylist(const QString &id, PlaylistPtr);

    QSettings                                   settings;
    PlaylistPtr                    m_playingPlaylist;
    PlaylistPtr                    m_selectedPlaylist;
    QStringList                                 sortPlaylists;
    QMap<QString, PlaylistPtr>     playlists;
};

Q_DECLARE_METATYPE(PlaylistPtr);
Q_DECLARE_METATYPE(QList<PlaylistPtr >);

#endif // PLAYLISTMANAGER_H
