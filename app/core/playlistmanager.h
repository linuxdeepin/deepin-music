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

    void load();
    void sync();

    QList<QSharedPointer<Playlist> > allplaylist();
    QSharedPointer<Playlist> addplaylist(const MusicListInfo &info);
    QSharedPointer<Playlist> playlist(const QString &id);

    QSharedPointer<Playlist> currentPlaylist() const
    {
        return m_currentPlaylist;
    }

signals:
    void currentPlaylistChanged(QSharedPointer<Playlist> currentPlaylist);

public slots:
    void setCurrentPlaylist(QSharedPointer<Playlist> currentPlaylist);

private:
    QString getPlaylistPath(const QString &id);
    void insertPlaylist(const QString &id, QSharedPointer<Playlist>);

    QSettings                                   listmgrSetting;
    QSharedPointer<Playlist>                    m_currentPlaylist;
    QMap<QString, QSharedPointer<Playlist>>     playlists;
};

Q_DECLARE_METATYPE(QSharedPointer<Playlist>);

extern const QString AllMusicListID;
extern const QString FavMusicListID;

#endif // PLAYLISTMANAGER_H
