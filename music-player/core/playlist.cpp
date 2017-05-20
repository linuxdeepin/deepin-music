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
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QCollator>

#include "medialibrary.h"
#include "mediadatabase.h"

static QCollator collator;

Playlist::Playlist(const PlaylistMeta &musiclistinfo, QObject *parent)
    : QObject(parent)
{
    playlistMeta = musiclistinfo;
}

const MetaPtr Playlist::first() const
{
    return playlistMeta.metas.value(playlistMeta.sortMetas.value(0));
}

const MetaPtr Playlist::prev(const MetaPtr meta) const
{
    if (0 == playlistMeta.sortMetas.length() || meta.isNull()) {
        return MetaPtr();
    }
    auto index = playlistMeta.sortMetas.indexOf(meta->hash);
    auto prev = (index + playlistMeta.sortMetas.length() - 1) % playlistMeta.sortMetas.length();
    return playlistMeta.metas.value(playlistMeta.sortMetas.at(prev));
}

const MetaPtr Playlist::next(const MetaPtr meta) const
{
    if (0 == playlistMeta.sortMetas.length() || meta.isNull()) {
        return MetaPtr();
    }
    auto index = playlistMeta.sortMetas.indexOf(meta->hash);
    auto prev = (index + 1) % playlistMeta.sortMetas.length();
    return playlistMeta.metas.value(playlistMeta.sortMetas.at(prev));
}

const MetaPtr Playlist::music(int index) const
{
    return playlistMeta.metas.value(playlistMeta.sortMetas.value(index));
}

const MetaPtr Playlist::music(const QString &id) const
{
    return playlistMeta.metas.value(id);
}

const MetaPtr Playlist::playing() const
{
    return playlistMeta.playing;
}

int Playlist::index(const QString &hash)
{
    // FIXME: improve performance
    return playlistMeta.sortMetas.indexOf(hash);
}

bool Playlist::isLast(const MetaPtr meta) const
{
    Q_ASSERT(!meta.isNull());
    return playlistMeta.sortMetas.last() == meta->hash;
}

bool Playlist::contains(const MetaPtr meta) const
{
    return !meta.isNull() && playlistMeta.metas.contains(meta->hash);
}

QString Playlist::id() const
{
    return playlistMeta.uuid;
}

QString Playlist::displayName() const
{
    return playlistMeta.displayName;
}

QString Playlist::icon() const
{
    return playlistMeta.icon;
}

bool Playlist::readonly() const
{
    return playlistMeta.readonly;
}

bool Playlist::editmode() const
{
    return playlistMeta.editmode;
}

bool Playlist::hide() const
{
    return playlistMeta.hide;
}

bool Playlist::isEmpty() const
{
    return playlistMeta.metas.isEmpty();
}

bool Playlist::canNext() const
{
    return playlistMeta.invalidMetas.size() < playlistMeta.metas.size()
           && playlistMeta.metas.size() > 0;
}

bool Playlist::active() const
{
    return  playlistMeta.active;
}

int Playlist::length() const
{
    return playlistMeta.metas.size();
}

void Playlist::play(const MetaPtr meta)
{
    if (contains(meta)) {
        playlistMeta.active = true;
        playlistMeta.playing = meta;
    } else {
        playlistMeta.active = false;
        playlistMeta.playing = MetaPtr();
    }
}

int Playlist::sortType() const
{
    return playlistMeta.sortType;
}

uint Playlist::sortID() const
{
    return playlistMeta.sortID;
}

void Playlist::setActive(bool active)
{
    playlistMeta.active = active;
}

MetaPtrList Playlist::allmusic() const
{
    MetaPtrList mlist;
    for (auto id : playlistMeta.sortMetas) {
        mlist << playlistMeta.metas.value(id);
    }
    return mlist;
}

void Playlist::reset(const MetaPtrList metalist)
{
    playlistMeta.sortMetas.clear();
    playlistMeta.metas.clear();
    playlistMeta.invalidMetas.clear();

    for (auto &meta : metalist) {
        playlistMeta.sortMetas << meta->hash;
        playlistMeta.metas.insert(meta->hash, meta);
    }
}

void Playlist::load()
{
    QMap<int, QString> sortHashs;

    QSqlQuery query;
    query.prepare(QString("SELECT music_id, sort_id FROM playlist_%1").arg(playlistMeta.uuid));
    if (!query.exec()) {
        qWarning() << query.lastError();
        return;
    }

    QStringList toAppendMusicHashs;
    while (query.next()) {
        auto musicID = query.value(0).toString();
        auto sortID = query.value(1).toInt();
        if (!sortHashs.contains(sortID)) {
            sortHashs.insert(sortID, musicID);
        } else {
            toAppendMusicHashs << musicID;
        }
    }

    // remove invalid meta
    auto sortIDs = sortHashs.keys();
    qSort(sortIDs.begin(), sortIDs.end());

    playlistMeta.sortMetas.clear();
    for (auto sortID : sortIDs) {
        playlistMeta.sortMetas << sortHashs.value(sortID);
    }
    playlistMeta.sortMetas << toAppendMusicHashs;


    QStringList toRemoveMusicHashs;
    for (auto hash : playlistMeta.sortMetas) {
        auto meta = MediaLibrary::instance()->meta(hash);
        if (meta.isNull()) {
            toRemoveMusicHashs << hash;
            continue;
        }
        playlistMeta.metas.insert(hash, meta);
        if (meta->invalid) {
            playlistMeta.invalidMetas.insert(hash, 1);
        } else {
            playlistMeta.invalidMetas.remove(hash);
        }
    }

    for (auto removeHash : toRemoveMusicHashs) {
        playlistMeta.sortMetas.removeOne(removeHash);
    }

    if (!toAppendMusicHashs.isEmpty() || !toRemoveMusicHashs.isEmpty()) {
        QMap<QString, int> hashIndexs;
        for (auto i = 0; i < playlistMeta.sortMetas.length(); ++i) {
            hashIndexs.insert(playlistMeta.sortMetas.value(i), i);
        }
        this->saveSort(hashIndexs);
    }
}

void Playlist::setDisplayName(const QString &name)
{
    playlistMeta.displayName = name;

    QSqlQuery query;
    query.prepare("UPDATE playlist SET displayname = :displayname WHERE uuid= :uuid");
    query.bindValue(":displayname", playlistMeta.displayName);
    query.bindValue(":uuid", playlistMeta.uuid);

    if (!query.exec()) {
        qWarning() << query.lastError();
        return;
    }
}

void Playlist::appendMusicList(const MetaPtrList metalist)
{
    MetaPtrList newMetalist;
    for (auto meta : metalist) {
        if (playlistMeta.metas.contains(meta->hash)) {
            qDebug() << "skip dump music " << meta->hash << meta->localPath;
            continue;
        }

        newMetalist << meta;
        playlistMeta.sortMetas << meta->hash;
        playlistMeta.metas.insert(meta->hash, meta);
    }

    emit MediaDatabase::instance()->insertMusicList(newMetalist, this->playlistMeta);
    emit musiclistAdded(newMetalist);
}

void Playlist::updateMeta(const MetaPtr meta)
{
    Q_ASSERT(!meta.isNull());
    if (!playlistMeta.metas.contains(meta->hash)) {
//        qWarning() << "no such id in playlist" << meta->hash << meta->localPath << listmeta->displayName;
        return;
    }

    playlistMeta.metas.insert(meta->hash, meta);
    // TODO : chcek and update;
    if (meta->invalid) {
        playlistMeta.invalidMetas.insert(meta->hash, 1);
    } else {
        playlistMeta.invalidMetas.remove(meta->hash);
    }
}

MetaPtr Playlist::removeMusicList(const MetaPtrList metalist)
{
    MetaPtr next;
    QSqlDatabase::database().transaction();
    for (auto &meta : metalist) {
        next = removeOneMusic(meta);
    }
    QSqlDatabase::database().commit();
    emit musiclistRemoved(metalist);
    return next;
}

MetaPtr Playlist::removeOneMusic(const MetaPtr meta)
{
    Q_ASSERT(!meta.isNull());
    if (meta->hash.isEmpty()) {
        qCritical() << "Cannot remove empty id" << meta->hash << meta->title;
        return MetaPtr();
    }
    if (!playlistMeta.metas.contains(meta->hash)) {
//        qWarning() << "no such id in playlist" << meta->hash << meta->localPath << listmeta->displayName;
        return MetaPtr();
    }

    if (!playlistMeta.playing.isNull() && playlistMeta.playing->hash == meta->hash) {
        playlistMeta.playing = MetaPtr();
    }

    MetaPtr nextMeta;
    auto nextPos = playlistMeta.sortMetas.lastIndexOf(meta->hash) + 1;
    if (playlistMeta.sortMetas.length() > nextPos) {
        nextMeta = playlistMeta.metas.value(playlistMeta.sortMetas.value(nextPos));
    }
    int ret = playlistMeta.sortMetas.removeAll(meta->hash);

    Q_ASSERT(ret == 1);

    playlistMeta.metas.remove(meta->hash);
    playlistMeta.invalidMetas.remove(meta->hash);

    MediaDatabase::deleteMusic(meta, playlistMeta);
    return nextMeta;
}

inline bool startWithHanzi(const QString &text)
{
    const QVector<uint> list = text.toUcs4();
    return !list.isEmpty() && list.first() >= 0x4e00 && list.first() <= 0x9fbf;
}

bool lessCompareByString(const QString &str1, const QString &str2)
{
    if (startWithHanzi(str1)) {
        if (!startWithHanzi(str2)) {
            return false;
        }
    } else if (startWithHanzi(str2)) {
        return true;
    }

    return collator.compare(str1, str2) < 0;
}

bool lessThanTimestamp(const MetaPtr v1, const MetaPtr v2)
{
    Q_ASSERT(!v1.isNull());
    Q_ASSERT(!v2.isNull());
    return v1->timestamp < v2->timestamp;
}

bool moreThanTimestamp(const MetaPtr v1, const MetaPtr v2)
{
    return !lessThanTimestamp(v1, v2);
}

bool lessThanTitile(const MetaPtr v1, const MetaPtr v2)
{
    Q_ASSERT(!v1.isNull());
    Q_ASSERT(!v2.isNull());
    if (v1->title.isEmpty()) {
        return false;
    }
    if (v2->title.isEmpty()) {
        return true;
    }
//    qDebug() << v1->title << v2->title << collator.compare(v1->title , v2->title);
    return lessCompareByString(v1->title , v2->title);
}

bool moreThanTitile(const MetaPtr v1, const MetaPtr v2)
{
    return !lessThanTitile(v1, v2);
}

bool lessThanArtist(const MetaPtr v1, const MetaPtr v2)
{
    Q_ASSERT(!v1.isNull());
    Q_ASSERT(!v2.isNull());
    if (v1->artist.isEmpty()) {
        return false;
    }
    if (v2->artist.isEmpty()) {
        return true;
    }
    return lessCompareByString(v1->artist , v2->artist);
}
bool moreThanArtist(const MetaPtr v1, const MetaPtr v2)
{
    return !lessThanArtist(v1, v2);
}

bool lessThanAblum(const MetaPtr v1, const MetaPtr v2)
{
    Q_ASSERT(!v1.isNull());
    Q_ASSERT(!v2.isNull());

    if (v1->album.isEmpty()) {
        return false;
    }
    if (v2->album.isEmpty()) {
        return true;
    }
    return lessCompareByString(v1->album , v2->album);
}
bool moreThanAblum(const MetaPtr v1, const MetaPtr v2)
{
    return !lessThanAblum(v1, v2);
}

typedef bool (*LessThanFunctionPtr)(const MetaPtr v1, const MetaPtr v2);

LessThanFunctionPtr getSortFunction(Playlist::SortType sortType, Playlist::OrderType orderType)
{
    switch (orderType) {
    case Playlist::Ascending:
        switch (sortType) {
        case Playlist::SortByAddTime :
            return &lessThanTimestamp;
        case Playlist::SortByTitle :
            return &lessThanTitile;
        case Playlist::SortByArtist :
            return &lessThanArtist;
        case Playlist::SortByAblum :
            return &lessThanAblum;
        case Playlist::SortByCustom:
            return &lessThanTimestamp;
        }
        break;
    case Playlist::Descending:
        switch (sortType) {
        case Playlist::SortByAddTime :
            return &moreThanTimestamp;
        case Playlist::SortByTitle :
            return &moreThanTitile;
        case Playlist::SortByArtist :
            return &moreThanArtist;
        case Playlist::SortByAblum :
            return &moreThanAblum;
        case Playlist::SortByCustom:
            return &moreThanTimestamp;
        }
        break;
    }
    qCritical() << "show not sort by invaild typr" << sortType << orderType;
    return &lessThanTitile;
}

void Playlist::sortBy(Playlist::SortType sortType)
{
    playlistMeta.sortType = sortType;

    if (playlistMeta.orderType == Playlist::Ascending) {
        playlistMeta.orderType = Playlist::Descending ;
    } else {
        playlistMeta.orderType = Playlist::Ascending;
    }

    MediaDatabase::updatePlaylist(playlistMeta);

    resort();
}

void Playlist::resort()
{
    collator.setNumericMode(true);
    collator.setCaseSensitivity(Qt::CaseInsensitive);

    auto sortType = static_cast<Playlist::SortType>(playlistMeta.sortType);
    auto orderType = static_cast<Playlist::OrderType>(playlistMeta.orderType);
    if (sortType != Playlist::SortByCustom) {
        QList<MetaPtr> sortList;

        for (auto id : playlistMeta.metas.keys()) {
//        qDebug() << playlistMeta.metas.value(id) << id;
            sortList << playlistMeta.metas.value(id);
        }

        qSort(sortList.begin(), sortList.end(),
              getSortFunction(sortType, orderType));

        QMap<QString, int> hashIndexs;
        for (auto i = 0; i < sortList.length(); ++i) {
            hashIndexs.insert(sortList.value(i)->hash, i);
        }

        saveSort(hashIndexs);
    }
}

void Playlist::saveSort(QMap<QString, int> hashIndexs)
{
    QSqlDatabase::database().transaction();

    QMap<int, QString> sortHashs;

    for (auto hash :  hashIndexs.keys()) {
        QSqlQuery query;
        query.prepare(QString("UPDATE playlist_%1 SET sort_id = :sort_id WHERE music_id = :music_id; ").arg(playlistMeta.uuid));
        query.bindValue(":sort_id", hashIndexs.value(hash));
        query.bindValue(":music_id", hash);
        if (! query.exec()) {
            qDebug() << query.lastError();
        }
        sortHashs.insert(hashIndexs.value(hash), hash);
    }

    QSqlDatabase::database().commit();

    playlistMeta.sortMetas.clear();
    for (auto i = 0; i < sortHashs.size(); ++i) {
        playlistMeta.sortMetas << sortHashs.value(i);
    }
}
