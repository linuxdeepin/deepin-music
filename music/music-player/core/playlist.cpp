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

#include "playlist.h"

#include <QDebug>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QCollator>
#include <QTime>

#include "../core/metasearchservice.h"
#include "../core/musicsettings.h"
#include "util/pinyinsearch.h"

#include "medialibrary.h"
#include "mediadatabase.h"

static QCollator collator;

Playlist::Playlist(const PlaylistMeta &musiclistinfo, QObject *parent)
    : QObject(parent)
{
    playlistMeta = musiclistinfo;
    shuffleSeed = static_cast<int>(QTime::currentTime().msecsSinceStartOfDay());
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
    int index = playlistMeta.sortMetas.indexOf(meta->hash);
    int prev = (index + playlistMeta.sortMetas.length() - 1) % playlistMeta.sortMetas.length();
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

const MetaPtrList shuffle(MetaPtrList &&musiclist, int seed)
{
    auto size = musiclist.size();
    auto mid = size / 2;
    // Preprocess: reorder musiclist with seed
    // to make sure music in the first half and second half is different every playlist
    // and same in the same playlist: for shuffle uniformity
    for (auto i = 0; i < size; ++i)
        musiclist.swap(i, seed % size);

    for (auto i = 0; i < mid; ++i)
        musiclist.swap(i, std::rand() % mid);
    for (auto i = mid; i < size; ++i)
        musiclist.swap(i, mid + (std::rand() % (size - mid)));
    return musiclist;
}

const MetaPtr Playlist::shuffleNext(const MetaPtr meta)
{
    if (0 == playlistMeta.sortMetas.length() || meta.isNull()) {
        return MetaPtr();
    }
    if (shuffleList.isEmpty()) {
        shuffleList = shuffle(allmusic(), shuffleSeed);
    }
    if (meta == shuffleList.last())
        return shuffleTurn(true);
    auto offset = shuffleList.indexOf(meta) + 1;
    // just ignore removed music
    for (auto i = shuffleList.begin() + offset; i < shuffleList.end(); ++i)
        if (contains(*i))
            return *i;
    return shuffleTurn(true);
}

const MetaPtr Playlist::shufflePrev(const MetaPtr meta)
{
    if (0 == playlistMeta.sortMetas.length() || meta.isNull()) {
        return MetaPtr();
    }
    if (shuffleList.isEmpty()) {
        shuffleList = shuffle(allmusic(), shuffleSeed);
    }
    if (meta == shuffleList.first())
        return shuffleTurn(false);
    auto offset = shuffleList.length() - shuffleList.indexOf(meta);
    for (auto i = shuffleList.rbegin() + offset; i < shuffleList.rend(); ++i)
        if (contains(*i))
            return *i;
    return shuffleTurn(false);
}

const MetaPtr Playlist::shuffleTurn(bool forward)
{
    std::swap(shuffleList, shuffleHistory);
    switch (shuffleHistoryState) {
    case ShuffleHistoryState::Empty:
        shuffleList = shuffle(allmusic(), shuffleSeed);
        break;
    case ShuffleHistoryState::Prev:
        if (forward)
            shuffleList = shuffle(allmusic(), shuffleSeed);
        break;
    case ShuffleHistoryState::Next:
        if (!forward)
            shuffleList = shuffle(allmusic(), shuffleSeed);
        break;
    }
    shuffleHistoryState = forward ? ShuffleHistoryState::Prev : ShuffleHistoryState::Next;
    return forward ? shuffleList.first() : shuffleList.last();
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

bool Playlist::playingStatus() const
{
    return playlistMeta.playStatus;
}

void Playlist::setPlayingStatus(bool status)
{
    playlistMeta.playStatus = status;
}

int Playlist::index(const QString &hash)
{
    // FIXME: improve performance
    return playlistMeta.sortMetas.indexOf(hash);
}

bool Playlist::isLast(const MetaPtr meta) const
{
    Q_ASSERT(!meta.isNull());
    return playlistMeta.sortMetas.isEmpty() ? false : playlistMeta.sortMetas.last() == meta->hash;
}

bool Playlist::contains(const MetaPtr meta) const
{
    //    qDebug() << meta->hash << playlistMeta.metas;
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

PlayMusicTypePtrList Playlist::playMusicTypePtrList() const
{
    PlayMusicTypePtrList mlist;
    for (auto id : playMusicTypePtrListData.sortMetas) {
        mlist << playMusicTypePtrListData.metas.value(id);
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

void Playlist::changePlayMusicTypeOrderType()
{
    playMusicTypePtrListData.orderType = playMusicTypePtrListData.orderType == 0 ? 1 : 0;
}

void Playlist::setSearchStr(const QString &str)
{
    searchData = str;
}

QString Playlist::searchStr() const
{
    return searchData;
}

void Playlist::setViewMode(const int &mode)
{
    viewModeFlag = mode;
}

int Playlist::viewMode() const
{
    return viewModeFlag;
}

void Playlist::clearTypePtr()
{
    if (playMusicTypePtrListData.sortMetas.size() > 0) {
        playMusicTypePtrListData.sortMetas.clear();
        playMusicTypePtrListData.metas.clear();
    }
}

void Playlist::appendMusicTypePtrListData(PlayMusicTypePtr musicTypePtr)
{
    playMusicTypePtrListData.sortMetas.append(musicTypePtr->name);
    playMusicTypePtrListData.metas.insert(musicTypePtr->name, musicTypePtr);
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

    QSqlDatabase::database().transaction();
    QSqlQuery query;
    query.prepare("UPDATE playlist SET displayname = :displayname WHERE uuid= :uuid");
    query.bindValue(":displayname", playlistMeta.displayName);
    query.bindValue(":uuid", playlistMeta.uuid);

    if (!query.exec()) {
        qWarning() << query.lastError();
        return;
    }
    QSqlDatabase::database().commit();
}

void Playlist::appendMusicList(const MetaPtrList metalist)
{
    MetaPtrList newMetalist;
    for (auto meta : metalist) {
        // TODO: Get called multiple times, maybe because multi-thread. Should find out why.
        if (playlistMeta.metas.contains(meta->hash)) {
            qDebug() << "skip dump music " << meta->hash << meta->localPath;
            continue;
        }

        newMetalist << meta;
        playlistMeta.sortMetas << meta->hash;
        playlistMeta.metas.insert(meta->hash, meta);
        MusicSettings::setOption("base.play.showFlag", 1);
    }

    Q_EMIT MediaDatabase::instance()->insertMusicList(newMetalist, this->playlistMeta);
    Q_EMIT musiclistAdded(newMetalist);
}

void Playlist::updateMeta(const MetaPtr meta)
{
    Q_ASSERT(!meta.isNull());
    if (!playlistMeta.metas.contains(meta->hash)) {
        //        qWarning() << "no such id in playlist" << meta->hash << meta->localPath << listmeta->displayName;
        return;
    }

    playlistMeta.metas.insert(meta->hash, meta);
    // TODO : check and update;
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
    Q_EMIT musiclistRemoved(metalist);
    return next;
}

MetaPtr Playlist::removeOneMusic(const MetaPtr meta)
{
//    Q_ASSERT(!meta.isNull());
    if (meta.isNull())
        return MetaPtr();
    if (meta->hash.isEmpty()) {
        qCritical() << "Cannot remove empty id" << meta->hash << meta->title;
        return MetaPtr();
    }
    if (id() == AlbumMusicListID) {
        QString albumStr = meta->album;
        if (albumStr.isEmpty()) {
            albumStr = tr("Unknown album");
        }
        if (playMusicTypePtrListData.metas.contains(albumStr)) {
            playMusicTypePtrListData.metas[albumStr]->playlistMeta.sortMetas.removeOne(meta->hash);
            playMusicTypePtrListData.metas[albumStr]->playlistMeta.metas.remove(meta->hash);
            if (playMusicTypePtrListData.metas[albumStr]->playlistMeta.metas.isEmpty()) {
                playMusicTypePtrListData.sortMetas.removeOne(albumStr);
                playMusicTypePtrListData.metas.remove(albumStr);
            }
        }
    } else if (id() == ArtistMusicListID) {
        QString artistStr = meta->artist;
        if (artistStr.isEmpty()) {
            artistStr = tr("Unknown artist");
        }
        if (playMusicTypePtrListData.metas.contains(artistStr)) {
            playMusicTypePtrListData.metas[artistStr]->playlistMeta.sortMetas.removeOne(meta->hash);
            playMusicTypePtrListData.metas[artistStr]->playlistMeta.metas.remove(meta->hash);
            if (playMusicTypePtrListData.metas[artistStr]->playlistMeta.metas.isEmpty()) {
                playMusicTypePtrListData.sortMetas.removeOne(artistStr);
                playMusicTypePtrListData.metas.remove(artistStr);
            }
        }
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

//inline bool startWithHanzi(const QString &text)
//{
//    const QVector<uint> list = text.toUcs4();
//    return !list.isEmpty() && list.first() >= 0x4e00 && list.first() <= 0x9fbf;
//}

bool lessCompareByString(const QString &str1, const QString &str2)
{
    //    if (startWithHanzi(str1)) {
    //        if (!startWithHanzi(str2)) {
    //            return false;
    //        }
    //    } else if (startWithHanzi(str2)) {
    //        return true;
    //    }

    QString curStr1(str1), curStr2(str2);
    QStringList strList1 = DMusic::PinyinSearch::simpleChineseSplit(curStr1);
    QStringList strList2 = DMusic::PinyinSearch::simpleChineseSplit(curStr2);

    QString endStr1 = strList1.join(""), enStr2 = strList2.join("");
    return collator.compare(endStr1, enStr2) < 0;
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

bool lessThanTitle(const MetaPtr v1, const MetaPtr v2)
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
    return lessCompareByString(v1->title, v2->title);
}

bool moreThanTitle(const MetaPtr v1, const MetaPtr v2)
{
    return !lessThanTitle(v1, v2);
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
    return lessCompareByString(v1->artist, v2->artist);
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
    return lessCompareByString(v1->album, v2->album);
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
            return &lessThanTitle;
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
            return &moreThanTitle;
        case Playlist::SortByArtist :
            return &moreThanArtist;
        case Playlist::SortByAblum :
            return &moreThanAblum;
        case Playlist::SortByCustom:
            return &moreThanTimestamp;
        }
        break;
    }
    qCritical() << "show not sort by invalid typr" << sortType << orderType;
    return &lessThanTitle;
}

void Playlist::sortBy(Playlist::SortType sortType)
{
    if (playlistMeta.sortType == sortType) {
        if (playlistMeta.orderType == Playlist::Ascending) {
            playlistMeta.orderType = Playlist::Descending ;
        } else {
            playlistMeta.orderType = Playlist::Ascending;
        }
    } else {
        playlistMeta.orderType = Playlist::Ascending;
        playlistMeta.sortType = sortType;
    }

    MediaDatabase::updatePlaylist(playlistMeta);

    resort();
}

void Playlist::sortPlayMusicTypePtrListData(int sortType)
{
    QList<PlayMusicTypePtr> sortList;
    for (auto id : playMusicTypePtrListData.metas.keys()) {
        sortList << playMusicTypePtrListData.metas.value(id);
    }
    if (playMusicTypePtrListData.orderType == 0) {
        if (sortType == 0) {
            qSort(sortList.begin(), sortList.end(), [ = ](PlayMusicTypePtr p1, PlayMusicTypePtr p2) {
                return lessCompareByString(p1->name, p2->name);
            });
        } else {
            qSort(sortList.begin(), sortList.end(), [ = ](PlayMusicTypePtr p1, PlayMusicTypePtr p2) {
                return p1->timestamp < p2->timestamp;
            });
        }
    } else {
        if (sortType == 0) {
            qSort(sortList.begin(), sortList.end(), [ = ](PlayMusicTypePtr p1, PlayMusicTypePtr p2) {
                return !lessCompareByString(p1->name, p2->name);
            });
        } else {
            qSort(sortList.begin(), sortList.end(), [ = ](PlayMusicTypePtr p1, PlayMusicTypePtr p2) {
                return p1->timestamp > p2->timestamp;
            });
        }
    }

    playMusicTypePtrListData.sortMetas.clear();
    for (auto i = 0; i < sortList.size(); ++i) {
        playMusicTypePtrListData.sortMetas << sortList[i]->name;
    }
}

void Playlist::resort()
{
    if (playlistMeta.uuid != AlbumMusicListID || playlistMeta.uuid != ArtistMusicListID) {
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

void Playlist::metaListToPlayMusicTypePtrList(Playlist::SortType sortType, const MetaPtrList metalist)
{
    playMusicTypePtrListData.sortMetas.clear();
    playMusicTypePtrListData.metas.clear();
    if (sortType == SortByAblum) {
        for (auto meta : metalist) {
            QString albumStr = meta->album;
            if (albumStr.isEmpty()) {
                albumStr = tr("Unknown album");
            }
            QString artistStr = meta->artist;
            if (artistStr.isEmpty()) {
                artistStr = tr("Unknown artist");
            }
            if (playMusicTypePtrListData.metas.contains(albumStr)) {
                if (playMusicTypePtrListData.metas[albumStr]->playlistMeta.metas.contains(meta->hash)) {
                    qDebug() << "skip dump music " << meta->hash << meta->localPath;
                    continue;
                }
                if (!playMusicTypePtrListData.metas[albumStr]->extraName.contains(artistStr, Qt::CaseInsensitive))
                    playMusicTypePtrListData.metas[albumStr]->extraName += ("&" + artistStr);
                if (playMusicTypePtrListData.metas[albumStr]->icon.isNull())
                    playMusicTypePtrListData.metas[albumStr]->icon = MetaSearchService::coverData(meta);
                if (playMusicTypePtrListData.metas[albumStr]->timestamp < meta->timestamp)
                    playMusicTypePtrListData.metas[albumStr]->timestamp = meta->timestamp;
                playMusicTypePtrListData.metas[albumStr]->playlistMeta.sortMetas << meta->hash;
                playMusicTypePtrListData.metas[albumStr]->playlistMeta.metas.insert(meta->hash, meta);
            } else {
                PlayMusicTypePtr t_playMusicTypePtr(new PlayMusicType);
                t_playMusicTypePtr->name = albumStr;
                t_playMusicTypePtr->extraName = artistStr;
                t_playMusicTypePtr->icon = MetaSearchService::coverData(meta);
                t_playMusicTypePtr->timestamp = meta->timestamp;
                t_playMusicTypePtr->playlistMeta.sortMetas << meta->hash;
                t_playMusicTypePtr->playlistMeta.metas.insert(meta->hash, meta);
                playMusicTypePtrListData.sortMetas << albumStr;
                playMusicTypePtrListData.metas.insert(albumStr, t_playMusicTypePtr);
            }
        }
    } else if (sortType == SortByArtist) {
        for (auto meta : metalist) {
            QString artistStr = meta->artist;
            if (artistStr.isEmpty()) {
                artistStr = tr("Unknown artist");
            }
            if (playMusicTypePtrListData.metas.contains(artistStr)) {
                if (playMusicTypePtrListData.metas[artistStr]->playlistMeta.metas.contains(meta->hash)) {
                    qDebug() << "skip dump music " << meta->hash << meta->localPath;
                    continue;
                }
                if (playMusicTypePtrListData.metas[artistStr]->icon.isNull())
                    playMusicTypePtrListData.metas[artistStr]->icon = MetaSearchService::coverData(meta);
                if (playMusicTypePtrListData.metas[artistStr]->timestamp < meta->timestamp)
                    playMusicTypePtrListData.metas[artistStr]->timestamp = meta->timestamp;
                playMusicTypePtrListData.metas[artistStr]->playlistMeta.sortMetas << meta->hash;
                playMusicTypePtrListData.metas[artistStr]->playlistMeta.metas.insert(meta->hash, meta);
            } else {
                PlayMusicTypePtr t_playMusicTypePtr(new PlayMusicType);
                t_playMusicTypePtr->name = artistStr;
                t_playMusicTypePtr->icon = MetaSearchService::coverData(meta);
                t_playMusicTypePtr->timestamp = meta->timestamp;
                t_playMusicTypePtr->playlistMeta.sortMetas << meta->hash;
                t_playMusicTypePtr->playlistMeta.metas.insert(meta->hash, meta);
                playMusicTypePtrListData.sortMetas << artistStr;

                playMusicTypePtrListData.metas.insert(artistStr, t_playMusicTypePtr);
            }
        }
    }
    sortPlayMusicTypePtrListData(playMusicTypePtrListData.sortType);
}

void Playlist::playMusicTypeToMeta(QString name, QStringList sortMetas)
{
    //removeMusicList(allmusic());
    playlistMeta.sortMetas.clear();
    playlistMeta.metas.clear();
    MetaPtrList mlist;
    for (auto meta : playMusicTypePtrListData.metas) {
        if (name.isEmpty() || name == meta->name) {
            for (auto hashCode : meta->playlistMeta.sortMetas) {
                if (meta->playlistMeta.metas[hashCode] != nullptr) {
                    mlist << meta->playlistMeta.metas[hashCode];
                }
            }
        }
    }
    //    appendMusicList(mlist);
    MetaPtrList newMetalist;
    for (auto meta : mlist) {
        // TODO: Get called multiple times, maybe because multi-thread. Should find out why.
        if (playlistMeta.metas.contains(meta->hash)) {
            qDebug() << "skip dump music " << meta->hash << meta->localPath;
            continue;
        }

        newMetalist << meta;
        playlistMeta.sortMetas << meta->hash;
        playlistMeta.metas.insert(meta->hash, meta);
    }
    if (!name.isEmpty() && sortMetas.size() == playlistMeta.sortMetas.size()) {
        bool allFlag = true;
        for (auto curStr : playlistMeta.sortMetas) {
            if (!sortMetas.contains(curStr)) {
                allFlag = false;
                break;
            }
        }
        if (allFlag) {
            playlistMeta.sortMetas = sortMetas;
        }
    }

    Q_EMIT MediaDatabase::instance()->insertMusicList(newMetalist, this->playlistMeta);
}
