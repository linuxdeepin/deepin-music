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

class PlaylistManager : public QObject
{
    Q_OBJECT
public:
    explicit PlaylistManager(QObject *parent = 0);

    void load();

//    void sync();
signals:
//    Playlist &playlist(const QString &name);

public slots:
//    void addFilesToPlaylist();

private:
    QMap<QString, QSharedPointer<Playlist>>  playlists;
};

#endif // PLAYLISTMANAGER_H
