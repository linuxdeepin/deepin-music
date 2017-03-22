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
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlDatabase>

#include "mediadatabase.h"

const QString AllMusicListID = "all";
const QString FavMusicListID = "fav";
const QString SearchMusicListID = "search";
const QString NewMusicListID = "new";

static PlaylistMeta emptyInfo;

class PlaylistManagerPrivate
{
public:
    PlaylistManagerPrivate(PlaylistManager *parent) : q_ptr(parent) {}

    inline uint indexUUID(const QString &uuid)
    {
        return sortUUIDs.indexOf(uuid);
    }

    QStringList                 sortUUIDs;
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

    QString temp = tr("New playlist");
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
    Q_D(PlaylistManager);

    for (auto &playlistmeta : MediaDatabase::instance()->allPlaylistMeta()) {
        PlaylistPtr emptylist(new Playlist(playlistmeta));
        emptylist->load();
        insertPlaylist(playlistmeta.uuid, emptylist);
    }

    QMap<uint, QString> sortUUIDs;
    for (auto playlist : d->playlists) {
        sortUUIDs.insert(playlist->sortID(), playlist->id());
    }

    QSqlDatabase::database().transaction();

    if (sortUUIDs.size() != d->playlists.size()) {
        qWarning() << "playlist order crash, restrot";
        d->sortUUIDs.clear();
        d->sortUUIDs << SearchMusicListID << AllMusicListID << FavMusicListID;

        QStringList sortUUIDs;
        for (auto playlist : d->playlists.values()) {
            sortUUIDs <<  playlist->id();
        }

        sortUUIDs.removeAll(SearchMusicListID);
        sortUUIDs.removeAll(AllMusicListID);
        sortUUIDs.removeAll(FavMusicListID);

        d->sortUUIDs << sortUUIDs;

        saveSortOrder();
    } else {
        for (auto sortID = 0; sortID < sortUUIDs.size(); ++sortID) {
            d->sortUUIDs << sortUUIDs.value(static_cast<uint>(sortID));
        }
    }

    auto all = playlist(AllMusicListID);
    auto trAllName = tr("All Music");
    if (!all.isNull() && all->displayName() != trAllName) {
        all->setDisplayName(trAllName);
    }

    auto fav = playlist(FavMusicListID);
    auto trFavName = tr("My favorites");
    if (!fav.isNull() && fav->displayName() != trFavName) {
        fav->setDisplayName(trFavName);
    }

    auto search = playlist(SearchMusicListID);
    auto searchName = tr("Search result");
    if (!search.isNull() && search->displayName() != searchName) {
        search->setDisplayName(searchName);
    }
    QSqlDatabase::database().commit();
}

void PlaylistManager::saveSortOrder()
{
    Q_D(PlaylistManager);

    qDebug() << d->sortUUIDs;

    for (int sortID = 0; sortID < d->sortUUIDs.length(); ++sortID) {
        auto uuid = d->sortUUIDs.value(sortID);
        QSqlQuery query;
        query.prepare(QString("UPDATE playlist SET sort_id = :sort_id WHERE uuid = :uuid; "));
        query.bindValue(":sort_id", sortID);
        query.bindValue(":uuid", uuid);
        if (! query.exec()) {
            qDebug() << query.lastError();
        }
    }
}

QList<PlaylistPtr > PlaylistManager::allplaylist()
{
    Q_D(PlaylistManager);

    QList<PlaylistPtr >  list;
    for (auto &uuid : d->sortUUIDs) {
        list << d->playlists.value(uuid);
    }
    return list;
}

PlaylistPtr PlaylistManager::addPlaylist(const PlaylistMeta &listinfo)
{
    Q_D(PlaylistManager);
    PlaylistMeta saveInfo(listinfo);
    saveInfo.sortID = d->sortUUIDs.length();
    d->sortUUIDs << saveInfo.uuid;
    insertPlaylist(listinfo.uuid, PlaylistPtr(new Playlist(saveInfo)));
    MediaDatabase::addPlaylist(saveInfo);
    return d->playlists.value(listinfo.uuid);
}

void PlaylistManager::onCustomResort(QStringList uuids)
{
    Q_D(PlaylistManager);
    Q_ASSERT(uuids.length() == d->playlists.size() - 1);

    uuids.push_front(playlist(SearchMusicListID)->id());

    d->sortUUIDs.clear();
    for (auto sortID = 0; sortID < uuids.length(); ++sortID) {
        d->sortUUIDs << uuids.value(sortID);
    }

    QSqlDatabase::database().transaction();
    saveSortOrder();
    QSqlDatabase::database().commit();
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

    d->playlists.insert(uuid, playlist);

    connect(playlist.data(), &Playlist::removed,
    this, [ = ] {
        qDebug() << "remove playlist" << deleteID << d->playlists.size();
        emit playlistRemove(playlist);
        d->playlists.remove(uuid);
        d->sortUUIDs.removeAll(uuid);
        PlaylistMeta listmeta;
        listmeta.uuid = deleteID;

        QSqlDatabase::database().transaction();
        MediaDatabase::removePlaylist(listmeta);
        saveSortOrder();
        QSqlDatabase::database().commit();
    });

    connect(playlist.data(), &Playlist::musiclistAdded,
    this, [ = ](const MetaPtrList metalist) {
        emit musiclistAdded(playlist, metalist);
    });

    connect(playlist.data(), &Playlist::musiclistRemoved,
    this, [ = ](const MetaPtrList metalist) {
        emit musiclistRemoved(playlist, metalist);
    });
}
