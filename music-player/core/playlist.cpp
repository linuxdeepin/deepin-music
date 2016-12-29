/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "playlist.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QCollator>
#include <QDebug>

#include "mediadatabase.h"

static QCollator collator;

Playlist::Playlist(const PlaylistMeta &musiclistinfo, QObject *parent)
    : QObject(parent)
{
    listmeta = musiclistinfo;
}

const MusicMeta Playlist::first() const
{
    return listmeta.metas.value(listmeta.sortMetas.value(0));
}

const MusicMeta Playlist::prev(const MusicMeta &info) const
{
    if (0 == listmeta.sortMetas.length()) {
        return MusicMeta();
    }
    auto index = listmeta.sortMetas.indexOf(info.hash);
    auto prev = (index + listmeta.sortMetas.length() - 1) % listmeta.sortMetas.length();
    return listmeta.metas.value(listmeta.sortMetas.at(prev));
}

const MusicMeta Playlist::next(const MusicMeta &info) const
{
    if (0 == listmeta.sortMetas.length()) {
        return MusicMeta();
    }
    auto index = listmeta.sortMetas.indexOf(info.hash);
    auto prev = (index + 1) % listmeta.sortMetas.length();
    return listmeta.metas.value(listmeta.sortMetas.at(prev));
}

const MusicMeta Playlist::music(int index) const
{
    return listmeta.metas.value(listmeta.sortMetas.value(index));
}

const MusicMeta Playlist::music(const QString &id) const
{
    return listmeta.metas.value(id);
}

const MusicMeta Playlist::playing() const
{
    return listmeta.playing;
}

bool Playlist::isLast(const MusicMeta &info) const
{
    return listmeta.sortMetas.last() == info.hash;
}

bool Playlist::contains(const MusicMeta &info) const
{
    return listmeta.metas.contains(info.hash);
}

QString Playlist::id() const
{
    return listmeta.uuid;
}

QString Playlist::displayName() const
{
    return listmeta.displayName;
}

QString Playlist::icon() const
{
    return listmeta.icon;
}

bool Playlist::readonly() const
{
    return listmeta.readonly;
}

bool Playlist::editmode() const
{
    return listmeta.editmode;
}

bool Playlist::hide() const
{
    return listmeta.hide;
}

bool Playlist::isEmpty() const
{
    return listmeta.metas.isEmpty();
}

bool Playlist::canNext() const
{
    return listmeta.invalidMetas.size() < listmeta.metas.size();
}

bool Playlist::active() const
{
    return  listmeta.active;
}

int Playlist::length() const
{
    return listmeta.metas.size();
}

void Playlist::play(const MusicMeta &meta)
{
    if (contains(meta)) {
        listmeta.playing = meta;
    } else {
        listmeta.playing = MusicMeta();
    }
}

int Playlist::sorttype() const
{
    return listmeta.sortType;
}

void Playlist::setActive(bool active)
{
    listmeta.active = active;
}

MusicMetaList Playlist::allmusic() const
{
    MusicMetaList mlist;
    for (auto id : listmeta.sortMetas) {
        mlist << listmeta.metas.value(id);
    }
    return mlist;
}

void Playlist::reset(const MusicMetaList &metas)
{
    listmeta.sortMetas.clear();
    listmeta.metas.clear();

    for (auto &meta : metas) {
        listmeta.sortMetas << meta.hash;
        listmeta.metas.insert(meta.hash, meta);
    }
}

void Playlist::load()
{
    QList<PlaylistMeta> list;
    QSqlQuery query;
    query.prepare(QString("SELECT music_id FROM playlist_%1").arg(listmeta.uuid));

    if (!query.exec()) {
        qWarning() << query.lastError();
        return;
    }

    QStringList musicIDs;
    while (query.next()) {
        auto musicID = query.value(0).toString();
        listmeta.sortMetas << musicID;
        musicIDs << QString("\"%1\"").arg(musicID);
    }
    auto sqlStr = QString("SELECT hash, localpath, title, artist, album, "
                          "filetype, track, offset, length, size, "
                          "timestamp, cuepath, invalid "
                          "FROM music WHERE hash IN (%1)").arg(musicIDs.join(","));
    if (!query.exec(sqlStr)) {
        qWarning() << query.lastError();
        return;
    }

    while (query.next()) {
        MusicMeta info;
        info.hash = query.value(0).toString();
        info.localPath = query.value(1).toString();
        info.title = query.value(2).toString();
        info.artist = query.value(3).toString();
        info.album = query.value(4).toString();
        info.filetype = query.value(5).toString();
        info.track = query.value(6).toInt();
        info.offset = query.value(7).toInt();
        info.length = query.value(8).toInt();
        info.size = query.value(9).toInt();
        info.timestamp = query.value(10).toInt();
        info.cuePath = query.value(11).toString();
        info.invalid = query.value(12).toBool();
        listmeta.metas.insert(info.hash, info);
        if (info.invalid) {
            listmeta.invalidMetas.insert(info.hash, 1);
        } else {
            listmeta.invalidMetas.remove(info.hash);
        }

    }
    resort();
}

void Playlist::setDisplayName(const QString &name)
{
    listmeta.displayName = name;

    QSqlQuery query;
    query.prepare("UPDATE playlist SET displayname = :displayname WHERE uuid= :uuid");
    query.bindValue(":displayname", listmeta.displayName);
    query.bindValue(":uuid", listmeta.uuid);

    if (!query.exec()) {
        qWarning() << query.lastError();
        return;
    }
}

void Playlist::appendMusic(const MusicMetaList &metalist)
{
    MusicMetaList newMetalist;
    for (auto &meta : metalist) {
        if (listmeta.metas.contains(meta.hash)) {
            qDebug() << "skip dump music " << meta.hash << meta.localPath;
            continue;
        }

        newMetalist << meta;
        listmeta.sortMetas << meta.hash;
        listmeta.metas.insert(meta.hash, meta);
    }

    emit MediaDatabase::instance()->insertMusicList(newMetalist, this->listmeta);
    emit musiclistAdded(newMetalist);
}

void Playlist::updateMeta(const MusicMeta &meta)
{
    if (!listmeta.metas.contains(meta.hash)) {
//        qWarning() << "no such id in playlist" << meta.hash << meta.localPath << listmeta.displayName;
        return;
    }

    listmeta.metas.insert(meta.hash, meta);
    // TODO : chcek and update;
    if (meta.invalid) {
        listmeta.invalidMetas.insert(meta.hash, 1);
    } else {
        listmeta.invalidMetas.remove(meta.hash);
    }
}

MusicMeta Playlist::removeMusic(const MusicMetaList &metalist)
{
    MusicMeta next;
    QSqlDatabase::database().transaction();
    for (auto &meta : metalist) {
        next = removeOneMusic(meta);
    }
    QSqlDatabase::database().commit();
    return next;
}

MusicMeta Playlist::removeOneMusic(const MusicMeta &meta)
{
    if (meta.hash.isEmpty()) {
        qCritical() << "Cannot remove empty id" << meta.hash << meta.title;
        return MusicMeta();
    }
    if (!listmeta.metas.contains(meta.hash)) {
//        qWarning() << "no such id in playlist" << meta.hash << meta.localPath << listmeta.displayName;
        return MusicMeta();
    }

    MusicMeta nextMeta;
    auto nextPos = listmeta.sortMetas.lastIndexOf(meta.hash) + 1;
    if (listmeta.sortMetas.length() > nextPos) {
        nextMeta = listmeta.metas.value(listmeta.sortMetas.value(nextPos));
    }
    int ret = listmeta.sortMetas.removeAll(meta.hash);

    Q_ASSERT(ret == 1);

    listmeta.metas.remove(meta.hash);

    emit musicRemoved(meta);

    MediaDatabase::deleteMusic(meta, listmeta);
    return nextMeta;
}

bool lessThanTimestamp(const MusicMeta &v1, const MusicMeta &v2)
{
    return v1.timestamp < v2.timestamp;
}

bool lessThanTitile(const MusicMeta &v1, const MusicMeta &v2)
{
    if (v1.title.isEmpty()) {
        return false;
    }
    if (v2.title.isEmpty()) {
        return true;
    }
    return collator.compare(v1.title , v2.title) < 0;
}

bool lessThanArtist(const MusicMeta &v1, const MusicMeta &v2)
{
    if (v1.artist.isEmpty()) {
        return false;
    }
    if (v2.artist.isEmpty()) {
        return true;
    }
    return collator.compare(v1.artist , v2.artist) < 0;
}

bool lessThanAblum(const MusicMeta &v1, const MusicMeta &v2)
{
    if (v1.album.isEmpty()) {
        return false;
    }
    if (v2.album.isEmpty()) {
        return true;
    }
    return collator.compare(v1.album , v2.album) < 0;
}

typedef bool (*LessThanFunctionPtr)(const MusicMeta &v1, const MusicMeta &v2);

LessThanFunctionPtr getSortFunction(Playlist::SortType sortType)
{
    switch (sortType) {
    case Playlist::SortByAddTime :
        return &lessThanTimestamp;
    case Playlist::SortByTitle :
        return &lessThanTitile;
    case Playlist::SortByArtist :
        return &lessThanArtist;
    case Playlist::SortByAblum :
        return &lessThanAblum;
    }
    return &lessThanTimestamp;
}

void Playlist::sortBy(Playlist::SortType sortType)
{
    if (listmeta.sortType == sortType) {
        return;
    }
    listmeta.sortType = sortType;
    MediaDatabase::updatePlaylist(listmeta);
    resort();
}

void Playlist::resort()
{
    collator.setNumericMode(true);
    collator.setCaseSensitivity(Qt::CaseInsensitive);

    QList<MusicMeta> sortList;

    for (auto id : listmeta.sortMetas) {
        sortList << listmeta.metas.value(id);
    }

    qSort(sortList.begin(), sortList.end(),
          getSortFunction(static_cast<Playlist::SortType>(listmeta.sortType)));

    listmeta.sortMetas.clear();
    for (auto meta : sortList) {
        listmeta.sortMetas << meta.hash;
    }
}
