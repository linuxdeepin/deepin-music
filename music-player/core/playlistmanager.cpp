/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "playlistmanager.h"

#include <QFileInfo>
#include <QDebug>
#include <QUuid>
#include <QApplication>
#include <QThread>

#include "../musicapp.h"
#include "mediadatabase.h"

const QString AllMusicListID = "all";
const QString FavMusicListID = "fav";
const QString SearchMusicListID = "search";
const QString NewMusicListID = "new";

static PlaylistMeta emptyInfo;

PlaylistManager::PlaylistManager(QObject *parent)
    : QObject(parent), settings(MusicApp::configPath() + "/playlist.ini", QSettings::IniFormat)
{
}

PlaylistManager::~PlaylistManager()
{
    settings.sync();
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
    for (auto &playlistmeta : MediaDatabase::instance()->allPlaylist()) {
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

    auto currentTitle = AllMusicListID;
    m_playingPlaylist = playlist(currentTitle);
    if (m_playingPlaylist.isNull()) {
        qWarning() << "change to default all playlist";
        m_playingPlaylist = playlist(AllMusicListID);
    }
    m_selectedPlaylist = m_playingPlaylist;
}

void PlaylistManager::sync()
{
    settings.sync();
}

QList<PlaylistPtr > PlaylistManager::allplaylist()
{
    QList<PlaylistPtr >  list;
    for (auto &playlist : sortPlaylists) {
        list << playlists.value(playlist);
    }
    return list;
}

PlaylistPtr PlaylistManager::addPlaylist(const PlaylistMeta &listinfo)
{
    PlaylistMeta saveInfo(listinfo);
    QString playlistPath = getPlaylistPath(listinfo.uuid);
    saveInfo.url = playlistPath;
    insertPlaylist(listinfo.uuid, PlaylistPtr(new Playlist(saveInfo)));

    MediaDatabase::addPlaylist(saveInfo);

    return playlists.value(listinfo.uuid);
}

PlaylistPtr PlaylistManager::playlist(const QString &id)
{
    return playlists.value(id);
}

PlaylistPtr PlaylistManager::playingPlaylist() const
{
    return m_playingPlaylist;
}

PlaylistPtr PlaylistManager::selectedPlaylist() const
{
    return m_selectedPlaylist;
}

void PlaylistManager::setPlayingPlaylist(PlaylistPtr currentPlaylist)
{
    if (m_playingPlaylist == currentPlaylist) {
        return;
    }
    m_playingPlaylist = currentPlaylist;
    emit playingPlaylistChanged(currentPlaylist);
    settings.beginGroup("PlaylistManager");
    settings.setValue("Current", m_playingPlaylist->id());
    settings.endGroup();
    settings.sync();
}

void PlaylistManager::setSelectedPlaylist(PlaylistPtr selectedPlaylist)
{
    if (m_selectedPlaylist == selectedPlaylist) {
        return;
    }

    m_selectedPlaylist = selectedPlaylist;
    emit selectedPlaylistChanged(selectedPlaylist);
}

QString PlaylistManager::getPlaylistPath(const QString &id)
{
    return MusicApp::configPath() + "/" + id + ".plsx";
}

void PlaylistManager::insertPlaylist(const QString &uuid, PlaylistPtr playlist)
{
    QString deleteID = uuid;

    connect(playlist.data(), &Playlist::removed,
    this, [ = ] {
        qDebug() << deleteID << m_playingPlaylist;
        emit playlistRemove(playlist);

        if (m_selectedPlaylist.isNull() || m_selectedPlaylist->id() == deleteID)
        {
            setSelectedPlaylist(this->playlist(AllMusicListID));
        }
        sortPlaylists.removeAll(deleteID);
        PlaylistMeta listmeta;
        listmeta.uuid = deleteID;
        MediaDatabase::removePlaylist(listmeta);
    });

    connect(playlist.data(), &Playlist::musicAdded,
    this, [ = ](const MusicMeta & info) {
        emit musicAdded(playlist,  info);
    });

    connect(playlist.data(), &Playlist::musiclistAdded,
    this, [ = ](const MusicMetaList & info) {
        emit musiclistAdded(playlist, info);
    });

    connect(playlist.data(), &Playlist::musicRemoved,
    this, [ = ](const MusicMeta & info) {
        emit musicRemoved(playlist, info);
    });

    sortPlaylists << uuid;
    playlists.insert(uuid, playlist);
}
