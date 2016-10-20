/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "playlist.h"
#include <QDebug>

Playlist::Playlist(const MusicListInfo &musiclistinfo, QObject *parent)
    : QObject(parent), settings(musiclistinfo.url, QSettings::IniFormat)
{
    listinfo = musiclistinfo;
}

int Playlist::length()
{
    return listinfo.musicMap.size();
}

bool Playlist::contains(const MusicInfo &info)
{
    return listinfo.musicMap.contains(info.id);
}

QString Playlist::id()
{
    return listinfo.id;
}

QString Playlist::displayName()
{
    return listinfo.displayName;
}

QString Playlist::icon()
{
    return listinfo.icon;
}

bool Playlist::readonly()
{
    return listinfo.readonly;
}

bool Playlist::editmode()
{
    return listinfo.editmode;
}

MusicList Playlist::allmusic()
{
    MusicList mlist;
    for (auto id : listinfo.musicIds) {
        mlist << listinfo.musicMap.value(id);
    }
    return mlist;
}

void Playlist::load()
{
    settings.beginGroup("PlaylistInfo");
    listinfo.id = settings.value("Title").toString();
    listinfo.editmode = settings.value("EditMode").toBool();
    listinfo.displayName = settings.value("DisplayName").toString();
    listinfo.icon = settings.value("Icon").toString();
    listinfo.readonly = settings.value("Readonly").toBool();
    listinfo.musicIds = settings.value("SortIds").toStringList();
    settings.endGroup();

    for (auto &key : listinfo.musicIds) {
        settings.beginGroup(key);
        MusicInfo info;
        info.id = key;
        info.url = settings.value("Url").toString();;
        info.track = settings.value("Track").toLongLong();
        info.title = settings.value("Title").toString();
        info.artist = settings.value("Artist").toString();
        info.album = settings.value("Album").toString();
        settings.endGroup();

        listinfo.musicMap.insert(key, info);
    }
}

void Playlist::save()
{
    settings.beginGroup("PlaylistInfo");
    settings.setValue("Title", listinfo.id);
    settings.setValue("NumberOfEntries", listinfo.musicIds.length());
    settings.setValue("EditMode", false);
    settings.setValue("DisplayName", listinfo.displayName);
    settings.setValue("Icon", listinfo.icon);
    settings.setValue("Readonly", listinfo.readonly);
    settings.setValue("SortIds", listinfo.musicIds);
    settings.endGroup();

    for (auto &id : listinfo.musicIds) {
        auto info = listinfo.musicMap.value(id);
        settings.beginGroup(id);
        settings.setValue("Url", info.url);
        settings.setValue("Track", info.track);
        settings.setValue("Title", info.title);
        settings.setValue("Artist", info.artist);
        settings.setValue("Album", info.album);
        settings.endGroup();
    }

    settings.sync();
}

void Playlist::setDisplayName(const QString &name)
{
    listinfo.displayName = name;
    settings.beginGroup("PlaylistInfo");
    settings.setValue("DisplayName", name);
    settings.endGroup();
    settings.sync();
}

void Playlist::appendMusic(const MusicInfo &info)
{
    if (listinfo.musicMap.contains(info.id)) {
        qDebug() << "add dump music " << info.id << info.url;
        return;
    }

    listinfo.musicIds << info.id;
    listinfo.musicMap.insert(info.id, info);

    settings.beginGroup("PlaylistInfo");
    settings.setValue("SortIds", listinfo.musicIds);
    settings.endGroup();

    settings.beginGroup(info.id);
    settings.setValue("Url", info.url);
    settings.setValue("Track", info.track);
    settings.setValue("Title", info.title);
    settings.setValue("Artist", info.artist);
    settings.setValue("Album", info.album);
    settings.endGroup();
    settings.sync();

    emit musicAdded(info);
}

void Playlist::removeMusic(const MusicInfo &info)
{
    if (info.id.isEmpty()) {
        qCritical() << "Cannot remove empty id";
        return;
    }
    if (!listinfo.musicMap.contains(info.id)) {
        qWarning() << "no such id in playlist" << info.id << info.url << listinfo.displayName;
        return;
    }
    listinfo.musicIds.removeAll(info.id);
    listinfo.musicMap.remove(info.id);

    settings.beginGroup("PlaylistInfo");
    settings.setValue("SortIds", listinfo.musicIds);
    settings.endGroup();

    settings.beginGroup(info.id);
    settings.remove("");
    settings.endGroup();

    settings.sync();

    emit musicRemoved(info);
}
