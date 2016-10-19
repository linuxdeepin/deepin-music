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

Playlist::Playlist(const MusicListInfo &info, QObject *parent)
    : QObject(parent), setting(info.url, QSettings::IniFormat)
{
    d = info;
}

MusicListInfo &Playlist::info()
{
    return d;
}

void Playlist::load()
{
    setting.beginGroup("PlaylistInfo");
    d.id = setting.value("Title").toString();
    d.editmode = setting.value("EditMode").toBool();
    d.displayName = setting.value("DisplayName").toString();
    d.icon = setting.value("Icon").toString();
    d.readonly = setting.value("Readonly").toBool();
    setting.endGroup();

    setting.beginGroup("MusicEntry");
    auto allkeys = setting.allKeys();
    setting.endGroup();
    for (auto &key : allkeys) {
        setting.beginGroup(key);
        MusicInfo info;
        info.url = setting.value("Url").toString();;
        info.track = setting.value("Track").toLongLong();
        info.title = setting.value("Title").toString();
        info.artist = setting.value("Artist").toString();
        info.album = setting.value("Album").toString();
        setting.endGroup();

        qDebug() << info.title << info.url;
        d.list.push_back(info);
    }
}

void Playlist::save()
{
    setting.beginGroup("PlaylistInfo");
    setting.setValue("Title", d.id);
    setting.setValue("NumberOfEntries", d.list.length());
    setting.setValue("EditMode", false);
    setting.setValue("DisplayName", d.displayName);
    setting.setValue("Icon", d.icon);
    setting.setValue("Readonly", d.readonly);
    setting.endGroup();

    setting.beginGroup("MusicEntry");
    int index = 0;
    for (auto &info : d.list) {
        index++;
        setting.setValue(QString("Music%1").arg(index), info.url);
    }
    setting.endGroup();

    index = 0;
    for (auto &info : d.list) {
        index++;
        setting.beginGroup(QString("Music%1").arg(index));
        setting.setValue("Url", info.url);
        setting.setValue("Track", info.track);
        setting.setValue("Title", info.title);
        setting.setValue("Artist", info.artist);
        setting.setValue("Album", info.album);
        setting.endGroup();
    }

    setting.sync();
}

void Playlist::onDisplayNameChanged(const QString &name)
{
    d.displayName = name;
    setting.beginGroup("PlaylistInfo");
    setting.setValue("DisplayName", name);
    setting.endGroup();
    setting.sync();
}

void Playlist::addMusic(const MusicInfo &info)
{
    // remove dump
    d.list.push_back(info);

    qDebug() << "add" << d.displayName << info.title;

    setting.beginGroup("MusicEntry");
    setting.setValue(QString("Music%1").arg(d.list.length()), info.url);
    setting.endGroup();

    setting.beginGroup(QString("Music%1").arg(d.list.length()));
    setting.setValue("Url", info.url);
    setting.setValue("Track", info.track);
    setting.setValue("Title", info.title);
    setting.setValue("Artist", info.artist);
    setting.setValue("Album", info.album);
    setting.endGroup();
    setting.sync();
}
