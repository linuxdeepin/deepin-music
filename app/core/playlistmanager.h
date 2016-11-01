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

    Q_PROPERTY(QSharedPointer<Playlist> playingPlaylist READ playingPlaylist WRITE setPlayingPlaylist NOTIFY playingPlaylistChanged)
    Q_PROPERTY(QSharedPointer<Playlist> selectedPlaylist READ selectedPlaylist WRITE setSelectedPlaylist NOTIFY selectedPlaylistChanged)
public:
    explicit PlaylistManager(QObject *parent = 0);
    ~PlaylistManager();

    QString newID();
    QString newDisplayName();

    QList<QSharedPointer<Playlist> > allplaylist();
    QSharedPointer<Playlist> playlist(const QString &id);
    QSharedPointer<Playlist> playingPlaylist() const;
    QSharedPointer<Playlist> selectedPlaylist() const;

    QSharedPointer<Playlist> addPlaylist(const PlaylistMeta &listinfo);

    void load();
    void sync();

signals:
    void playingPlaylistChanged(QSharedPointer<Playlist> playingPlaylist);
    void selectedPlaylistChanged(QSharedPointer<Playlist> selectedPlaylist);
    void musicAdded(QSharedPointer<Playlist> palylist, const MusicMeta &info);
    void musiclistAdded(QSharedPointer<Playlist> palylist, const MusicMetaList &metalist);
    void musicRemoved(QSharedPointer<Playlist> palylist, const MusicMeta &info);

public slots:
    void setPlayingPlaylist(QSharedPointer<Playlist> playingPlaylist);
    void setSelectedPlaylist(QSharedPointer<Playlist> selectedPlaylist);

private:
    QString getPlaylistPath(const QString &id);
    void insertPlaylist(const QString &id, QSharedPointer<Playlist>);

    QSettings                                   settings;
    QSharedPointer<Playlist>                    m_playingPlaylist;
    QSharedPointer<Playlist>                    m_selectedPlaylist;
    QStringList                                 sortPlaylists;
    QMap<QString, QSharedPointer<Playlist>>     playlists;
};

Q_DECLARE_METATYPE(QSharedPointer<Playlist>);
Q_DECLARE_METATYPE(QList<QSharedPointer<Playlist> >);

#endif // PLAYLISTMANAGER_H
