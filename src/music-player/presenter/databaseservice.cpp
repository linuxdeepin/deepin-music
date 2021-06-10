/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     ZouYa <zouya@uniontech.com>
 *
 * Maintainer: WangYu <wangyu@uniontech.com>
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

#include "databaseservice.h"
#include <algorithm>
#include <QDebug>
#include <QDir>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QThread>

#include "../util/global.h"
#include "mediameta.h"
#include "dboperate.h"
#include "medialibrary.h"
#include "commonservice.h"
static const QString DatabaseUUID = "0fcbd091-2356-161c-9026-f49779f9c71c40";

//int databaseVersionNew();

//int updateDatabaseVersionNew(int version);

//void megrateToVserionNew_0();

//void megrateToVserionNew_1();

typedef void (*MargeFunctionn)();


static  QString strcmpuuid = "";
static  QString strcmphash = "";
static  QString strcmpSingerName = "";
static  QString strcmpAlbumName = "";

bool compareUuid(const DataBaseService::PlaylistData &data)
{
    return data.uuid == strcmpuuid;
}

bool compareHash(const MediaMeta &data)
{
    return data.hash == strcmphash;
}

bool compareSingerName(const SingerInfo &data)
{
    return data.singerName == strcmpSingerName;
}

bool compareAlbumName(const AlbumInfo &data)
{
    return data.albumName == strcmpAlbumName;
}
//获取所有音乐，排序后按count取数据
QList<MediaMeta> DataBaseService::getMusicInfosBySortAndCount(int count)
{
    QList<MediaMeta> mediaMetas;

    DataBaseService::ListSortType type = static_cast<DataBaseService::ListSortType>
                                         (DataBaseService::getInstance()->getPlaylistSortType("all"));

    QString queryStringNew;
    switch (type) {
    case DataBaseService::SortByAddTimeASC: {
        queryStringNew = QString("SELECT hash, localpath, title, artist, album, "
                                 "filetype, track, offset, length, size, "
                                 "timestamp, invalid, search_id, cuepath, "
                                 "lyricPath, codec, py_title, py_artist, py_album "
                                 "FROM musicNew ORDER BY timestamp ASC LIMIT ") + QString::number(count);
        break;
    }
    case DataBaseService::SortByTitleASC: {
        queryStringNew = QString("SELECT hash, localpath, title, artist, album, "
                                 "filetype, track, offset, length, size, "
                                 "timestamp, invalid, search_id, cuepath, "
                                 "lyricPath, codec, py_title, py_artist, py_album "
                                 "FROM musicNew ORDER BY title ASC LIMIT ") + QString::number(count);
        break;
    }
    case DataBaseService::SortBySingerASC: {
        queryStringNew = QString("SELECT hash, localpath, title, artist, album, "
                                 "filetype, track, offset, length, size, "
                                 "timestamp, invalid, search_id, cuepath, "
                                 "lyricPath, codec, py_title, py_artist, py_album "
                                 "FROM musicNew ORDER BY artist ASC LIMIT ") + QString::number(count);
        break;
    }
    case DataBaseService::SortByAblumASC: {
        queryStringNew = QString("SELECT hash, localpath, title, artist, album, "
                                 "filetype, track, offset, length, size, "
                                 "timestamp, invalid, search_id, cuepath, "
                                 "lyricPath, codec, py_title, py_artist, py_album "
                                 "FROM musicNew ORDER BY album ASC LIMIT ") + QString::number(count);
        break;
    }
    case DataBaseService::SortByAddTimeDES: {
        queryStringNew = QString("SELECT hash, localpath, title, artist, album, "
                                 "filetype, track, offset, length, size, "
                                 "timestamp, invalid, search_id, cuepath, "
                                 "lyricPath, codec, py_title, py_artist, py_album "
                                 "FROM musicNew ORDER BY timestamp DESC LIMIT ") + QString::number(count);
        break;
    }
    case DataBaseService::SortByTitleDES: {
        queryStringNew = QString("SELECT hash, localpath, title, artist, album, "
                                 "filetype, track, offset, length, size, "
                                 "timestamp, invalid, search_id, cuepath, "
                                 "lyricPath, codec, py_title, py_artist, py_album "
                                 "FROM musicNew ORDER BY title DESC LIMIT ") + QString::number(count);
        break;
    }
    case DataBaseService::SortBySingerDES: {
        queryStringNew = QString("SELECT hash, localpath, title, artist, album, "
                                 "filetype, track, offset, length, size, "
                                 "timestamp, invalid, search_id, cuepath, "
                                 "lyricPath, codec, py_title, py_artist, py_album "
                                 "FROM musicNew ORDER BY artist DESC LIMIT ") + QString::number(count);
        break;
    }
    case DataBaseService::SortByAblumDES: {
        queryStringNew = QString("SELECT hash, localpath, title, artist, album, "
                                 "filetype, track, offset, length, size, "
                                 "timestamp, invalid, search_id, cuepath, "
                                 "lyricPath, codec, py_title, py_artist, py_album "
                                 "FROM musicNew ORDER BY album DESC LIMIT ") + QString::number(count);
        break;
    }
    default:
        break;
    }

    QSqlQuery queryNew(m_db);
    if (!queryNew.prepare(queryStringNew)) {
        qCritical() << queryNew.lastError();
        return mediaMetas;
    }
    if (! queryNew.exec()) {
        qCritical() << queryNew.lastError();
        return mediaMetas;
    }

    while (queryNew.next()) {
        MediaMeta meta;
        meta.hash = queryNew.value(0).toString();
        meta.localPath = queryNew.value(1).toString();
        meta.title = queryNew.value(2).toString();
        meta.singer = queryNew.value(3).toString();
        meta.album = queryNew.value(4).toString();
        meta.filetype = queryNew.value(5).toString();
        meta.track = queryNew.value(6).toLongLong();
        meta.offset = queryNew.value(7).toLongLong();
        meta.length = queryNew.value(8).toLongLong();
        meta.size = queryNew.value(9).toLongLong();
        meta.timestamp = queryNew.value(10).toLongLong();
        meta.invalid = queryNew.value(11).toBool();
        meta.searchID = queryNew.value(12).toString();
        meta.cuePath = queryNew.value(13).toString();
        meta.lyricPath = queryNew.value(14).toString();
        meta.codec = queryNew.value(15).toString();
        meta.pinyinTitle = queryNew.value(16).toString();
        meta.pinyinArtist = queryNew.value(17).toString();
        meta.pinyinAlbum = queryNew.value(18).toString();
        mediaMetas << meta;
    }

    return mediaMetas;
}

QList<MediaMeta> DataBaseService::allMusicInfos(bool refresh)
{
    if (!refresh) {
        return m_AllMediaMeta;
    } else {
        m_AllMediaMeta.clear();
        QString queryStringNew = QString("SELECT hash, localpath, title, artist, album, "
                                         "filetype, track, offset, length, size, "
                                         "timestamp, invalid, search_id, cuepath, "
                                         "lyricPath, codec, py_title, py_artist, py_album "
                                         "FROM musicNew");

        QSqlQuery queryNew(m_db);
        if (!queryNew.prepare(queryStringNew)) {
            qCritical() << queryNew.lastError();
            return m_AllMediaMeta;
        }
        if (! queryNew.exec()) {
            qCritical() << queryNew.lastError();
            return m_AllMediaMeta;
        }

        while (queryNew.next()) {
            MediaMeta meta;
            meta.hash = queryNew.value(0).toString();
            meta.localPath = queryNew.value(1).toString();
            meta.title = queryNew.value(2).toString();
            meta.singer = queryNew.value(3).toString();
            meta.album = queryNew.value(4).toString();
            meta.filetype = queryNew.value(5).toString();
            meta.track = queryNew.value(6).toLongLong();
            meta.offset = queryNew.value(7).toLongLong();
            meta.length = queryNew.value(8).toLongLong();
            meta.size = queryNew.value(9).toLongLong();
            meta.timestamp = queryNew.value(10).toLongLong();
            meta.invalid = queryNew.value(11).toBool();
            meta.searchID = queryNew.value(12).toString();
            meta.cuePath = queryNew.value(13).toString();
            meta.lyricPath = queryNew.value(14).toString();
            meta.codec = queryNew.value(15).toString();
            meta.pinyinTitle = queryNew.value(16).toString();
            meta.pinyinArtist = queryNew.value(17).toString();
            meta.pinyinAlbum = queryNew.value(18).toString();
            m_MediaMetaMap[meta.hash] = meta;
            //utf-8为默认的编码类型，每次更新数据时刷新非utf-8的编码，
            //需要更新编码，并使其生效，第一次启动时不更新编码，影响启动性能
            if (!m_firstInitAllMusic && meta.codec != "UTF-8") {
                meta.updateCodec(meta.codec.toUtf8());
            }
            m_AllMediaMeta << meta;
        }
        m_firstInitAllMusic = false;
        allAlbumInfos();
        return m_AllMediaMeta;
    }
}

int DataBaseService::allMusicInfosCount()
{
    int count = 0;
    if (m_AllMediaMeta.size() > 0) {
        count = m_AllMediaMeta.size();
    } else {
        QString queryString = QString("SELECT count(*) FROM musicNew");
        QSqlQuery queryNew(m_db);
        if (!queryNew.prepare(queryString)) {
            qCritical() << queryNew.lastError();
            return 0;
        }
        if (!queryNew.exec()) {
            qCritical() << queryNew.lastError();
            count = 0;
        }
        while (queryNew.next()) {
            count = queryNew.value(0).toInt();
        }
    }
    return count;
}

MediaMeta DataBaseService::getMusicInfoByHash(const QString &hash)
{
    MediaMeta medMeta;
    strcmphash = hash;
    QList<MediaMeta>::iterator itr = std::find_if(m_AllMediaMeta.begin(), m_AllMediaMeta.end(), compareHash);
    if (itr != m_AllMediaMeta.end()) {
        medMeta = *itr;
    }
    return medMeta;
}

QList<AlbumInfo> DataBaseService::allAlbumInfos()
{
    m_AllAlbumInfo.clear();
    for (MediaMeta &meta : m_AllMediaMeta) {
        strcmpAlbumName = meta.album;
        QList<AlbumInfo>::iterator itr = std::find_if(m_AllAlbumInfo.begin(), m_AllAlbumInfo.end(), compareAlbumName);
        if (itr != m_AllAlbumInfo.end()) {
            itr->musicinfos[meta.hash] = meta;
            if (meta.timestamp < itr->timestamp) {
                itr->timestamp = meta.timestamp;
            }
        } else {
            AlbumInfo albumNew;
            albumNew.pinyinAlbum = meta.pinyinAlbum;
            albumNew.albumName = meta.album;
            albumNew.singer = meta.singer;
            albumNew.musicinfos[meta.hash] = meta;
            albumNew.timestamp = meta.timestamp;
            m_AllAlbumInfo.append(albumNew);
        }
    }
    return m_AllAlbumInfo;
}

QList<SingerInfo> DataBaseService::allSingerInfos()
{
    m_AllSingerInfo.clear();
    for (MediaMeta &meta : m_AllMediaMeta) {
        strcmpSingerName = meta.singer;
        QList<SingerInfo>::iterator itr = std::find_if(m_AllSingerInfo.begin(), m_AllSingerInfo.end(), compareSingerName);
        if (itr != m_AllSingerInfo.end()) {
            itr->musicinfos[meta.hash] = meta;
            if (meta.timestamp < itr->timestamp) {
                itr->timestamp = meta.timestamp;
            }
        } else {
            SingerInfo singer;
            singer.pinyinSinger = meta.pinyinArtist;
            singer.singerName = meta.singer;
            singer.musicinfos[meta.hash] = meta;
            singer.timestamp = meta.timestamp;
            m_AllSingerInfo.append(singer);
        }
    }
    return m_AllSingerInfo;
}

QList<MediaMeta> DataBaseService::customizeMusicInfos(const QString &hash)
{
    qDebug() << "---DataBaseService::customizeMusicInfos hash = " << hash;
    if (m_AllMediaMeta.count() <= 0) {
        allMusicInfos();
    }

    QList<MediaMeta> medialist;
    QSqlQuery query;
    if (!query.prepare(QString("SELECT music_id FROM playlist_%1").arg(hash))) {
        qWarning() << query.lastError();
        return medialist;
    }
    if (!query.exec()) {
        qWarning() << query.lastError();
        return medialist;
    }

    while (query.next()) {
        for (MediaMeta &meta : m_AllMediaMeta) {
            if (meta.hash == query.value(0).toString()) {
                medialist << meta;
            }
        }
    }

    return medialist;
}

QList<DataBaseService::PlaylistData> DataBaseService::getCustomSongList()
{
    QList<DataBaseService::PlaylistData> customlist;
    QSqlQuery query;
    if (!query.prepare("SELECT uuid ,displayname FROM playlist WHERE readonly=0 order by sort_id ASC")) {
        qWarning() << query.lastError();
        return customlist;
    }

    if (!query.exec()) {
        qWarning() << query.lastError();
        return customlist;
    }

    while (query.next()) {
        DataBaseService::PlaylistData mt;
        mt.uuid = query.value(0).toString(); //uuid
        mt.displayName = query.value(1).toString(); //display name
        customlist << mt;
    }

    return customlist;
}

void DataBaseService::removeSelectedSongs(const QString &curpage, const QStringList &musichashlist, bool removeFromLocal)
{
    m_deleting = true;
    m_musichashlistToDel = musichashlist;
    emit sigRemoveSelectedSongs(curpage, musichashlist, removeFromLocal);
}


//QSqlDatabase DataBaseService::getDatabase()
//{
//    if (!m_db.open()) {
//        QString cachePath = Global::cacheDir() + "/mediameta.sqlite";
//        m_db = QSqlDatabase::addDatabase("QSQLITE");
//        m_db.setDatabaseName(cachePath);
//        qDebug() << "zy------Open database error:" << m_db.lastError();
//        if (!m_db.open()) {
//            qDebug() << "zy------Open database error:" << m_db.lastError();
//            return QSqlDatabase();
//        }
//    }
//    return m_db;
//}

void DataBaseService::importMedias(const QString &importHash, const QStringList &urllist)
{
// bool值判断用来解决重复导入问题，考虑多次导入，
// 在线程中已由信号槽队列处理，这个值多余了，注释处理
//    // 导入中直接返回，不做处理
//    if (m_importing) {
//        return;
//    }
    m_importHash = importHash;
    emit signalImportedPercent(0);
    emit signalImportMedias(importHash, urllist);
}

//bool DataBaseService::getImportStatus()
//{
//    return m_Importing;
//}

// 移到子线程中执行
//void DataBaseService::addMediaMeta(const MediaMeta &meta)
//{
//    QSqlQuery query(m_db);

//    query.prepare("INSERT INTO musicNew ("
//                  "hash, timestamp, title, artist, album, "
//                  "filetype, size, track, offset, favourite, localpath, length, "
//                  "py_title, py_title_short, py_artist, py_artist_short, "
//                  "py_album, py_album_short, lyricPath, codec, cuepath "
//                  ") "
//                  "VALUES ("
//                  ":hash, :timestamp, :title, :artist, :album, "
//                  ":filetype, :size, :track, :offset, :favourite, :localpath, :length, "
//                  ":py_title, :py_title_short, :py_artist, :py_artist_short, "
//                  ":py_album, :py_album_short, :lyricPath, :codec, :cuepath "
//                  ")");

//    query.bindValue(":hash", meta.hash);
//    query.bindValue(":timestamp", meta.timestamp);
//    query.bindValue(":title", meta.title);
//    query.bindValue(":artist", meta.singer);
//    query.bindValue(":album", meta.album);
//    query.bindValue(":filetype", meta.filetype);
//    query.bindValue(":size", meta.size);
//    query.bindValue(":track", meta.track);
//    query.bindValue(":offset", meta.offset);
//    query.bindValue(":favourite", meta.favourite);
//    query.bindValue(":localpath", meta.localPath);
//    query.bindValue(":length", meta.length);
//    query.bindValue(":py_title", meta.pinyinTitle);
//    query.bindValue(":py_title_short", meta.pinyinTitleShort);
//    query.bindValue(":py_artist", meta.pinyinArtist);
//    query.bindValue(":py_artist_short", meta.pinyinArtistShort);
//    query.bindValue(":py_album", meta.pinyinAlbum);
//    query.bindValue(":py_album_short", meta.pinyinAlbumShort);
//    query.bindValue(":lyricPath", meta.lyricPath);
//    query.bindValue(":codec", meta.codec);
//    query.bindValue(":cuepath", meta.cuePath);

//    if (! query.exec()) {
//        qCritical() << query.lastError();
//        return;
//    }
//    m_AllMediaMeta.append(meta);
//}

void DataBaseService::addPlaylist(const DataBaseService::PlaylistData &playlistMeta)
{
    QSqlQuery query;
    bool isPrepare = query.prepare("INSERT INTO playlist ("
                                   "uuid, displayname, icon, readonly, hide, "
                                   "sort_type, order_type, sort_id "
                                   ") "
                                   "VALUES ("
                                   ":uuid, :displayname, :icon, :readonly, :hide, "
                                   ":sort_type, :order_type, :sort_id "
                                   ")");
    query.bindValue(":uuid", playlistMeta.uuid);
    query.bindValue(":displayname", playlistMeta.displayName);
    query.bindValue(":icon", playlistMeta.icon);
    query.bindValue(":readonly", playlistMeta.readonly);
    query.bindValue(":hide", playlistMeta.hide);
    query.bindValue(":sort_type", playlistMeta.sortType);
    query.bindValue(":order_type", playlistMeta.orderType);
    query.bindValue(":sort_id", playlistMeta.sortID);

    if ((!isPrepare) || (! query.exec())) {
        qWarning() << query.lastError();
        return;
    }
    m_PlaylistMeta.append(playlistMeta);

    QString sqlstring = QString("CREATE TABLE IF NOT EXISTS playlist_%1 ("
                                "music_id TEXT primary key not null, "
                                "playlist_id TEXT, sort_id INTEGER"
                                ")").arg(playlistMeta.uuid);
    if (! query.exec(sqlstring)) {
        qWarning() << query.lastError();
        return;
    }
}
// 添加是否删除成功返回值
bool DataBaseService::deletePlaylist(const QString &hash)
{
    QSqlQuery query(m_db);
    QString sqlstring = QString("DROP TABLE IF EXISTS playlist_%1").arg(hash);
    if (! query.exec(sqlstring)) {
        qWarning() << query.lastError();
        return false;
    }

    sqlstring = QString("DELETE FROM playlist WHERE uuid = '%1'").arg(hash);
    if (! query.exec(sqlstring)) {
        qWarning() << query.lastError();
        return false;
    }
    for (int i = 0; i < m_PlaylistMeta.size(); i++) {
        if (m_PlaylistMeta.at(i).uuid == hash) {
            m_PlaylistMeta.removeAt(i);
            break;
        }
    }
    return true;
}

void DataBaseService::updatePlaylist(const QVector<DataBaseService::PlaylistData> &playlistDataList)
{
    QSqlQuery query;

    for (auto item : playlistDataList) {
        bool isPrepare = query.prepare("UPDATE playlist SET displayname = :displayname WHERE uuid = :uuid");

        query.bindValue(":uuid", item.uuid);
        query.bindValue(":displayname", item.displayName);

        if ((!isPrepare) || (! query.exec())) {
            qWarning() << query.lastError();
            return;
        }
    }
}

bool DataBaseService::favoriteMusic(const MediaMeta meta)
{
    bool ret = false;
    QSqlQuery query(m_db);

    if (favoriteExist(meta)) {
        QString sqlDelete = QString("delete from playlist_fav where music_id = '%1'").arg(meta.hash);

        if (!query.exec(sqlDelete))
            qCritical() << query.lastError() << sqlDelete;
    } else {
        QString sqlInsert = QString("insert into playlist_fav values('%1', 'fav', 0)").arg(meta.hash);

        if (!query.exec(sqlInsert)) {
            qCritical() << query.lastError() << sqlInsert;
        } else {
            ret = true;
        }
    }

    return ret;
}

bool DataBaseService::favoriteExist(const MediaMeta meta)
{
    bool ret = false;
    QSqlQuery query(m_db);

    QString sqlIsExists = QString("select music_id from playlist_fav where music_id = '%1'").arg(meta.hash);

    if (query.exec(sqlIsExists)) {
        if (query.next()) {
            ret = true;
        }
    } else {
        qCritical() << query.lastError() << sqlIsExists;
    }

    return ret;
}

bool DataBaseService::deleteMetaFromPlaylist(QString uuid, const QStringList &metaHash)
{
    QSqlQuery query(m_db);
    QString strsql;
    for (QString hash : metaHash) {
        QString sqlIsExists = QString("select music_id from playlist_%1 where music_id = '%2'").arg(uuid).arg(hash);
        if (query.exec(sqlIsExists)) {
            if (query.next()) {
                strsql = QString("DELETE FROM playlist_%1 WHERE music_id='%2'").arg(uuid).arg(hash);
                bool isPrepare = query.prepare(strsql);
                if ((!isPrepare) || (! query.exec())) {
                    qCritical() << query.lastError() << strsql;
                }
                if (uuid == "fav") {
                    emit signalFavSongRemove(hash);
                }
                emit signalRmvSong(uuid, hash);
            }
        } else {
            qCritical() << query.lastError() << sqlIsExists;
        }
    }
    return true;
}

void DataBaseService::slotGetMetaFromThread(MediaMeta meta)
{
    m_AllMediaMeta.append(meta);
}

void DataBaseService::slotImportFinished(int failCount, int successCount, int exsitCount)
{
    Q_UNUSED(failCount)
    if (successCount > 0 || exsitCount > 0) {
        emit signalImportFinished(m_importHash, successCount);
        // 拖拽到搜索结果提示导入到所有音乐
        if (m_importHash == "musicResult" || m_importHash == "albumResult" || m_importHash == "artistResult"
                || m_importHash == "play") {
            emit CommonService::getInstance()->signalShowPopupMessage(
                DataBaseService::getInstance()->getPlaylistNameByUUID("all"), successCount + exsitCount, successCount);
        } else {
            emit CommonService::getInstance()->signalShowPopupMessage(
                DataBaseService::getInstance()->getPlaylistNameByUUID(m_importHash), successCount + exsitCount, successCount);
        }
    } else {
        emit signalImportFailed();
    }
    //数据加载完后再加载图片
    emit signalCreatCoverImg(m_AllMediaMeta);
    //启动加载数据完成后直接播放第一首歌
    emit signalPlayFromFileMaganager();
}

void DataBaseService::slotRmvSongThread(const QString &listHash, const QString &musicHash, bool removeFromLocal)
{
    if (listHash == "all") {
        for (int i = 0; i < m_AllMediaMeta.size(); i++) {
            if (m_AllMediaMeta.at(i).hash == musicHash) {
                if (removeFromLocal) {
                    QFile info(m_AllMediaMeta.at(i).localPath);
                    if (info.exists()) {
                        info.remove();
                    }
                }
                m_AllMediaMeta.removeAt(i);
                break;
            }
        }
        emit signalRmvSong("all", musicHash);
    } else {
        emit signalRmvSong(listHash, musicHash);
    }
}

void DataBaseService::slotDelFinish()
{
    m_deleting = false;
    m_musichashlistToDel.clear();
    emit signalDelFinish();
}

QList<DataBaseService::PlaylistData> DataBaseService::allPlaylistMeta()
{
    if (m_PlaylistMeta.size() > 0) {
        return m_PlaylistMeta;
    } else {
        m_PlaylistMeta.clear();
        QSqlQuery query(m_db);
        bool isPrepare = query.prepare("SELECT uuid, displayname, icon, readonly, hide, "
                                       "sort_type, order_type, sort_id FROM playlist");

        if ((!isPrepare) || (! query.exec())) {
            qWarning() << query.lastError();
            return m_PlaylistMeta;
        }

        while (query.next()) {
            PlaylistData playlistMeta;
            playlistMeta.uuid = query.value(0).toString();
            playlistMeta.displayName = query.value(1).toString();
            playlistMeta.icon = query.value(2).toString();
            playlistMeta.readonly = query.value(3).toBool();
            playlistMeta.hide = query.value(4).toBool();
            playlistMeta.sortType = query.value(5).toInt();
            playlistMeta.orderType = query.value(6).toInt();
            playlistMeta.sortID = query.value(7).toUInt();
            m_PlaylistMeta << playlistMeta;
        }
        return m_PlaylistMeta;
    }
}

int DataBaseService::addMetaToPlaylist(QString uuid, const QList<MediaMeta> &metas)
{
    int insert_count = 0;

    for (MediaMeta meta : metas) {
        QSqlQuery query(m_db);
        QString sqlStr = QString("SELECT * FROM playlist_%1 WHERE music_id = :music_id").arg(uuid);
        bool isPrepare = query.prepare(sqlStr);
        query.bindValue(":music_id", meta.hash);

        if (isPrepare && query.exec()) {
            if (!query.next()) {
                sqlStr = QString("INSERT INTO playlist_%1 "
                                 "(music_id, playlist_id, sort_id) "
                                 "SELECT :music_id, :playlist_id, :sort_id ").arg(uuid);

                isPrepare = query.prepare(sqlStr);
                query.bindValue(":playlist_id", uuid);
                query.bindValue(":music_id", meta.hash);
                query.bindValue(":sort_id", 0);
                if (isPrepare && query.exec()) {
                    insert_count++;
                    if (uuid == "fav") {
                        emit signalFavSongAdd(meta.hash);
                    }
                } else {
                    qCritical() << query.lastError() << sqlStr;
                }
            }
        } else {
            qCritical() << query.lastError() << sqlStr;
        }
    }

    return insert_count;
}

void DataBaseService::updatePlaylistSortType(int type, QString uuid)
{
    QSqlQuery query(m_db);
    bool isPrepare = query.prepare("UPDATE playlist "
                                   "SET sort_type = :sort_type "
                                   "WHERE uuid = :uuid;");
    query.bindValue(":uuid", uuid);
    query.bindValue(":sort_type", type);

    if ((!isPrepare) || (! query.exec())) {
        qWarning() << query.lastError();
        return;
    }
}

void DataBaseService::updatePlaylistDisplayName(QString displayname, QString uuid)
{
    QSqlQuery query(m_db);
    bool isPrepare = query.prepare("UPDATE playlist "
                                   "SET displayname = :displayname "
                                   "WHERE uuid = :uuid;");
    query.bindValue(":uuid", uuid);
    query.bindValue(":displayname", displayname);

    if ((!isPrepare) || (! query.exec())) {
        qWarning() << query.lastError();
        return;
    }
    for (int i = 0; i < m_PlaylistMeta.size(); i++) {
        if (m_PlaylistMeta.at(i).uuid == uuid) {
            PlaylistData data = m_PlaylistMeta.at(i);
            data.displayName = displayname;
            m_PlaylistMeta.replace(i, data);
            break;
        }
    }
    emit signalPlaylistNameUpdate(uuid);
}

int DataBaseService::getPlaylistSortType(QString uuid)
{
    QSqlQuery query(m_db);
    bool isPrepare = query.prepare("SELECT sort_type FROM playlist where uuid = :uuid;");
    query.bindValue(":uuid", uuid);
    if ((!isPrepare) || (! query.exec())) {
        qWarning() << query.lastError();
        return -1;
    }

    while (query.next()) {
        auto sort_type =  query.value(0).toInt();
        return sort_type;
    }
    return 0;
}

int DataBaseService::getPlaylistSongCount(QString uuid)
{
    int count = 0;
    QString queryString = QString("SELECT count(*) FROM playlist_%1").arg(uuid);
    QSqlQuery queryNew(m_db);
    bool isPrepare = queryNew.prepare(queryString);
    if ((!isPrepare) || (! queryNew.exec())) {
        qCritical() << queryNew.lastError();
        count = 0;
    }
    while (queryNew.next()) {
        count = queryNew.value(0).toInt();
    }
    return count;
}

bool DataBaseService::isMediaMetaInSonglist(const QString &songlistHash, const QString &musicHash)
{
    QSqlQuery query(m_db);
    QString sqlIsExists = QString("select music_id from playlist_%1 where music_id = '%2'").arg(songlistHash).arg(musicHash);
    if (query.exec(sqlIsExists)) {
        if (query.next()) {
            return true;
        } else {
            return false;
        }
    } else {
        qCritical() << query.lastError() << sqlIsExists;
        return false;
    }
}

void DataBaseService::updateMetaCodec(const MediaMeta &meta)
{
    QSqlQuery query;
    bool isPrepare = query.prepare(QString("UPDATE musicNew set codec='%1' WHERE hash='%2'").arg(meta.codec).arg(meta.hash));
    if ((!isPrepare) || (! query.exec())) {
        qWarning() << query.lastError();
        return ;
    }
    strcmphash = meta.hash;
    //update mm
    QList<MediaMeta>::iterator itr = std::find_if(m_AllMediaMeta.begin(), m_AllMediaMeta.end(), compareHash);
    if (itr != m_AllMediaMeta.end()) {
        itr->codec = meta.codec;
        itr->updateCodec(itr->codec.toUtf8());
    }
}

void DataBaseService::setFirstSong(const QString &strurl)
{
    m_firstSonsg = strurl;
}

QString DataBaseService::getFirstSong()
{
    QString strurl = m_firstSonsg;
    return strurl;
}

bool DataBaseService::getDelStatus()
{
    return m_deleting;
}

QStringList DataBaseService::getDelMetaHashs()
{
    return m_musichashlistToDel;
}

void DataBaseService::setDelNeedSleep()
{
    m_worker.setNeedSleep();
}

//void DataBaseService::updatePlaylistOrderType(int type, QString uuid)
//{
//    QSqlQuery query(m_db);
//    query.prepare("UPDATE playlist "
//                  "SET order_type = :order_type "
//                  "WHERE uuid = :uuid;");
//    query.bindValue(":uuid", uuid);
//    query.bindValue(":order_type", type);

//    if (! query.exec()) {
//        qWarning() << query.lastError();
//        return;
//    }
//}

//int DataBaseService::getPlaylistOrderType(QString uuid)
//{
//    QSqlQuery query;
//    query.prepare("SELECT order_type FROM playlist where uuid = :uuid;");
//    query.bindValue(":uuid", uuid);
//    if (!query.exec()) {
//        qWarning() << query.lastError();
//        return -1;
//    }

//    while (query.next()) {
//        auto order_type =  query.value(0).toInt();
//        return order_type;
//    }
//    return 0;
//}l

QString DataBaseService::getPlaylistNameByUUID(const QString &uuid)
{
    if (m_PlaylistMeta.size() <= 0) {
        allPlaylistMeta();
    }

    strcmpuuid = uuid;
    QList<PlaylistData>::iterator itr = std::find_if(m_PlaylistMeta.begin(), m_PlaylistMeta.end(), compareUuid);
    if (itr != m_PlaylistMeta.end()) {
        return itr->displayName;
    }

    return QString();
}

uint DataBaseService::getPlaylistMaxSortid()
{
    uint max = 1;
    QSqlQuery query(m_db);
    bool isPrepare = query.prepare("SELECT uuid, displayname, icon, readonly, hide, "
                                   "sort_type, order_type, sort_id FROM playlist");

    if ((!isPrepare) || (! query.exec())) {
        qWarning() << query.lastError();
        return 0;
    }

    while (query.next()) {
        if (query.value(7).toUInt() > max) {
            max = query.value(7).toUInt();
        }
    }
    return (max + 1);
}

bool DataBaseService::createConnection()
{
    QDir cacheDir(Global::cacheDir());
    if (!cacheDir.exists()) {
        cacheDir.mkpath(".");
    }
    QString cachePath = Global::cacheDir() + "/mediameta.sqlite";
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(cachePath);

    if (!m_db.open()) {
        qDebug() << m_db.lastError()
                 << Global::cacheDir()
                 << cachePath;
        return false;
    }

    QSqlQuery query(m_db);
    bool isExec = query.exec("CREATE TABLE IF NOT EXISTS music (hash TEXT primary key not null, "
                             "timestamp INTEGER,"
                             "title VARCHAR(256), artist VARCHAR(256), "
                             "py_title VARCHAR(256), py_title_short VARCHAR(256), "
                             "py_artist VARCHAR(256), py_artist_short VARCHAR(256), "
                             "py_album VARCHAR(256), py_album_short VARCHAR(256), "
                             "album VARCHAR(256), filetype VARCHAR(32), "
                             "size INTEGER, track INTEGER, "
                             "offset INTEGER, favourite INTEGER(32), "
                             "localpath VARCHAR(4096), length INTEGER, "
                             "search_id VARCHAR(256), "
                             "invalid INTEGER(32), "
                             "cuepath VARCHAR(4096) )"
                            );

    //Smooth transition
    isExec &= query.exec("CREATE TABLE IF NOT EXISTS musicNew (hash TEXT primary key not null, "
                         "timestamp INTEGER,"
                         "title VARCHAR(256), artist VARCHAR(256), "
                         "py_title VARCHAR(256), py_title_short VARCHAR(256), "
                         "py_artist VARCHAR(256), py_artist_short VARCHAR(256), "
                         "py_album VARCHAR(256), py_album_short VARCHAR(256), "
                         "album VARCHAR(256), filetype VARCHAR(32), "
                         "size INTEGER, track INTEGER, "
                         "offset INTEGER, favourite INTEGER(32), "
                         "localpath VARCHAR(4096), length INTEGER, "
                         "search_id VARCHAR(256), "
                         "invalid INTEGER(32), "
                         "lyricPath VARCHAR(4096), "
                         "codec VARCHAR(35), "
                         "isCoverLoaded INTEGER(32), "
                         "cuepath VARCHAR(4096) )"
                        );

    // 判断musicNew中是否有isCoverLoaded字段
    isExec &= query.exec("select sql from sqlite_master where name = \"musicNew\" and sql like \"%hasimage%\"");
    if (!query.next()) {
        // 无isCoverLoaded字段,则增加isCoverLoaded字段,默认值1
        isExec &= query.exec(QString("ALTER TABLE \"musicNew\" ADD COLUMN \"hasimage\" INTEGER default \"%1\"")
                             .arg("1"));
    }



    isExec &= query.exec("CREATE TABLE IF NOT EXISTS ablum (id int primary key, "
                         "name VARCHAR(20), localpath VARCHAR(4096), url VARCHAR(4096))");

    isExec &= query.exec("CREATE TABLE IF NOT EXISTS artist (id int primary key, "
                         "name VARCHAR(20))");

    isExec &= query.exec("CREATE TABLE IF NOT EXISTS playlist (uuid TEXT primary key not null, "
                         "displayname VARCHAR(4096), "
                         "icon VARCHAR(256), readonly INTEGER, "
                         "hide INTEGER, sort_type INTEGER, "
                         "sort_id INTEGER, "
                         "order_type INTEGER )");

    isExec &= query.exec("CREATE TABLE IF NOT EXISTS info (uuid TEXT primary key not null, "
                         "version INTEGER )");
    initPlaylistTable();
    return isExec;
}

bool DataBaseService::isPlaylistExist(const QString &uuid)
{
    QSqlQuery query(m_db);
    bool isPrepare = query.prepare("SELECT COUNT(*) FROM playlist where uuid = :uuid");
    query.bindValue(":uuid", uuid);

    if ((!isPrepare) || (! query.exec())) {
        qWarning() << query.lastError();
        return false;
    }
    query.first();

    return query.value(0).toInt() > 0;
}

void DataBaseService::initPlaylistTable()
{
    QSqlDatabase::database().transaction();

    QVector<PlaylistData> playlistDataList;

    PlaylistData playlistMeta;
    playlistMeta.uuid = "album";
    playlistMeta.displayName = tr("Albums");
    playlistMeta.icon = "album";
    playlistMeta.readonly = true;
    playlistMeta.hide = false;
    playlistMeta.sortID = 1;
    playlistMeta.sortType = SortByAddTimeASC;
    playlistDataList << playlistMeta;

    playlistMeta.uuid = "artist";
    playlistMeta.displayName = tr("Artists");
    playlistMeta.icon = "artist";
    playlistMeta.readonly = true;
    playlistMeta.hide = false;
    playlistMeta.sortID = 2;
    playlistDataList << playlistMeta;

    playlistMeta.uuid = "all";
    playlistMeta.displayName = tr("All Music");
    playlistMeta.icon = "all";
    playlistMeta.readonly = true;
    playlistMeta.hide = false;
    playlistMeta.sortID = 3;
    playlistDataList << playlistMeta;

    playlistMeta.displayName = tr("My Favorites");
    playlistMeta.uuid = "fav";
    playlistMeta.icon = "fav";
    playlistMeta.readonly = true;
    playlistMeta.hide = false;
    playlistMeta.sortID = 4;
    playlistDataList << playlistMeta;

    playlistMeta.displayName = "Playlist";
    playlistMeta.uuid = "play";
    playlistMeta.icon = "play";
    playlistMeta.readonly = true;
    playlistMeta.hide = true;
    playlistMeta.sortID = 5;
    playlistDataList << playlistMeta;

    playlistMeta.displayName = "Search result";
    playlistMeta.uuid = "search";
    playlistMeta.icon = "search";
    playlistMeta.readonly = true;
    playlistMeta.hide = true;
    playlistMeta.sortID = 6;
    playlistDataList << playlistMeta;

    playlistMeta.displayName = "Music";
    playlistMeta.uuid = "musicCand";
    playlistMeta.icon = "musicCand";
    playlistMeta.readonly = true;
    playlistMeta.hide = true;
    playlistMeta.sortID = 7;
    playlistDataList << playlistMeta;

    playlistMeta.displayName = "Album";
    playlistMeta.uuid = "albumCand";
    playlistMeta.icon = "albumCand";
    playlistMeta.readonly = true;
    playlistMeta.hide = true;
    playlistMeta.sortID = 8;
    playlistDataList << playlistMeta;

    playlistMeta.displayName = "Artist";
    playlistMeta.uuid = "artistCand";
    playlistMeta.icon = "artistCand";
    playlistMeta.readonly = true;
    playlistMeta.hide = true;
    playlistMeta.sortID = 9;
    playlistDataList << playlistMeta;

    playlistMeta.displayName = "Music";
    playlistMeta.uuid = "musicResult";
    playlistMeta.icon = "musicResult";
    playlistMeta.readonly = true;
    playlistMeta.hide = true;
    playlistMeta.sortID = 10;
    playlistDataList << playlistMeta;

    playlistMeta.displayName = "Album";
    playlistMeta.uuid = "albumResult";
    playlistMeta.icon = "albumResult";
    playlistMeta.readonly = true;
    playlistMeta.hide = true;
    playlistMeta.sortID = 11;
    playlistDataList << playlistMeta;

    playlistMeta.displayName = "Artist";
    playlistMeta.uuid = "artistResult";
    playlistMeta.icon = "artistResult";
    playlistMeta.readonly = true;
    playlistMeta.hide = true;
    playlistMeta.sortID = 12;
    playlistDataList << playlistMeta;

    if (!isPlaylistExist("album")) {
        for (auto item : playlistDataList) {
            addPlaylist(item);
        }
    } else {
        QVector<PlaylistData> updatePlaylistDataList(4);
        qCopy(playlistDataList.begin(), playlistDataList.begin() + updatePlaylistDataList.size(), updatePlaylistDataList.begin());

        updatePlaylist(updatePlaylistDataList);
    }

    QSqlDatabase::database().commit();
}

DataBaseService::DataBaseService()
{
    m_PlaylistMeta.clear();
    m_AllMediaMeta.clear();
    createConnection();
    //margeDatabaseNew();

    qRegisterMetaType<QList<MediaMeta>>("QList<MediaMeta>");
    qRegisterMetaType<QVector<float>>("QVector<float>");

    m_workerThread = new QThread(this);
//    DBOperate *worker = new DBOperate(m_workerThread);
    m_worker.moveToThread(m_workerThread);
    // 发送信号给子线程导入数据
    connect(this, &DataBaseService::signalImportMedias, &m_worker, &DBOperate::slotImportMedias);
    // 发送给子线程删除单曲
    connect(this, SIGNAL(sigRemoveSelectedSongs(const QString &, const QStringList &, bool)),
            &m_worker, SLOT(slotRemoveSelectedSongs(const QString &, const QStringList &, bool)));
    // 发送给子线程加载图片
    connect(this, &DataBaseService::signalCreatCoverImg, &m_worker, &DBOperate::slotCreatCoverImg);

    // 单首歌曲完成解析
    connect(&m_worker, &DBOperate::sigImportMetaFromThread, this, &DataBaseService::slotGetMetaFromThread, Qt::QueuedConnection);
    // 完成加载
    connect(&m_worker, &DBOperate::sigImportFinished, this, &DataBaseService::slotImportFinished, Qt::QueuedConnection);
    // 单张图片完成封面解析
    connect(&m_worker, &DBOperate::sigCreatOneCoverImg, this, &DataBaseService::signalCoverUpdate, Qt::QueuedConnection);
    // 收藏中的歌曲被删除，动态显示
    connect(&m_worker, &DBOperate::signalFavSongRemove, this, &DataBaseService::signalFavSongRemove, Qt::QueuedConnection);
    // 发送删除歌曲通知消息，动态显示
    connect(&m_worker, &DBOperate::signalRmvSong, this, &DataBaseService::slotRmvSongThread, Qt::QueuedConnection);
    // 所有歌曲被清空
    connect(&m_worker, &DBOperate::signalAllMusicCleared, this, &DataBaseService::signalAllMusicCleared, Qt::QueuedConnection);
    // 已导入百分比
    connect(&m_worker, &DBOperate::signalImportedPercent, this, &DataBaseService::signalImportedPercent, Qt::QueuedConnection);
    connect(&m_worker, &DBOperate::signalMusicAddOne, this, &DataBaseService::signalMusicAddOne, Qt::QueuedConnection);
    connect(&m_worker, &DBOperate::signalFavSongAdd, this, &DataBaseService::signalFavSongAdd, Qt::QueuedConnection);
    // 删除结束
    connect(&m_worker, &DBOperate::signalDelFinish, this, &DataBaseService::slotDelFinish, Qt::QueuedConnection);

    m_workerThread->start();
}

DataBaseService::~DataBaseService()
{
    m_worker.stop();
    m_workerThread->quit();
    m_workerThread->wait();
}

//void margeDatabaseNew()
//{
//    QMap<int, MargeFunctionn> margeFuncs;
//    margeFuncs.insert(0, megrateToVserionNew_0);
//    margeFuncs.insert(1, megrateToVserionNew_1);

//    int currentVersion = databaseVersionNew();

//    QList<int> sortVer = margeFuncs.keys();
//    qSort(sortVer.begin(), sortVer.end());

//    for (auto ver : sortVer) {
//        if (ver > currentVersion) {
//            margeFuncs.value(ver)();
//        }
//    }
//}

//void megrateToVserionNew_1()
//{
//    // FIXME: remove old
//    QSqlDatabase::database().transaction();
//    QSqlQuery query;

//    query.prepare("ALTER TABLE playlist ADD COLUMN sort_id INTEGER(32);");
//    if (!query.exec()) {
//        qWarning() << "sql upgrade with error:" << query.lastError().type();
//    }

//    updateDatabaseVersionNew(1);
//    QSqlDatabase::database().commit();
//}

//void megrateToVserionNew_0()
//{
//    // FIXME: remove old
//    QSqlDatabase::database().transaction();
//    QSqlQuery query;
//    qWarning() << "sql upgrade with error:" << query.lastError().type();
//    query.prepare("ALTER TABLE music ADD COLUMN cuepath VARCHAR(4096);");
//    if (!query.exec()) {
//        qWarning() << "sql upgrade with error:" << query.lastError().type();
//    }

//    query.prepare("ALTER TABLE music ADD COLUMN invalid INTEGER(32);");
//    if (!query.exec()) {
//        qWarning() << "sql upgrade with error:" << query.lastError().type();
//    }

//    query.prepare("ALTER TABLE music ADD COLUMN search_id VARCHAR(256);");
//    if (!query.exec()) {
//        qWarning() << "sql upgrade with error:" << query.lastError().type();
//    }

//    query.prepare("ALTER TABLE playlist ADD COLUMN order_type INTEGER(32);");
//    if (!query.exec()) {
//        qWarning() << "sql upgrade with error:" << query.lastError().type();
//    }

//    query.prepare("ALTER TABLE playlist ADD COLUMN sort_type INTEGER(32);");
//    if (!query.exec()) {
//        qWarning() << "sql upgrade with error:" << query.lastError().type();
//    }

//    query.prepare("ALTER TABLE playlist ADD COLUMN sort_id INTEGER(32);");
//    if (!query.exec()) {
//        qWarning() << "sql upgrade with error:" << query.lastError().type();
//    }

//    QStringList list;
//    query.prepare("SELECT uuid FROM playlist;");
//    if (!query.exec()) {
//        qWarning() << "sql upgrade with error:" << query.lastError().type();
//    }
//    while (query.next()) {
//        list <<  query.value(0).toString();
//    }

//    for (auto uuid : list) {
//        auto sqlStr = QString("ALTER TABLE playlist_%1  ADD COLUMN sort_id INTEGER(32);").arg(uuid);
//        query.prepare(sqlStr);
//        if (!query.exec()) {
//            qWarning() << "sql upgrade playlist with error:" << query.lastError().type();
//        }
//    }

//    updateDatabaseVersionNew(0);
//    QSqlDatabase::database().commit();
//}

//int updateDatabaseVersionNew(int version)
//{
//    QSqlQuery query;

//    query.prepare("INSERT INTO info ("
//                  "uuid, version "
//                  ") "
//                  "VALUES ("
//                  ":uuid, :version "
//                  ")");
//    query.bindValue(":version", version);
//    query.bindValue(":uuid", DatabaseUUID);
//    query.exec();
//    qWarning() << query.lastError();

//    query.prepare("UPDATE info SET version = :version where uuid = :uuid; ");
//    query.bindValue(":version", version);
//    query.bindValue(":uuid", DatabaseUUID);

//    if (!query.exec()) {
//        qWarning() << query.lastError();
//        return -1;
//    }

//    return version;
//}

//int databaseVersionNew()
//{
//    QSqlQuery query;
//    query.prepare("SELECT version FROM info where uuid = :uuid;");
//    query.bindValue(":uuid", DatabaseUUID);
//    if (!query.exec()) {
//        qWarning() << query.lastError();
//        return -1;
//    }

//    while (query.next()) {
//        auto version =  query.value(0).toInt();
//        return version;
//    }
//    return -1;
//}
