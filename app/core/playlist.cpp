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
    m_history = listinfo.musicIds;
}

int Playlist::length()
{
    return listinfo.musicMap.size();
}

const MusicInfo Playlist::first()
{
    return listinfo.musicMap.value(listinfo.musicIds.value(0));
}

const MusicInfo Playlist::prev(const MusicInfo &info)
{
    if (0 == listinfo.musicIds.length()) {
        return MusicInfo();
    }
    auto index = listinfo.musicIds.indexOf(info.id);
    // can not find
    auto prev = (index + listinfo.musicIds.length() - 1) % listinfo.musicIds.length();
    return listinfo.musicMap.value(listinfo.musicIds.at(prev));
}

const MusicInfo Playlist::next(const MusicInfo &info)
{
    if (0 == listinfo.musicIds.length()) {
        return MusicInfo();
    }
    auto index = listinfo.musicIds.indexOf(info.id);
    // can not find
    auto prev = (index + 1) % listinfo.musicIds.length();
    return listinfo.musicMap.value(listinfo.musicIds.at(prev));
}

const MusicInfo Playlist::music(int index)
{
    return listinfo.musicMap.value(listinfo.musicIds.value(index));
}

const MusicInfo Playlist::music(const QString &id)
{
    return listinfo.musicMap.value(id);
}

bool Playlist::isLast(const MusicInfo &info)
{
    return listinfo.musicIds.last() == info.id;
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

void Playlist::buildHistory(const QString &last)
{
    auto lastindex = listinfo.musicIds.indexOf(last);
    m_history.clear();
    for (int i = lastindex + 1; i < listinfo.musicIds.length(); ++i) {
        m_history.append(listinfo.musicIds.value(i));
    }
    for (int i = 0; i <= lastindex; ++i) {
        m_history.append(listinfo.musicIds.value(i));
    }
}

void Playlist::clearHistory()
{
    m_history.clear();
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
        info.filetype = settings.value("Filetype").toString();
        info.length = settings.value("Length").toLongLong();
        info.size = settings.value("Size").toLongLong();
        settings.endGroup();

        listinfo.musicMap.insert(key, info);
    }
    m_history = listinfo.musicIds;
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
        settings.setValue("Length", info.length);
        settings.setValue("Filetype", info.filetype);
        settings.setValue("Size", info.size);
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
    settings.setValue("Length", info.length);
    settings.setValue("Filetype", info.filetype);
    settings.setValue("Size", info.size);
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
    m_history.removeAll(info.id);
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
