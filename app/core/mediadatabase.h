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

#include "../model/musiclistmodel.h"

class MusicMeta;
class PlaylistMeta;
class MediaDatabase : public QObject
{
    Q_OBJECT
public:
    explicit MediaDatabase(QObject *parent = 0);

    static QStringList allPlaylistDisplayName();
    QList<PlaylistMeta> allPlaylist();
signals:

public slots:
    static QList<MusicMeta> searchMusicTitle(const QString &title, int limit);
    static QList<MusicMeta> searchMusicInfo(const QString &title, int limit);

public slots:
    static void addPlaylist(const PlaylistMeta &palylistMeta);
    static void updatePlaylist(const PlaylistMeta &palylistMeta);
    static void removePlaylist(const PlaylistMeta &palylistMeta);

    static void addMusicMeta(const MusicMeta &meta);
    static void removeMusicMeta(const MusicMeta &meta);

    static void insertMusic(const MusicMeta &meta, const PlaylistMeta &palylistMeta);
    static void deleteMusic(const MusicMeta &meta, const PlaylistMeta &palylistMeta);

private:
    bool playlistExist(const QString &uuid);
};

#endif // MEDIADATABASE_H
