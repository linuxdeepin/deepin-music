/*
 * Copyright (C) 2016 ~ 2018 Wuhan Deepin Technology Co., Ltd.
 *
 * Author:     Iceyer <me@iceyer.net>
 *
 * Maintainer: Iceyer <me@iceyer.net>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "playlistmanager.h"

#include <QDebug>
#include <QUuid>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlDatabase>

#include "mediadatabase.h"

const QString AlbumMusicListID      = "album";
const QString ArtistMusicListID     = "artist";
const QString AllMusicListID        = "all";
const QString FavMusicListID        = "fav";
const QString PlayMusicListID       = "play";
const QString MusicCandListID       = "musicCand";
const QString AlbumCandListID       = "albumCand";
const QString ArtistCandListID      = "artistCand";
const QString MusicResultListID     = "musicResult";
const QString AlbumResultListID     = "albumResult";
const QString ArtistResultListID    = "artistResult";
const QString SearchMusicListID     = "search";
const QString NewMusicListID        = "new";

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

    if (sortUUIDs.size() <= d->playlists.size()) {
        qWarning() << "playlist order crash, restrot";
        d->sortUUIDs.clear();
        d->sortUUIDs << AlbumMusicListID
                     << ArtistMusicListID
                     << AllMusicListID
                     << FavMusicListID
                     << PlayMusicListID
                     << MusicCandListID
                     << AlbumCandListID
                     << ArtistCandListID
                     << MusicResultListID
                     << AlbumResultListID
                     << ArtistResultListID
                     << SearchMusicListID
                     << NewMusicListID;

        QStringList sortUUIDs;
        for (auto playlist : d->playlists.values()) {
            sortUUIDs <<  playlist->id();
        }

        sortUUIDs.removeAll(AlbumMusicListID);
        sortUUIDs.removeAll(ArtistMusicListID);
        sortUUIDs.removeAll(AllMusicListID);
        sortUUIDs.removeAll(FavMusicListID);
        sortUUIDs.removeAll(PlayMusicListID);
        sortUUIDs.removeAll(MusicCandListID);
        sortUUIDs.removeAll(AlbumCandListID);
        sortUUIDs.removeAll(ArtistCandListID);
        sortUUIDs.removeAll(MusicResultListID);
        sortUUIDs.removeAll(AlbumResultListID);
        sortUUIDs.removeAll(ArtistResultListID);
        sortUUIDs.removeAll(SearchMusicListID);
        sortUUIDs.removeAll(NewMusicListID);

        d->sortUUIDs << sortUUIDs;

        // saveSortOrder();
    }


    d->sortUUIDs.clear();
    for (auto sortID = 0; sortID < sortUUIDs.size(); ++sortID) {
        d->sortUUIDs << sortUUIDs.value(static_cast<uint>(sortID));
    }


    auto album = playlist(AlbumMusicListID);
    auto trAlbumName = tr("Albums");
    if (!album.isNull() && album->displayName() != trAlbumName) {
        album->setDisplayName(trAlbumName);
    }

    auto artist = playlist(ArtistMusicListID);
    auto trArtistName = tr("Artists");
    if (!artist.isNull() && artist->displayName() != trArtistName) {
        artist->setDisplayName(trArtistName);
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

    auto play = playlist(PlayMusicListID);
    auto playName = tr("Play");
    if (!play.isNull() && play->displayName() != playName) {
        play->setDisplayName(playName);
    }

    auto musicCand = playlist(MusicCandListID);
    auto strMusic = tr("All Music");
    if (!musicCand.isNull() && musicCand->displayName() != playName) {
        musicCand->setDisplayName(strMusic);
    }

    auto albumCand = playlist(AlbumCandListID);
    auto strAlbum = tr("Albums");
    if (!albumCand.isNull() && albumCand->displayName() != playName) {
        albumCand->setDisplayName(strAlbum);
    }

    auto artistCand = playlist(ArtistCandListID);
    auto strArtist = tr("Artists");
    if (!artistCand.isNull() && artistCand->displayName() != playName) {
        artistCand->setDisplayName(strArtist);
    }

    auto musicResult = playlist(MusicResultListID);
    auto resMusic = tr("Music");
    if (!musicResult.isNull() && musicResult->displayName() != playName) {
        musicResult->setDisplayName(resMusic);
    }

    auto albumResult = playlist(AlbumResultListID);
    auto resAlbum = tr("Albums");
    if (!albumResult.isNull() && albumResult->displayName() != playName) {
        albumResult->setDisplayName(resAlbum);
    }

    auto artistResult = playlist(ArtistResultListID);
    auto resArtist = tr("Artists");
    if (!artistResult.isNull() && artistResult->displayName() != playName) {
        artistResult->setDisplayName(resArtist);
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
        if (d->playlists.value(uuid) != nullptr)
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
        Q_EMIT playlistRemove(playlist);
        d->playlists.remove(uuid);
        d->sortUUIDs.removeAll(uuid);
        PlaylistMeta listmeta;
        listmeta.uuid = deleteID;

        QSqlDatabase::database().transaction();
        MediaDatabase::removePlaylist(listmeta);
        saveSortOrder();
        QSqlDatabase::database().commit();
    });
    connect(playlist.data(), &Playlist::displayNameChanged,
    this, [ = ](QString displayName) {
        QSqlDatabase::database().transaction();
        playlist.data()->setDisplayName(displayName);
        QSqlDatabase::database().commit();
    });

    connect(playlist.data(), &Playlist::musiclistAdded,
    this, [ = ](const MetaPtrList metalist) {
        Q_EMIT musiclistAdded(playlist, metalist);
    });

    connect(playlist.data(), &Playlist::musiclistRemoved,
    this, [ = ](const MetaPtrList metalist) {
        Q_EMIT musiclistRemoved(playlist, metalist);
    });
}
