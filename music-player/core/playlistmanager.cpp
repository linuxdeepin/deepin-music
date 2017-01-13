/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "playlistmanager.h"

#include <QDebug>
#include <QUuid>

#include "mediadatabase.h"

const QString AllMusicListID = "all";
const QString FavMusicListID = "fav";
const QString SearchMusicListID = "search";
const QString NewMusicListID = "new";

static PlaylistMeta emptyInfo;

class PlaylistManagerPrivate
{
public:
    PlaylistManagerPrivate(PlaylistManager *parent) : q_ptr(parent){}

    QStringList                 sortPlaylists;
    QMap<QString, PlaylistPtr>  playlists;

    PlaylistManager *q_ptr;
    Q_DECLARE_PUBLIC(PlaylistManager)
};

PlaylistManager::PlaylistManager(QObject *parent) :
    QObject(parent), d_ptr(new PlaylistManagerPrivate(this))
{

}

PlaylistManager::~PlaylistManager()
{

}

QString PlaylistManager::newID()
{
    return QUuid::createUuid().toString().remove("{").remove("}").remove("-");
}

QString PlaylistManager::newDisplayName()
{
    QMap<QString, QString> existName;
    for (auto &name : MediaDatabase::allPlaylistDisplayName()) {
        existName.insert(name, name);
    }

    QString temp = tr("New Playlist");
    if (!existName.contains(temp)) {
        return temp;
    }

    int i = 1;
    for (i = 1; i < existName.size() + 1; ++i) {
        QString newName = QString("%1 %2").arg(temp).arg(i);
        if (!existName.contains(newName)) {
            return newName;
        }
    }
    return QString("%1 %2").arg(temp).arg(i);
}

void PlaylistManager::load()
{
    for (auto &playlistmeta : MediaDatabase::instance()->allPlaylistMeta()) {
        PlaylistPtr emptylist(new Playlist(playlistmeta));
        emptylist->load();
        insertPlaylist(playlistmeta.uuid, emptylist);
    }

    auto all = playlist(AllMusicListID);
    if (!all.isNull()) {
        all->setDisplayName("");
        all->setDisplayName(tr("All Music"));
    }
    auto fav = playlist(FavMusicListID);
    if (!fav.isNull()) {
        fav->setDisplayName("My favorites");
        fav->setDisplayName(tr("My favorites"));
    }
}

QList<PlaylistPtr > PlaylistManager::allplaylist()
{
    Q_D(PlaylistManager);

    QList<PlaylistPtr >  list;
    for (auto &playlist : d->sortPlaylists) {
        list << d->playlists.value(playlist);
    }
    return list;
}

PlaylistPtr PlaylistManager::addPlaylist(const PlaylistMeta &listinfo)
{
    Q_D(PlaylistManager);
    PlaylistMeta saveInfo(listinfo);
    insertPlaylist(listinfo.uuid, PlaylistPtr(new Playlist(saveInfo)));

    MediaDatabase::addPlaylist(saveInfo);

    return d->playlists.value(listinfo.uuid);
}

PlaylistPtr PlaylistManager::playlist(const QString &id)
{
    Q_D(PlaylistManager);
    return d->playlists.value(id);
}

void PlaylistManager::insertPlaylist(const QString &uuid, PlaylistPtr playlist)
{
    Q_D(PlaylistManager);
    QString deleteID = uuid;

    connect(playlist.data(), &Playlist::removed,
    this, [ = ] {
        qDebug() << "remove playlist" << deleteID;
        emit playlistRemove(playlist);

        d->sortPlaylists.removeAll(deleteID);
        PlaylistMeta listmeta;
        listmeta.uuid = deleteID;
        MediaDatabase::removePlaylist(listmeta);
    });

//    connect(playlist.data(), &Playlist::musicAdded,
//    this, [ = ](const MetaPtr meta) {
//        emit musicAdded(playlist, meta);
//    });

    connect(playlist.data(), &Playlist::musiclistAdded,
    this, [ = ](const MetaPtrList metalist) {
        emit musiclistAdded(playlist, metalist);
    });

    connect(playlist.data(), &Playlist::musiclistRemoved,
    this, [ = ](const MetaPtrList metalist) {
        emit musiclistRemoved(playlist, metalist);
    });

    d->sortPlaylists << uuid;
    d->playlists.insert(uuid, playlist);
}
