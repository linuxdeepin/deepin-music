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

#include <QObject>
#include <QMap>
#include <QSharedPointer>

#include "playlist.h"

#include <QSettings>

class PlaylistManager : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QSharedPointer<Playlist> currentPlaylist READ currentPlaylist WRITE setCurrentPlaylist NOTIFY currentPlaylistChanged)
public:
    explicit PlaylistManager(QObject *parent = 0);
    ~PlaylistManager();

    QString newID();
    QString newDisplayName();

    int playMode();
    QList<QSharedPointer<Playlist> > allplaylist();
    QSharedPointer<Playlist> currentPlaylist() const;
    QSharedPointer<Playlist> playlist(const QString &id);

    QSharedPointer<Playlist> addPlaylist(const MusicListInfo &listinfo);

    void load();
    void sync();
signals:
    void currentPlaylistChanged(QSharedPointer<Playlist> currentPlaylist);

public slots:
    void setCurrentPlaylist(QSharedPointer<Playlist> currentPlaylist);

private:
    QString getPlaylistPath(const QString &id);
    void insertPlaylist(const QString &id, QSharedPointer<Playlist>);

    int                                         m_palyMode;
    QSettings                                   settings;
    QSharedPointer<Playlist>                    m_currentPlaylist;
    QStringList                                 sortPlaylists;
    QMap<QString, QSharedPointer<Playlist>>     playlists;
};

Q_DECLARE_METATYPE(QSharedPointer<Playlist>);

extern const QString AllMusicListID;
extern const QString FavMusicListID;

#endif // PLAYLISTMANAGER_H
