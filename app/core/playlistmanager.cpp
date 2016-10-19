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

#include "../musicapp.h"

static const QString DefaultListID = "Default";
const QString AllMusicListID = "All";
const QString FavMusicListID = "Fav";
static MusicListInfo emptyInfo;

PlaylistManager::PlaylistManager(QObject *parent)
    : QObject(parent), listmgrSetting(MusicApp::configPath() + "/Playlist.ini", QSettings::IniFormat)
{
}

PlaylistManager::~PlaylistManager()
{
    listmgrSetting.sync();
}

QString PlaylistManager::newID()
{
    return QUuid::createUuid().toString();
}

QString PlaylistManager::newDisplayName()
{
    QMap<QString, QString> existName;
    listmgrSetting.beginGroup("PlaylistEntry");
    for (auto &key : listmgrSetting.allKeys()) {
        auto name = listmgrSetting.value(key).toString();
        existName.insert(name, name);
    }
    listmgrSetting.endGroup();

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
    listmgrSetting.beginGroup("PlaylistManager");
    auto currentTitle = listmgrSetting.value("Current").toString();
    listmgrSetting.endGroup();

    listmgrSetting.beginGroup("PlaylistEntry");
    for (auto &playlistid : listmgrSetting.allKeys()) {
        qDebug() << playlistid;
        auto playlistPath = getPlaylistPath(playlistid);
        QFileInfo fi(playlistPath);
        if (!fi.exists()) {
            qDebug() << playlistid;
            continue;
        }
        MusicListInfo info;
        info.url = playlistPath;
        auto emptylist = QSharedPointer<Playlist>(new Playlist(info));
        emptylist->load();
        insertPlaylist(playlistid, emptylist);
    }
    listmgrSetting.endGroup();

    if (playlist(AllMusicListID).isNull()) {
        MusicListInfo info;
        info.id = AllMusicListID;
        info.editmode = false;
        info.icon = "all";
        info.readonly = true;
        info.displayName = tr("All Music");
        addplaylist(info);
        playlist(AllMusicListID)->save();
    }

    if (playlist(FavMusicListID).isNull()) {
        MusicListInfo info;
        info.id = FavMusicListID;
        info.editmode = false;
        info.icon = "fav";
        info.readonly = true;
        info.displayName = tr("Favator Music");
        addplaylist(info);
        playlist(FavMusicListID)->save();
    }
    qDebug() << playlists.size();
    m_currentPlaylist = playlist(currentTitle);
    if (m_currentPlaylist.isNull()) {
        qDebug() << "Change Default";
        m_currentPlaylist = playlist(AllMusicListID);
    }
    qDebug() << m_currentPlaylist->info().displayName;
}

void PlaylistManager::sync()
{
    listmgrSetting.sync();
}

QList<QSharedPointer<Playlist> > PlaylistManager::allplaylist()
{
    QList<QSharedPointer<Playlist> >  list;
    for (auto &playlist : playlists) {
        list << playlist;
    }
    return list;
}

QSharedPointer<Playlist> PlaylistManager::addplaylist(const MusicListInfo &info)
{
    MusicListInfo saveInfo(info);
    QString playlistPath = getPlaylistPath(info.id);
    listmgrSetting.beginGroup("PlaylistEntry");
    listmgrSetting.setValue(info.id, info.displayName);
    listmgrSetting.endGroup();
    listmgrSetting.sync();
    saveInfo.url = playlistPath;

    insertPlaylist(info.id, QSharedPointer<Playlist>(new Playlist(saveInfo)));
    playlists.value(info.id)->save();
    return playlists.value(info.id);
}

QSharedPointer<Playlist> PlaylistManager::playlist(const QString &id)
{
    return playlists.value(id);
}

void PlaylistManager::setCurrentPlaylist(QSharedPointer<Playlist> currentPlaylist)
{
    if (m_currentPlaylist == currentPlaylist) {
        return;
    }
    m_currentPlaylist = currentPlaylist;
    emit currentPlaylistChanged(currentPlaylist);
    listmgrSetting.beginGroup("PlaylistManager");
    listmgrSetting.setValue("Current", m_currentPlaylist->info().id);
    listmgrSetting.endGroup();
    listmgrSetting.sync();
}

QString PlaylistManager::getPlaylistPath(const QString &id)
{
    return MusicApp::configPath() + "/" + id + ".plsx";
}

void PlaylistManager::insertPlaylist(const QString &id, QSharedPointer<Playlist> playlist)
{
    QString deleteID = playlist->info().id;
    connect(playlist.data(), &Playlist::removed,
    this, [ = ] {
        if (m_currentPlaylist.isNull() || m_currentPlaylist->info().id == deleteID)
        {
            setCurrentPlaylist(this->playlist(AllMusicListID));
        }
        QFile::remove(getPlaylistPath(deleteID));
        playlists.remove(deleteID);
        listmgrSetting.beginGroup("PlaylistEntry");
        listmgrSetting.remove(deleteID);
        listmgrSetting.endGroup();
        listmgrSetting.sync();
    });

    playlists.insert(id, playlist);
}
