// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "datamanager.h"

#include <QFileInfo>
#include <QDirIterator>
#include <QDateTime>
#include <QStandardPaths>
#include <QThread>
#include <QUuid>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QTimer>
#include <QDebug>
#include <QCoreApplication>

#include "audioanalysis.h"
#include "utils.h"
#include "dboperate.h"
#include "musicsettings.h"

using namespace DMusic;

static  QString strcmpArtistName = "";
static  QString strcmpAlbumName = "";
static int FirstLoadCount = 15;

static bool compareArtistName(const ArtistInfo &data)
{
    return data.name == strcmpArtistName;
}

static bool compareAlbumName(const AlbumInfo &data)
{
    return data.name == strcmpAlbumName;
}

// 升序
static bool moreThanTimestampASC(MediaMeta v1, MediaMeta v2)
{
    return v1.timestamp < v2.timestamp;
}

static bool moreThanTitleASC(const MediaMeta &v1, const MediaMeta &v2)
{
    return v1.pinyinTitle.toLower() < v2.pinyinTitle.toLower();
}

static bool moreThanArtistASC(const MediaMeta &v1, const MediaMeta &v2)
{
    return v1.pinyinArtist.toLower() < v2.pinyinArtist.toLower();
}

static bool moreThanAblumASC(const MediaMeta &v1, const MediaMeta &v2)
{
    return v1.pinyinAlbum.toLower() < v2.pinyinAlbum.toLower();
}
// 降序
static bool moreThanTimestampDES(MediaMeta v1, MediaMeta v2)
{
    return v1.timestamp > v2.timestamp;
}

static bool moreThanTitleDES(const MediaMeta &v1, const MediaMeta &v2)
{
    return v1.pinyinTitle.toLower() > v2.pinyinTitle.toLower();
}

static bool moreThanArtistDES(const MediaMeta &v1, const MediaMeta &v2)
{
    return v1.pinyinArtist.toLower() > v2.pinyinArtist.toLower();
}

static bool moreThanAblumDES(const MediaMeta &v1, const MediaMeta &v2)
{
    return v1.pinyinAlbum.toLower() > v2.pinyinAlbum.toLower();
}

// 升序
static bool moreThanAlbumTimestampASC(AlbumInfo v1, AlbumInfo v2)
{
    return v1.timestamp < v2.timestamp;
}

static bool moreThanAlbumTitleASC(const AlbumInfo &v1, const AlbumInfo &v2)
{
    return v1.pinyin.toLower() < v2.pinyin.toLower();
}
// 降序
static bool moreThanAlbumTimestampDES(const AlbumInfo &v1, const AlbumInfo &v2)
{
    return v1.timestamp > v2.timestamp;
}

static bool moreThanAlbumTitleDES(const AlbumInfo &v1, const AlbumInfo &v2)
{
    return v1.pinyin.toLower() > v2.pinyin.toLower();
}

// 升序
static bool moreThanArtistTimestampASC(ArtistInfo v1, ArtistInfo v2)
{
    return v1.timestamp < v2.timestamp;
}

static bool moreThanArtistTitleASC(const ArtistInfo &v1, const ArtistInfo &v2)
{
    return v1.pinyin.toLower() < v2.pinyin.toLower();
}
// 降序
static bool moreThanArtistTimestampDES(ArtistInfo v1, ArtistInfo v2)
{
    return v1.timestamp > v2.timestamp;
}

static bool moreThanArtistTitleDES(const ArtistInfo &v1, const ArtistInfo &v2)
{
    return v1.pinyin.toLower() > v2.pinyin.toLower();
}

class DataManagerPrivate
{
public:
    DataManagerPrivate(QStringList supportedSuffixs, DataManager *parent)
        : m_parent(parent)
    {
        m_settings = new MusicSettings(m_parent);
        m_currentHash = m_settings->value("base.play.last_playlist").toString();
        if (m_currentHash.isEmpty()) m_currentHash = "all";
        m_dbOperate = new DBOperate(supportedSuffixs);
        m_workerThread = new QThread(m_parent);
        m_dbOperate->moveToThread(m_workerThread);
    }
    ~DataManagerPrivate()
    {
        m_workerThread->quit();
        if (m_dbOperate) {
            delete m_dbOperate;
            m_dbOperate = nullptr;
        }
    }
private:
    friend class DataManager;
    DataManager                      *m_parent;
    QThread                          *m_workerThread     = nullptr;
    DBOperate                        *m_dbOperate        = nullptr;
    MusicSettings                    *m_settings         = nullptr;
    QSqlDatabase                      m_database;
    QString                           m_currentHash;
    QList<DMusic::MediaMeta>          m_allMetas;
    QList<DMusic::AlbumInfo>          m_allAlbums;
    QList<DMusic::ArtistInfo>         m_allArtists;
    QList<DMusic::PlaylistInfo>       m_allPlaylist;
    QList<QString>                    m_searchMetas;
    QList<QString>                    m_searchArtists;
    QList<QString>                    m_searchAlbums;
};

DataManager::DataManager(QStringList supportedSuffixs, QObject *parent)
    : QObject(parent), m_data(new DataManagerPrivate(supportedSuffixs, this))
{
    initPlaylist();

    connect(this, &DataManager::signalImportMetas, m_data->m_dbOperate, &DBOperate::slotImportMetas, Qt::QueuedConnection);
    connect(m_data->m_dbOperate, &DBOperate::signalAddOneMeta, this, &DataManager::slotAddOneMeta, Qt::QueuedConnection);
    connect(m_data->m_dbOperate, &DBOperate::signalImportFinished, this, &DataManager::signalImportFinished, Qt::QueuedConnection);

    m_data->m_workerThread->start();
}

DataManager::~DataManager()
{
    m_data->m_workerThread->quit();
    saveDataToDB();
}

void DataManager::setCurrentPlayliHash(const QString &hash)
{
    m_data->m_currentHash = hash;
    m_data->m_settings->setValue("base.play.last_playlist", hash);
    emit signalCurrentPlaylistSChanged(hash);
}

QString DataManager::currentPlayliHash()
{
    return m_data->m_currentHash;
}

int DataManager::metaIndexFromHash(const QString &hash)
{
    int index = -1;
    if (!hash.isEmpty()) {
        for (int i = 0; i < m_data->m_allMetas.size(); ++i) {
            if (m_data->m_allMetas[i].hash == hash) {
                index = i;
                break;
            }
        }
    }

    return index;
}

int DataManager::playlistIndexFromHash(const QString &hash)
{
    int index = -1;
    if (!hash.isEmpty()) {
        for (int i = 0; i < m_data->m_allPlaylist.size(); ++i) {
            if (m_data->m_allPlaylist[i].uuid == hash) {
                index = i;
                break;
            }
        }
    }

    return index;
}

void DataManager::deleteMetaFromAllMetas(const QStringList &hashs)
{
    QStringList allHashs = hashs;
    for (int i = m_data->m_allMetas.size() - 1; i >= 0; --i) {
        if (allHashs.contains(m_data->m_allMetas[i].hash)) {
            allHashs.removeOne(m_data->m_allMetas[i].hash);
            m_data->m_allMetas.removeAt(i);
            if (allHashs.isEmpty()) break;
        }
    }
}

void DataManager::addMetaToAlbum(const MediaMeta &meta)
{
    strcmpAlbumName = meta.album;
    QList<AlbumInfo>::iterator itr = std::find_if(m_data->m_allAlbums.begin(), m_data->m_allAlbums.end(), compareAlbumName);
    if (itr != m_data->m_allAlbums.end()) {
        itr->musicinfos[meta.hash] = meta;
        if (meta.timestamp < itr->timestamp) {
            itr->timestamp = meta.timestamp;
        }
    } else {
        AlbumInfo albumNew;
        albumNew.pinyin = meta.pinyinAlbum;
        albumNew.name = meta.album;
        albumNew.artist = meta.artist;
        albumNew.musicinfos[meta.hash] = meta;
        albumNew.timestamp = meta.timestamp;
        m_data->m_allAlbums.append(albumNew);
    }
}

void DataManager::deleteMetaFromAlbum(const QString &metaHash, const QString &name)
{
    for (int i = 0; i < m_data->m_allAlbums.size(); ++i) {
        if (m_data->m_allAlbums[i].name == name) {
            m_data->m_allAlbums[i].musicinfos.remove(metaHash);
            if (m_data->m_allAlbums[i].musicinfos.isEmpty()) {
                m_data->m_allAlbums.removeAt(i);
            }
            break;
        }
    }
}

void DataManager::addMetaToArtist(const MediaMeta &meta)
{
    strcmpArtistName = meta.artist;
    QList<ArtistInfo>::iterator itr = std::find_if(m_data->m_allArtists.begin(), m_data->m_allArtists.end(), compareArtistName);
    if (itr != m_data->m_allArtists.end()) {
        itr->musicinfos[meta.hash] = meta;
        if (meta.timestamp < itr->timestamp) {
            itr->timestamp = meta.timestamp;
        }
    } else {
        ArtistInfo artist;
        artist.pinyin = meta.pinyinArtist;
        artist.name = meta.artist;
        artist.musicinfos[meta.hash] = meta;
        artist.timestamp = meta.timestamp;
        m_data->m_allArtists.append(artist);
    }
}

void DataManager::deleteMetaFromArtist(const QString &metaHash, const QString &name)
{
    for (int i = 0; i < m_data->m_allArtists.size(); ++i) {
        if (m_data->m_allArtists[i].name == name) {
            m_data->m_allArtists[i].musicinfos.remove(metaHash);
            if (m_data->m_allArtists[i].musicinfos.isEmpty()) {
                m_data->m_allArtists.removeAt(i);
            }
            break;
        }
    }
}

int DataManager::allMusicCountDB()
{
    int count = 0;
    QString queryString = QString("SELECT count(*) FROM musicNew");
    QSqlQuery queryNew(m_data->m_database);
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
    return count;
}

bool DataManager::loadCurrentMetasDB()
{
    int index = playlistIndexFromHash(m_data->m_currentHash);
    if (index < 0 || index >= m_data->m_allPlaylist.size()) return false;

    QSqlQuery query(m_data->m_database);
    // 加载歌单数据
    if (!query.prepare(QString("SELECT music_id FROM playlist_%1 order by sort_id ASC LIMIT %2")
                       .arg(m_data->m_currentHash).arg(FirstLoadCount))) {
        qWarning() << query.lastError();
        return false;
    }
    if (!query.exec()) {
        qWarning() << query.lastError();
        return false;
    }
    m_data->m_allPlaylist[index].sortMetas.clear();

    while (query.next()) {
        m_data->m_allPlaylist[index].sortMetas.append(query.value(0).toString());
//        m_data->m_allPlaylist[index].sortCustomMetas.append(query.value(0).toString());
    }

    //加载歌曲数据
    for (QString metaStr : m_data->m_allPlaylist[index].sortMetas) {
        QString queryString = QString("SELECT hash, localpath, title, artist, album, "
                                      "filetype, track, offset, length, size, "
                                      "timestamp, invalid, search_id, cuepath, "
                                      "lyricPath, codec, py_title, py_artist, py_album, hasimage, orititle, oriartist, orialbum "
                                      "FROM musicNew WHERE hash='%1'").arg(metaStr);

        if (!query.prepare(queryString)) {
            qCritical() << query.lastError();
            continue;
        }
        if (!query.exec()) {
            qCritical() << query.lastError();
            continue;
        }

        while (query.next()) {
            MediaMeta meta;
            meta.hash = query.value(0).toString();
            meta.localPath = query.value(1).toString();
            meta.title = query.value(2).toString();
            meta.artist = query.value(3).toString();
            meta.album = query.value(4).toString();
            meta.filetype = query.value(5).toString();
            meta.track = query.value(6).toInt();
            meta.offset = query.value(7).toLongLong();
            meta.length = query.value(8).toLongLong();
            meta.size = query.value(9).toLongLong();
            meta.timestamp = query.value(10).toLongLong();
            meta.invalid = query.value(11).toBool();
            meta.searchID = query.value(12).toString();
            meta.cuePath = query.value(13).toString();
            meta.lyricPath = query.value(14).toString();
            meta.codec = query.value(15).toString();
            meta.pinyinTitle = query.value(16).toString();
            meta.pinyinArtist = query.value(17).toString();
            meta.pinyinAlbum = query.value(18).toString();
            meta.hasimage = query.value(19).toBool();
            if (meta.hasimage) {
                meta.coverUrl = DmGlobal::cachePath() + "/images/" + meta.hash + ".jpg";
            } else {
                meta.coverUrl = DmGlobal::cachePath() + "/images/default_cover.png";
            }
            meta.originalTitle = query.value(20).toByteArray();
            meta.originalArtist = query.value(21).toByteArray();
            meta.originalAlbum = query.value(22).toByteArray();
            if (meta.album.isEmpty()) {
                meta.album = DmGlobal::unknownAlbumText();
            }
            if (meta.artist.isEmpty()) {
                meta.artist = DmGlobal::unknownArtistText();
            }

            m_data->m_allMetas.append(meta);
            addMetaToAlbum(meta);
            addMetaToArtist(meta);
        }
    }

    return true;
}

bool DataManager::loadMetasDB()
{
    QString queryString("SELECT hash, localpath, title, artist, album, "
                        "filetype, track, offset, length, size, "
                        "timestamp, invalid, search_id, cuepath, "
                        "lyricPath, codec, py_title, py_artist, py_album, hasimage, orititle, oriartist, orialbum "
                        "FROM musicNew");

    QSqlQuery queryNew(m_data->m_database);
    if (!queryNew.prepare(queryString)) {
        qCritical() << queryNew.lastError();
        return false;
    }
    if (! queryNew.exec()) {
        qCritical() << queryNew.lastError();
        return false;
    }
    m_data->m_allMetas.clear();
    m_data->m_allAlbums.clear();
    m_data->m_allArtists.clear();

    while (queryNew.next()) {
        MediaMeta meta;
        meta.hash = queryNew.value(0).toString();
        meta.localPath = queryNew.value(1).toString();
        meta.title = queryNew.value(2).toString();
        meta.artist = queryNew.value(3).toString();
        meta.album = queryNew.value(4).toString();
        meta.filetype = queryNew.value(5).toString();
        meta.track = queryNew.value(6).toInt();
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
        meta.hasimage = queryNew.value(19).toBool();
        if (meta.hasimage) {
            meta.coverUrl = DmGlobal::cachePath() + "/images/" + meta.hash + ".jpg";
        } else {
            meta.coverUrl = DmGlobal::cachePath() + "/images/default_cover.png";
        }
        meta.originalTitle = queryNew.value(20).toByteArray();
        meta.originalArtist = queryNew.value(21).toByteArray();
        meta.originalAlbum = queryNew.value(22).toByteArray();
        if (meta.album.isEmpty()) {
            meta.album = DmGlobal::unknownAlbumText();
        }
        if (meta.artist.isEmpty()) {
            meta.artist = DmGlobal::unknownArtistText();
        }

        m_data->m_allMetas.append(meta);
        addMetaToAlbum(meta);
        addMetaToArtist(meta);
    }

    return true;
}

bool DataManager::loadPlaylistDB()
{
    QSqlQuery query(m_data->m_database);
    bool isPrepare = query.prepare("SELECT uuid, displayname, icon, readonly, hide, "
                                   "sort_type, order_type, sort_id FROM playlist order by sort_id ASC");

    if ((!isPrepare) || (! query.exec())) {
        qWarning() << query.lastError();
        return false;
    }

    for (int i = m_data->m_allPlaylist.size() - 1; i >= 0; i--) {
        if (m_data->m_allPlaylist[i].saveFalg)
            m_data->m_allPlaylist.removeAt(i);
    }
    while (query.next()) {
        PlaylistInfo playlistMeta;
        playlistMeta.uuid = query.value(0).toString();
        playlistMeta.displayName = query.value(1).toString();
        playlistMeta.icon = query.value(2).toString();
        playlistMeta.readonly = query.value(3).toBool();
        playlistMeta.hide = query.value(4).toBool();
        playlistMeta.sortType = query.value(5).toInt();
        playlistMeta.orderType = query.value(6).toInt();
        playlistMeta.sortID = query.value(7).toUInt();
        m_data->m_allPlaylist << playlistMeta;
    }

    return true;
}

bool DataManager::loadPlaylistMetasDB()
{
    QSqlQuery query(m_data->m_database);
    // 加载数据
    for (int i = 0; i < m_data->m_allPlaylist.size(); ++i) {
        if (!m_data->m_allPlaylist[i].saveFalg) continue;
        if (!query.prepare(QString("SELECT music_id FROM playlist_%1 order by sort_id ASC")
                           .arg(m_data->m_allPlaylist[i].uuid))) {
            qWarning() << query.lastError();
            continue;
        }
        if (!query.exec()) {
            qWarning() << query.lastError();
            continue;
        }
        m_data->m_allPlaylist[i].sortMetas.clear();

        while (query.next()) {
            m_data->m_allPlaylist[i].sortMetas.append(query.value(0).toString());
        }
        if (m_data->m_allPlaylist[i].sortType == DmGlobal::SortByCustomASC)
            m_data->m_allPlaylist[i].sortCustomMetas = m_data->m_allPlaylist[i].sortMetas;
    }
    return true;
}

bool DataManager::isPlaylistExistDB(const QString &uuid)
{
    QSqlQuery query(m_data->m_database);
    bool isPrepare = query.prepare("SELECT COUNT(*) FROM playlist where uuid = :uuid");
    query.bindValue(":uuid", uuid);

    if ((!isPrepare) || (! query.exec())) {
        qWarning() << query.lastError();
        return false;
    }
    query.first();

    return query.value(0).toInt() > 0;
}

void DataManager::addPlaylistDB(const DMusic::PlaylistInfo &playlist)
{
    if (!playlist.saveFalg) return;
    QSqlQuery query;
    bool isPrepare = query.prepare("INSERT INTO playlist ("
                                   "uuid, displayname, icon, readonly, hide, "
                                   "sort_type, order_type, sort_id "
                                   ") "
                                   "VALUES ("
                                   ":uuid, :displayname, :icon, :readonly, :hide, "
                                   ":sort_type, :order_type, :sort_id "
                                   ")");
    query.bindValue(":uuid", playlist.uuid);
    query.bindValue(":displayname", playlist.displayName);
    query.bindValue(":icon", playlist.icon);
    query.bindValue(":readonly", playlist.readonly);
    query.bindValue(":hide", playlist.hide);
    query.bindValue(":sort_type", playlist.sortType);
    query.bindValue(":order_type", playlist.orderType);
    query.bindValue(":sort_id", playlist.sortID);

    if ((!isPrepare) || (! query.exec())) {
        qWarning() << query.lastError();
        return;
    }

    QString sqlstring = QString("CREATE TABLE IF NOT EXISTS playlist_%1 ("
                                "music_id TEXT primary key not null, "
                                "playlist_id TEXT, sort_id INTEGER"
                                ")").arg(playlist.uuid);
    if (! query.exec(sqlstring)) {
        qWarning() << query.lastError();
        return;
    }
}

bool DataManager::deletePlaylistDB(const QString &uuid)
{
    QSqlQuery query(m_data->m_database);
    QString sqlstring = QString("DROP TABLE IF EXISTS playlist_%1").arg(uuid);
    if (! query.exec(sqlstring)) {
        qWarning() << query.lastError();
        return false;
    }

    sqlstring = QString("DELETE FROM playlist WHERE uuid = '%1'").arg(uuid);
    if (! query.exec(sqlstring)) {
        qWarning() << query.lastError();
        return false;
    }
    return true;
}

bool DataManager::deleteAllPlaylistDB()
{
    QSqlQuery query(m_data->m_database);
    QString sqlstring = QString("SELECT uuid FROM playlist");
    if (! query.exec(sqlstring)) {
        qWarning() << query.lastError();
        return false;
    }
    QStringList allPlaylistIDs;
    while (query.next()) {
        allPlaylistIDs.append(query.value(0).toString());
    }
    for (QString id : allPlaylistIDs) {
        deletePlaylistDB(id);
    }

    return true;
}

int DataManager::addMetasToPlaylistDB(const QString &uuid, const QList<MediaMeta> &metas)
{
    int insert_count = 0;

    for (MediaMeta meta : metas) {
        int count = 0;
        if (uuid != "album" && uuid != "artist" && uuid != "all") {
            QString queryString = QString("SELECT MAX(sort_id) FROM playlist_%1").arg(uuid);
            QSqlQuery queryNew(m_data->m_database);
            bool isPrepare = queryNew.prepare(queryString);
            if ((!isPrepare) || (!queryNew.exec())) {
                qCritical() << queryNew.lastError();
                count = 0;
            }
            while (queryNew.next()) {
                count = queryNew.value(0).toInt();
                count++;
            }
        }

        QSqlQuery query(m_data->m_database);
        QString sqlStr = QString("SELECT * FROM playlist_%1 WHERE music_id = :music_id").arg(uuid);
        bool isPrepare = query.prepare(sqlStr);
        query.bindValue(":music_id", meta.hash);

        if (isPrepare && query.exec()) {
            if (!query.next()) {
                // 不存在则添加
                sqlStr = QString("INSERT INTO playlist_%1 "
                                 "(music_id, playlist_id, sort_id) "
                                 "SELECT :music_id, :playlist_id, :sort_id ").arg(uuid);

                isPrepare = query.prepare(sqlStr);
                query.bindValue(":playlist_id", uuid);
                query.bindValue(":music_id", meta.hash);
                query.bindValue(":sort_id", count);
                if (isPrepare && query.exec()) {
                    insert_count++;
                } else {
                    qCritical() << query.lastError() << sqlStr;
                }
            }
        } else {
            // 查询是否存在失败,导入失败计数器加1
            qCritical() << query.lastError() << sqlStr;
        }
    }

    return insert_count;
}

void DataManager::saveDataToDB()
{
    m_data->m_database.transaction();

    // 保存数据
    QSqlQuery query(m_data->m_database);
    QString sqlStr;

    // 删除歌曲
    sqlStr = QString("DELETE FROM musicNew");
    bool isPrepare = query.prepare(sqlStr);
    if ((!isPrepare) || (! query.exec())) {
        qCritical() << query.lastError() << sqlStr;
    }

    // 删除歌曲
    for (MediaMeta meta : m_data->m_allMetas) {
        bool isPrepare = query.prepare("INSERT INTO musicNew ("
                                       "hash, timestamp, title, artist, album, "
                                       "filetype, size, track, offset, hasimage, favourite, localpath, length, "
                                       "py_title, py_title_short, py_artist, py_artist_short, "
                                       "py_album, py_album_short, lyricPath, codec, cuepath, orititle, oriartist, orialbum "
                                       ") "
                                       "VALUES ("
                                       ":hash, :timestamp, :title, :artist, :album, "
                                       ":filetype, :size, :track, :offset, :hasimage, :favourite, :localpath, :length, "
                                       ":py_title, :py_title_short, :py_artist, :py_artist_short, "
                                       ":py_album, :py_album_short, :lyricPath, :codec, :cuepath, :orititle, :oriartist, :orialbum "
                                       ")");
        query.bindValue(":hash", meta.hash);
        query.bindValue(":timestamp", meta.timestamp);
        query.bindValue(":title", meta.title);
        query.bindValue(":artist", meta.artist);
        query.bindValue(":album", meta.album);
        query.bindValue(":filetype", meta.filetype);
        query.bindValue(":size", meta.size);
        query.bindValue(":track", meta.track);
        query.bindValue(":offset", meta.offset);
        query.bindValue(":hasimage", meta.hasimage);
        query.bindValue(":favourite", meta.favourite);
        query.bindValue(":localpath", meta.localPath);
        query.bindValue(":length", meta.length);
        query.bindValue(":py_title", meta.pinyinTitle);
        query.bindValue(":py_title_short", meta.pinyinTitleShort);
        query.bindValue(":py_artist", meta.pinyinArtist);
        query.bindValue(":py_artist_short", meta.pinyinArtistShort);
        query.bindValue(":py_album", meta.pinyinAlbum);
        query.bindValue(":py_album_short", meta.pinyinAlbumShort);
        query.bindValue(":lyricPath", meta.lyricPath);
        query.bindValue(":codec", meta.codec);
        query.bindValue(":cuepath", meta.cuePath);
        query.bindValue(":orititle", meta.originalTitle);
        query.bindValue(":oriartist", meta.originalArtist);
        query.bindValue(":orialbum", meta.originalAlbum);

        if ((!isPrepare) || (! query.exec())) {
            qCritical() << query.lastError();
        }
    }

    // 删除歌单
    deleteAllPlaylistDB();
    for (const PlaylistInfo &info : m_data->m_allPlaylist) {
        if (!info.saveFalg) continue;

        if (isPlaylistExistDB(info.uuid)) {
            deletePlaylistDB(info.uuid);
        }
        addPlaylistDB(info);

        addMetasToPlaylistDB(info.uuid, getPlaylistMetas(info.uuid));
    }

    m_data->m_database.commit();
}

MediaMeta DataManager::metaFromHash(const QString &hash)
{
    MediaMeta mata;
    int index = metaIndexFromHash(hash);
    if (index >= 0 && index < m_data->m_allMetas.size()) mata = m_data->m_allMetas[index];
    return mata;
}

PlaylistInfo DataManager::playlistFromHash(const QString &hash)
{
    PlaylistInfo playlist;
    int index = playlistIndexFromHash(hash);
    if (index >= 0 && index < m_data->m_allPlaylist.size()) playlist = m_data->m_allPlaylist[index];

    if (hash == "musicResult")
        playlist.sortMetas = m_data->m_searchMetas;
    return playlist;
}

QList<DMusic::MediaMeta> DataManager::getPlaylistMetas(const QString &hash, int count)
{
    QString curHash = !hash.isEmpty() ? hash : "all";
    QList<DMusic::MediaMeta> metas;
    int index = playlistIndexFromHash(curHash);
    if (index < 0 || index >= m_data->m_allPlaylist.size())  return metas;
    int favIndex = playlistIndexFromHash("fav");
    bool favExist = (favIndex >= 0 && favIndex < m_data->m_allPlaylist.size());

    if (hash == "all" && m_data->m_allPlaylist[index].sortMetas.isEmpty()) {
        for (const DMusic::MediaMeta &meta : m_data->m_allMetas) {
            DMusic::MediaMeta curMeta = meta;
            if (favExist && m_data->m_allPlaylist[favIndex].sortMetas.contains(curMeta.hash)) curMeta.favourite = true;
            if (!meta.hash.isEmpty())
                metas.append(curMeta);
            if (count >= 0 && count == metas.size()) break;
        }
    } else {
        QStringList metaHashs = (hash == "musicResult") ? m_data->m_searchMetas :
                                (m_data->m_allPlaylist[index].sortType == DmGlobal::SortByCustomASC && m_data->m_allPlaylist[index].sortCustomMetas.size() > 0 ? m_data->m_allPlaylist[index].sortCustomMetas
                                 : m_data->m_allPlaylist[index].sortMetas);
        for (QString metaHash : metaHashs) {
            DMusic::MediaMeta meta = metaFromHash(metaHash);
            if (favExist && m_data->m_allPlaylist[favIndex].sortMetas.contains(meta.hash)) meta.favourite = true;
            if (!meta.hash.isEmpty())
                metas.append(meta);
            if (count >= 0 && count == metas.size()) break;
        }
    }

    return metas;
}

QList<PlaylistInfo> DataManager::allPlaylistInfos()
{
    return m_data->m_allPlaylist;
}

QVariantList DataManager::allPlaylistVariantList()
{
    QVariantList allList;
    for (const PlaylistInfo &info : m_data->m_allPlaylist) {
        allList.append(Utils::playlistToVariantMap(info));
    }
    return allList;
}

QList<PlaylistInfo> DataManager::customPlaylistInfos()
{
    QList<PlaylistInfo> playlists;
    for (const PlaylistInfo &info : m_data->m_allPlaylist) {
        if (info.readonly) continue;
        playlists.append(info);
    }
    return playlists;
}

QVariantList DataManager::customPlaylistVariantList()
{
    QVariantList allList;
    for (const PlaylistInfo &info : customPlaylistInfos()) {
        allList.append(Utils::playlistToVariantMap(info));
    }
    return allList;
}

QList<AlbumInfo> DataManager::allAlbumInfos()
{
    m_data->m_allAlbums.clear();
    for (MediaMeta &meta : getPlaylistMetas()) {
        strcmpAlbumName = meta.album;
        QList<AlbumInfo>::iterator itr = std::find_if(m_data->m_allAlbums.begin(), m_data->m_allAlbums.end(), compareAlbumName);
        if (itr != m_data->m_allAlbums.end()) {
            itr->musicinfos[meta.hash] = meta;
            if (meta.timestamp < itr->timestamp) {
                itr->timestamp = meta.timestamp;
            }
        } else {
            AlbumInfo albumNew;
            albumNew.name = meta.album;
            albumNew.artist = meta.artist;
            albumNew.pinyin = meta.pinyinAlbum;
            albumNew.musicinfos[meta.hash] = meta;
            albumNew.timestamp = meta.timestamp;
            m_data->m_allAlbums.append(albumNew);
        }
    }

    // 排序
    int index = playlistIndexFromHash("album");
    if (index >= 0 && index < m_data->m_allPlaylist.size()) {
        PlaylistInfo &playlistMeta = m_data->m_allPlaylist[index];
        sortPlaylist(playlistMeta.sortType, playlistMeta.uuid, false);
    }

    return m_data->m_allAlbums;
}

QVariantList DataManager::allAlbumVariantList()
{
    QVariantList allList;
    for (AlbumInfo info : allAlbumInfos()) {
        allList.append(Utils::albumToVariantMap(info));
    }
    return allList;
}

QList<ArtistInfo> DataManager::allArtistInfos()
{
    m_data->m_allArtists.clear();
    for (MediaMeta &meta : getPlaylistMetas()) {
        strcmpArtistName = meta.artist;
        QList<ArtistInfo>::iterator itr = std::find_if(m_data->m_allArtists.begin(), m_data->m_allArtists.end(), compareArtistName);
        if (itr != m_data->m_allArtists.end()) {
            itr->musicinfos[meta.hash] = meta;
            if (meta.timestamp < itr->timestamp) {
                itr->timestamp = meta.timestamp;
            }
        } else {
            ArtistInfo artist;
            artist.name = meta.artist;
            artist.pinyin = meta.pinyinArtist;
            artist.musicinfos[meta.hash] = meta;
            artist.timestamp = meta.timestamp;
            m_data->m_allArtists.append(artist);
        }
    }
    // 排序
    int index = playlistIndexFromHash("artist");
    if (index >= 0 && index < m_data->m_allPlaylist.size()) {
        PlaylistInfo &playlistMeta = m_data->m_allPlaylist[index];
        sortPlaylist(playlistMeta.sortType, playlistMeta.uuid, false);
    }

    return m_data->m_allArtists;
}

QVariantList DataManager::allArtistVariantList()
{
    QVariantList allList;
    for (ArtistInfo info : allArtistInfos()) {
        allList.append(Utils::artistToVariantMap(info));
    }
    return allList;
}

void DataManager::importMetas(const QStringList &urls, const QString &playlistHash, const bool &playFalg)
{
    QSet<QString> metaHashs, playMetaHashs, allMetaHashs;
    for (MediaMeta &meta : m_data->m_allMetas) {
        allMetaHashs.insert(meta.hash);
    }
    if (!playlistHash.isEmpty()) {
        QString curPlaylistHash = playlistHash;
        // 专辑或者艺人添加到所有歌单
        if (playlistHash == "album" || playlistHash == "artist")
            curPlaylistHash = "all";
        for (PlaylistInfo &playlist : m_data->m_allPlaylist) {
            if (playlist.uuid == curPlaylistHash) {
                for (QString &hash : playlist.sortMetas) {
                    metaHashs.insert(hash);
                }
                break;
            }
        }
    }
    QString curPlaylistHash = playlistHash.isEmpty() ? "all" : playlistHash;
    bool importPlay = false;
    if (curPlaylistHash == m_data->m_currentHash && curPlaylistHash != "play") {
        for (PlaylistInfo &playlist : m_data->m_allPlaylist) {
            if (playlist.uuid == curPlaylistHash) {
                importPlay = true;
                for (QString &hash : playlist.sortMetas) {
                    playMetaHashs.insert(hash);
                }
                break;
            }
        }
    }
    emit signalImportMetas(urls, metaHashs, importPlay, playMetaHashs, allMetaHashs, playlistHash, playFalg);
}

void DataManager::addMetasToPlayList(const QList<QString> &metaHash,
                                     const QString &playlistHash, const bool &addToPlay)
{
    if (playlistHash.isEmpty() || metaHash.isEmpty()) return;

    int index = playlistIndexFromHash(playlistHash);
    if (index < 0 || index >= m_data->m_allPlaylist.size()) return;

    PlaylistInfo &curPlaylist = m_data->m_allPlaylist[index];
    PlaylistInfo &favPlaylist = m_data->m_allPlaylist[playlistIndexFromHash("fav")];

    QSet<QString> allPlaylistHashs;
    if (m_data->m_currentHash == playlistHash && playlistHash != "play") {
        PlaylistInfo &playPlaylist = m_data->m_allPlaylist[playlistIndexFromHash("play")];
        for (const QString &hash : metaHash) {
            if (!curPlaylist.sortMetas.contains(hash)) {
                curPlaylist.sortMetas.append(hash);
                QStringList playlistHashs;
                playlistHashs << playlistHash;
                allPlaylistHashs << playlistHash;
                // 自动添加到播放歌曲
                if (!playPlaylist.sortMetas.contains(hash)) {
                    playPlaylist.sortMetas.append(hash);
                    playlistHashs << "play";
                    allPlaylistHashs << "play";
                }
                MediaMeta meta = metaFromHash(hash);
                if (playlistHash == "fav" || favPlaylist.sortMetas.contains(meta.hash))
                    meta.favourite = true;
                emit signalAddOneMeta(playlistHashs, meta, addToPlay);
            }
        }
    } else {
        for (const QString &hash : metaHash) {
            if (!curPlaylist.sortMetas.contains(hash)) {
                curPlaylist.sortMetas.append(hash);
                QStringList playlistHashs;
                playlistHashs << playlistHash;
                allPlaylistHashs << playlistHash;
                MediaMeta meta = metaFromHash(hash);
                if (playlistHash == "fav" || favPlaylist.sortMetas.contains(meta.hash))
                    meta.favourite = true;
                emit signalAddOneMeta(playlistHashs, meta, addToPlay);
            }
        }
    }
    curPlaylist.sortCustomMetas = curPlaylist.sortMetas;
    emit signalAddMetaFinished(allPlaylistHashs.values());
}

void DataManager::addMetasToPlayList(const QList<MediaMeta> &metas, const QString &playlistHash, const bool &addToPlay)
{
    if (playlistHash.isEmpty() || metas.isEmpty()) return;

    int index = playlistIndexFromHash(playlistHash);
    if (index < 0 || index >= m_data->m_allPlaylist.size()) return;

    PlaylistInfo &curPlaylist = m_data->m_allPlaylist[index];
    PlaylistInfo &favPlaylist = m_data->m_allPlaylist[playlistIndexFromHash("fav")];

    QSet<QString> allPlaylistHashs;
    if (m_data->m_currentHash == playlistHash && playlistHash != "play") {
        PlaylistInfo &playPlaylist = m_data->m_allPlaylist[playlistIndexFromHash("play")];
        for (MediaMeta meta : metas) {
            if (!curPlaylist.sortMetas.contains(meta.hash)) {
                if (meta.filetype != "cdda")
                    curPlaylist.sortMetas.append(meta.hash);
                QStringList playlistHashs;
                playlistHashs << playlistHash;
                allPlaylistHashs << playlistHash;
                // 自动添加到播放歌曲
                if (!playPlaylist.sortMetas.contains(meta.hash)) {
                    playPlaylist.sortMetas.append(meta.hash);
                    playlistHashs << "play";
                    allPlaylistHashs << "play";
                }
                if (playlistHash == "fav" || favPlaylist.sortMetas.contains(meta.hash))
                    meta.favourite = true;
                emit signalAddOneMeta(playlistHashs, meta, addToPlay);
            }
        }
    } else {
        for (MediaMeta meta : metas) {
            if (!curPlaylist.sortMetas.contains(meta.hash)) {
                if (meta.filetype != "cdda")
                    curPlaylist.sortMetas.append(meta.hash);
                QStringList playlistHashs;
                playlistHashs << playlistHash;
                allPlaylistHashs << playlistHash;
                if (playlistHash == "fav" || favPlaylist.sortMetas.contains(meta.hash))
                    meta.favourite = true;
                emit signalAddOneMeta(playlistHashs, meta, addToPlay);
            }
        }
    }
    curPlaylist.sortCustomMetas = curPlaylist.sortMetas;
    emit signalAddMetaFinished(allPlaylistHashs.values());
}

void DataManager::clearPlayList(const QString &playlistHash, const bool &addToPlay)
{
    QString curHash = playlistHash.isEmpty() ? "play" : playlistHash;
    int index = playlistIndexFromHash(curHash);
    if (index < 0 || index >= m_data->m_allPlaylist.size()) return;

    PlaylistInfo &curPlaylist = m_data->m_allPlaylist[index];
    curPlaylist.sortMetas.clear();
    emit signalDeleteFinished(QStringList() << playlistHash);
}

void DataManager::removeFromPlayList(const QStringList listToDel, const QString &playlistHash,
                                     bool delFlag)
{
    QString curHash = playlistHash.isEmpty() ? "play" : playlistHash;

    if (playlistHash != "all" && playlistHash != "album" && playlistHash != "artist" && !delFlag) {
        int index = playlistIndexFromHash(curHash);
        if (index < 0 || index >= m_data->m_allPlaylist.size()) return;

        PlaylistInfo &curPlaylist = m_data->m_allPlaylist[index];
        for (const QString &hash : listToDel) {
            int curIndex = curPlaylist.sortMetas.indexOf(hash);
            if (curIndex >= 0 && curIndex < curPlaylist.sortMetas.size()) {
                curPlaylist.sortMetas.removeAt(curIndex);
                curPlaylist.sortCustomMetas.removeOne(hash);
                QStringList playlistHashs;
                playlistHashs << playlistHash;
                emit signalDeleteOneMeta(playlistHashs, hash, true);
            }
        }
    } else {
        for (const QString &hash : listToDel) {
            QStringList playlistHashs;
            for (PlaylistInfo &playlist : m_data->m_allPlaylist) {
                int curIndex = playlist.sortMetas.indexOf(hash);
                if (curIndex >= 0 && curIndex < playlist.sortMetas.size()) {
                    playlist.sortMetas.removeAt(curIndex);
                    if (curIndex >= 0 && curIndex < playlist.sortCustomMetas.size())
                        playlist.sortCustomMetas.removeAt(curIndex);
                    playlistHashs << playlist.uuid;
                }
            }
            if (!playlistHashs.isEmpty()) {
                DMusic::MediaMeta meta = metaFromHash(hash);
                if (delFlag) QFile::remove(meta.localPath);
                deleteMetaFromAllMetas(QStringList() << meta.hash);
                deleteMetaFromAlbum(meta.hash, meta.album);
                deleteMetaFromArtist(meta.hash, meta.artist);
                emit signalDeleteOneMeta(playlistHashs, hash, true);
            }
        }
    }
}

bool DataManager::moveMetasPlayList(const QStringList &metaHashs, const QString &playlistHash, const QString &nextHash)
{
    if (playlistHash == "all" || playlistHash == "album" || playlistHash == "artist")
        return false;
    if (nextHash.isEmpty() && metaHashs.contains(nextHash))
        return false;

    QString curHash = playlistHash.isEmpty() ? "play" : playlistHash;
    int index = playlistIndexFromHash(curHash);
    int customIndex = index;
    if (index < 0 || index >= m_data->m_allPlaylist.size()) return false;

    PlaylistInfo &curPlaylist = m_data->m_allPlaylist[index];
    if (curPlaylist.sortType != DmGlobal::SortByCustomASC
            && curPlaylist.sortType != DmGlobal::SortByCustomDES)
        return false;

    QStringList curMetas;
    index = -1;
    for (int i = curPlaylist.sortMetas.size() - 1; i >= 0; i--) {
        if (metaHashs.contains(curPlaylist.sortMetas[i])) {
            curMetas.insert(0, curPlaylist.sortMetas[i]);
            curPlaylist.sortMetas.removeAt(i);
        }
    }
    if (curMetas.isEmpty())
        return false;

    index = curPlaylist.sortMetas.size() - 1;
    if (!nextHash.isEmpty()) {
        for (int i = curPlaylist.sortMetas.size() - 1; i >= 0; i--) {
            if (curPlaylist.sortMetas[i]  == nextHash) {
                index = i;
                break;
            }
        }
    }
    if (index == curPlaylist.sortMetas.size() - 1) {
        curPlaylist.sortMetas += curMetas;
    } else {
        for (int i = curMetas.size() - 1; i >= 0; i--) {
            curPlaylist.sortMetas.insert(index, curMetas[i]);
        }
    }

    m_data->m_allPlaylist[customIndex].sortCustomMetas.clear();
    m_data->m_allPlaylist[customIndex].sortCustomMetas = curPlaylist.sortMetas;
    return true;
}

bool DataManager::isExistMeta()
{
    return !m_data->m_allMetas.isEmpty();
}

PlaylistInfo DataManager::addPlayList(const QString &name)
{
    QStringList existNames;
    for (PlaylistInfo &playlist : m_data->m_allPlaylist) {
        existNames.append(playlist.displayName);
    }
    QString curName = name;
    if (existNames.contains(curName)) {
        for (int i = 1; i < existNames.size() + 1; ++i) {
            curName = QString("%1 %2").arg(name).arg(i);
            if (!existNames.contains(curName)) {
                break;
            }
        }
    }

    PlaylistInfo playlistMeta;
    playlistMeta.uuid = QUuid::createUuid().toString().remove("{").remove("}").remove("-");
    playlistMeta.displayName = curName;
    playlistMeta.sortID = (uint)m_data->m_allPlaylist.size() + 1;
    playlistMeta.sortType = DmGlobal::SortByCustomASC;
    m_data->m_allPlaylist << playlistMeta;

    return playlistMeta;
}

void DataManager::sortPlaylist(const int &type, const QString &hash, bool signalFlag)
{
    int index = playlistIndexFromHash(hash);
    if (index < 0 || index >= m_data->m_allPlaylist.size()) return;
    PlaylistInfo &playlistMeta = m_data->m_allPlaylist[index];

    int sortType = DmGlobal::SortByAddTimeASC;
    if (signalFlag) {
        sortType = DmGlobal::SortByAddTimeASC;
        // 倒序
        switch (type) {
        case DmGlobal::SortByAddTime: {
            if (playlistMeta.sortType == DmGlobal::SortByAddTimeASC) {
                sortType = DmGlobal::SortByAddTimeDES;
            } else {
                sortType = DmGlobal::SortByAddTimeASC;
            }
            break;
        }
        case DmGlobal::SortByTitle: {
            if (playlistMeta.sortType == DmGlobal::SortByTitleASC) {
                sortType = DmGlobal::SortByTitleDES;
            } else {
                sortType = DmGlobal::SortByTitleASC;
            }
            break;
        }
        case DmGlobal::SortByArtist: {
            if (playlistMeta.sortType == DmGlobal::SortByArtistASC) {
                sortType = DmGlobal::SortByArtistDES;
            } else {
                sortType = DmGlobal::SortByArtistASC;
            }
            break;
        }
        case DmGlobal::SortByAblum: {
            if (playlistMeta.sortType == DmGlobal::SortByAblumASC) {
                sortType = DmGlobal::SortByAblumDES;
            } else {
                sortType = DmGlobal::SortByAblumASC;
            }
            break;
        }
        case DmGlobal::SortByCustom: {
            sortType = DmGlobal::SortByCustomASC;
            playlistMeta.sortType = sortType;
            emit signalPlaylistSortChanged(playlistMeta.uuid);
            break;
        }
        default:
            sortType = DmGlobal::SortByAddTimeASC;
            break;
        }
    } else {
        sortType = type;
    }


    playlistMeta.sortType = sortType;

    if (playlistMeta.uuid != "album" && playlistMeta.uuid != "artist"
            && playlistMeta.uuid != "albumResult" && playlistMeta.uuid != "artistResult") {
        QList<DMusic::MediaMeta> allMetas;
        QStringList sortMetas = playlistMeta.uuid == "musicResult" ? m_data->m_searchMetas : playlistMeta.sortMetas;
        for (QString hash : sortMetas) {
            allMetas.append(metaFromHash(hash));
        }
        bool sortFlag = true;
        switch (sortType) {
        case DmGlobal::SortByAddTimeASC: {
            std::sort(allMetas.begin(), allMetas.end(), moreThanTimestampASC);
        }
        break;
        case DmGlobal::SortByTitleASC: {
            std::sort(allMetas.begin(), allMetas.end(), moreThanTitleASC);
        }
        break;
        case DmGlobal::SortByArtistASC: {
            std::sort(allMetas.begin(), allMetas.end(), moreThanArtistASC);
        }
        break;
        case DmGlobal::SortByAblumASC: {
            std::sort(allMetas.begin(), allMetas.end(), moreThanAblumASC);
        }
        break;
        case DmGlobal::SortByAddTimeDES: {
            std::sort(allMetas.begin(), allMetas.end(), moreThanTimestampDES);
        }
        break;
        case DmGlobal::SortByTitleDES: {
            std::sort(allMetas.begin(), allMetas.end(), moreThanTitleDES);
        }
        break;
        case DmGlobal::SortByArtistDES: {
            std::sort(allMetas.begin(), allMetas.end(), moreThanArtistDES);
        }
        break;
        case DmGlobal::SortByAblumDES: {
            std::sort(allMetas.begin(), allMetas.end(), moreThanAblumDES);
        }
        break;
        default:
            sortFlag = false;
            break;
        }

        // 重新排序
        if (sortFlag) {
            if (playlistMeta.uuid == "musicResult") {
                m_data->m_searchMetas.clear();
            } else {
                playlistMeta.sortMetas.clear();
            }
            for (DMusic::MediaMeta meta : allMetas) {
                if (playlistMeta.uuid == "musicResult") {
                    m_data->m_searchMetas.append(meta.hash);
                } else {
                    playlistMeta.sortMetas.append(meta.hash);
                }
            }
            if (signalFlag)
                emit signalPlaylistSortChanged(playlistMeta.uuid);
        }
    } else {
        if (playlistMeta.uuid == "album") {
            bool sortFlag = true;
            switch (sortType) {
            case DmGlobal::SortByAddTimeASC: {
                std::sort(m_data->m_allAlbums.begin(), m_data->m_allAlbums.end(), moreThanAlbumTimestampASC);
            }
            break;
            case DmGlobal::SortByAblumASC: {
                std::sort(m_data->m_allAlbums.begin(), m_data->m_allAlbums.end(), moreThanAlbumTitleASC);
            }
            break;
            case DmGlobal::SortByAddTimeDES: {
                std::sort(m_data->m_allAlbums.begin(), m_data->m_allAlbums.end(), moreThanAlbumTimestampDES);
            }
            break;
            case DmGlobal::SortByAblumDES: {
                std::sort(m_data->m_allAlbums.begin(), m_data->m_allAlbums.end(), moreThanAlbumTitleDES);
            }
            break;
            default:
                sortFlag = false;
                break;
            }
            if (sortFlag && signalFlag) emit signalPlaylistSortChanged(playlistMeta.uuid);
        } else if (playlistMeta.uuid == "artist") {
            bool sortFlag = true;
            switch (sortType) {
            case DmGlobal::SortByAddTimeASC: {
                std::sort(m_data->m_allArtists.begin(), m_data->m_allArtists.end(), moreThanArtistTimestampASC);
            }
            break;
            case DmGlobal::SortByArtistASC: {
                std::sort(m_data->m_allArtists.begin(), m_data->m_allArtists.end(), moreThanArtistTitleASC);
            }
            break;
            case DmGlobal::SortByAddTimeDES: {
                std::sort(m_data->m_allArtists.begin(), m_data->m_allArtists.end(), moreThanArtistTimestampDES);
            }
            break;
            case DmGlobal::SortByArtistDES: {
                std::sort(m_data->m_allArtists.begin(), m_data->m_allArtists.end(), moreThanArtistTitleDES);
            }
            break;
            default:
                sortFlag = false;
                break;
            }
            if (sortFlag && signalFlag) emit signalPlaylistSortChanged(playlistMeta.uuid);
        } else if (playlistMeta.uuid == "albumResult") {
            bool sortFlag = true;
            QList<DMusic::AlbumInfo> searchAlbums = searchedAlbumInfos();
            switch (sortType) {
            case DmGlobal::SortByAddTimeASC: {
                std::sort(searchAlbums.begin(), searchAlbums.end(), moreThanAlbumTimestampASC);
            }
            break;
            case DmGlobal::SortByAblumASC: {
                std::sort(searchAlbums.begin(), searchAlbums.end(), moreThanAlbumTitleASC);
            }
            break;
            case DmGlobal::SortByAddTimeDES: {
                std::sort(searchAlbums.begin(), searchAlbums.end(), moreThanAlbumTimestampDES);
            }
            break;
            case DmGlobal::SortByAblumDES: {
                std::sort(searchAlbums.begin(), searchAlbums.end(), moreThanAlbumTitleDES);
            }
            break;
            default:
                sortFlag = false;
                break;
            }
            if (sortFlag && signalFlag) {
                m_data->m_searchAlbums.clear();
                for (const DMusic::AlbumInfo &info : searchAlbums) {
                    m_data->m_searchAlbums.append(info.name);
                }
                emit signalPlaylistSortChanged(playlistMeta.uuid);
            }
        } else {
            bool sortFlag = true;
            QList<DMusic::ArtistInfo> searchArtists = searchedArtistInfos();
            switch (sortType) {
            case DmGlobal::SortByAddTimeASC: {
                std::sort(searchArtists.begin(), searchArtists.end(), moreThanArtistTimestampASC);
            }
            break;
            case DmGlobal::SortByArtistASC: {
                std::sort(searchArtists.begin(), searchArtists.end(), moreThanArtistTitleASC);
            }
            break;
            case DmGlobal::SortByAddTimeDES: {
                std::sort(searchArtists.begin(), searchArtists.end(), moreThanArtistTimestampDES);
            }
            break;
            case DmGlobal::SortByArtistDES: {
                std::sort(searchArtists.begin(), searchArtists.end(), moreThanArtistTitleDES);
            }
            break;
            default:
                sortFlag = false;
                break;
            }
            if (sortFlag && signalFlag) {
                m_data->m_searchArtists.clear();
                for (const DMusic::ArtistInfo &info : searchArtists) {
                    m_data->m_searchArtists.append(info.name);
                }
                emit signalPlaylistSortChanged(playlistMeta.uuid);
            }
        }
    }
}

bool DataManager::deletePlaylist(QString playlistHash)
{
    int index = playlistIndexFromHash(playlistHash);
    if (index < 0 || index >= m_data->m_allPlaylist.size()) return false;
    m_data->m_allPlaylist.removeAt(index);
    if (currentPlayliHash() == playlistHash)
        setCurrentPlayliHash("");
    return true;
}

bool DataManager::renamePlaylist(const QString &name, const QString &playlistHash)
{
    int index = playlistIndexFromHash(playlistHash);
    if (index < 0 || index >= m_data->m_allPlaylist.size()) return false;

    QStringList existNames;
    for (PlaylistInfo &playlist : m_data->m_allPlaylist) {
        existNames.append(playlist.displayName);
    }
    if (existNames.contains(name)) return false;

    m_data->m_allPlaylist[index].displayName = name;
    return true;
}

void DataManager::movePlaylist(const QString &hash, const QString &nextHash)
{
    int index = playlistIndexFromHash(hash);
    if (index < 0 || index >= m_data->m_allPlaylist.size()) return;

    DMusic::PlaylistInfo playlist = m_data->m_allPlaylist[index];
    m_data->m_allPlaylist.removeAt(index);

    int nextIndex = playlistIndexFromHash(nextHash);
    if (nextIndex < 0 || nextIndex >= m_data->m_allPlaylist.size()) {
        m_data->m_allPlaylist.append(playlist); m_data->m_allPlaylist.append(playlist);
    } else {
        m_data->m_allPlaylist.insert(nextIndex, playlist);
    }

    uint num = 12;
    for (int i = 0; i < m_data->m_allPlaylist.size(); ++i) {
        if (m_data->m_allPlaylist[i].sortID > 12) {
            m_data->m_allPlaylist[i].sortID = (++num);
        }
    }
}

bool DataManager::isExistMeta(const QString &metaHash, const QString &playlistHash)
{
    int index = playlistIndexFromHash(playlistHash);
    if (index < 0 || index >= m_data->m_allPlaylist.size()) return false;

    return m_data->m_allPlaylist[index].sortMetas.contains(metaHash);
}

void DataManager::updateMetaCodec(const MediaMeta &meta)
{
    int index = metaIndexFromHash(meta.hash);
    if (index < 0 || index >= m_data->m_allMetas.size()) return;
    m_data->m_allMetas[index] = meta;

    QString preAlbum, preArtist;
    for (int i = 0; i < m_data->m_allAlbums.size(); ++i) {
        if (m_data->m_allAlbums[i].musicinfos.contains(meta.hash)) {
            bool existFla = false;
            preAlbum = m_data->m_allAlbums[i].name;
            for (int j = 0; j < m_data->m_allAlbums.size(); ++j) {
                if (m_data->m_allAlbums[j].name == meta.album) {
                    m_data->m_allAlbums[j].musicinfos[meta.hash] = meta;
                    existFla = true;
                }
            }
            if (!existFla) {
                AlbumInfo albumNew;
                albumNew.pinyin = meta.pinyinAlbum;
                albumNew.name = meta.album;
                albumNew.artist = meta.artist;
                albumNew.musicinfos[meta.hash] = meta;
                albumNew.timestamp = meta.timestamp;
                m_data->m_allAlbums.append(albumNew);
            }
            m_data->m_allAlbums[i].musicinfos.remove(meta.hash);
            // 踢出空专辑
            if (m_data->m_allAlbums[i].musicinfos.isEmpty())
                m_data->m_allAlbums.removeAt(i);
            break;
        }
    }
    for (int i = 0; i < m_data->m_allArtists.size(); ++i) {
        if (m_data->m_allArtists[i].musicinfos.contains(meta.hash)) {
            bool existFla = false;
            preArtist = m_data->m_allArtists[i].name;
            for (int j = 0; j < m_data->m_allArtists.size(); ++j) {
                if (m_data->m_allArtists[j].name == meta.artist) {
                    m_data->m_allArtists[j].musicinfos[meta.hash] = meta;
                    existFla = true;
                }
            }
            if (!existFla) {
                ArtistInfo artist;
                artist.pinyin = meta.pinyinArtist;
                artist.name = meta.artist;
                artist.musicinfos[meta.hash] = meta;
                artist.timestamp = meta.timestamp;
                m_data->m_allArtists.append(artist);
            }
            m_data->m_allArtists[i].musicinfos.remove(meta.hash);
            // 踢出空艺人
            if (m_data->m_allArtists[i].musicinfos.isEmpty())
                m_data->m_allArtists.removeAt(i);
            break;
        }
    }
    emit signalUpdatedMetaCodec(meta, preAlbum, preArtist);
}

void DataManager::quickSearchText(const QString &text, QStringList &metaTitles,
                                  QList<QPair<QString, QString> > &albums, QList<QPair<QString, QString> > &artists)
{
    // 查找名称
    for (const MediaMeta &meta : m_data->m_allMetas) {
        if (Utils::containsStr(text, meta.title)) metaTitles.append(meta.title);
        if (metaTitles.size() > 4) break;
    }
    for (const DMusic::AlbumInfo &album : allAlbumInfos()) {
        if (!album.musicinfos.isEmpty() && Utils::containsStr(text, album.name))
            albums.append(qMakePair(album.name, album.musicinfos.first().coverUrl));
        if (albums.size() > 2) break;
    }
    for (const DMusic::ArtistInfo &artist : allArtistInfos()) {
        if (!artist.musicinfos.isEmpty() && Utils::containsStr(text, artist.name))
            artists.append(qMakePair(artist.name, artist.musicinfos.first().coverUrl));
        if (artists.size() > 2) break;
    }
}

void DataManager::searchText(const QString &text, QList<MediaMeta> &metas,
                             QList<AlbumInfo> &albums, QList<ArtistInfo> &artists, const QString &type)
{
    // 查找名称
    m_data->m_searchMetas.clear();
    m_data->m_searchAlbums.clear();
    m_data->m_searchArtists.clear();
    if (type == "album") {
        QSet<QString> artistNames;
        for (const DMusic::AlbumInfo &album : allAlbumInfos()) {
            if (!album.musicinfos.isEmpty() && Utils::containsStr(text, album.name)) {
                albums.append(album);

                for (QString metaHash : album.musicinfos.keys()) {
                    if (!m_data->m_searchMetas.contains(metaHash)) {
                        metas.append(album.musicinfos[metaHash]);
                        m_data->m_searchMetas.append(metaHash);
                    }
                    artistNames.insert(album.musicinfos[metaHash].artist);
                }
            }
        }

        for (const DMusic::ArtistInfo &artist : allArtistInfos()) {
            if (!artist.musicinfos.isEmpty() && artistNames.contains(artist.name))
                artists.append(artist);
        }
    } else if (type == "artist") {
        QSet<QString> albumNames;
        for (const DMusic::ArtistInfo &artist : allArtistInfos()) {
            if (!artist.musicinfos.isEmpty() && Utils::containsStr(text, artist.name)) {
                artists.append(artist);

                for (QString metaHash : artist.musicinfos.keys()) {
                    if (!m_data->m_searchMetas.contains(metaHash)) {
                        metas.append(artist.musicinfos[metaHash]);
                        m_data->m_searchMetas.append(metaHash);
                    }
                    albumNames.insert(artist.musicinfos[metaHash].album);
                }
            }
        }

        for (const DMusic::AlbumInfo &album : allAlbumInfos()) {
            if (!album.musicinfos.isEmpty() && albumNames.contains(album.name)) {
                albums.append(album);
            }
        }
    } else if (type == "music") {
        QSet<QString> albumNames, artistNames;
        for (const MediaMeta &meta : m_data->m_allMetas) {
            if (Utils::containsStr(text, meta.title)) {
                m_data->m_searchMetas.append(meta.hash);
                metas.append(meta);
                albumNames.insert(meta.album);
                artistNames.insert(meta.artist);
            }
        }
        for (const DMusic::ArtistInfo &artist : allArtistInfos()) {
            if (!artist.musicinfos.isEmpty() && artistNames.contains(artist.name))
                artists.append(artist);
        }
        for (const DMusic::AlbumInfo &album : allAlbumInfos()) {
            if (!album.musicinfos.isEmpty() && albumNames.contains(album.name)) {
                albums.append(album);
            }
        }
    } else {
        QSet<QString> albumNames, artistNames;
        for (const MediaMeta &meta : m_data->m_allMetas) {
            if (Utils::containsStr(text, meta.title)) {
                m_data->m_searchMetas.append(meta.hash);
                metas.append(meta);
                albumNames.insert(meta.album);
                artistNames.insert(meta.artist);
            }
        }
        for (const DMusic::AlbumInfo &album : allAlbumInfos()) {
            if (!album.musicinfos.isEmpty() && Utils::containsStr(text, album.name)) {
                for (QString metaHash : album.musicinfos.keys()) {
                    if (!m_data->m_searchMetas.contains(metaHash)) {
                        m_data->m_searchMetas.append(metaHash);
                        metas.append(album.musicinfos[metaHash]);
                        artistNames.insert(album.musicinfos[metaHash].artist);
                    }
                }
                albumNames.insert(album.name);
            }
        }
        for (const DMusic::ArtistInfo &artist : allArtistInfos()) {
            if (!artist.musicinfos.isEmpty() && Utils::containsStr(text, artist.name)) {
                for (QString metaHash : artist.musicinfos.keys()) {
                    if (!m_data->m_searchMetas.contains(metaHash)) {
                        m_data->m_searchMetas.append(metaHash);
                        metas.append(artist.musicinfos[metaHash]);
                        albumNames.insert(artist.musicinfos[metaHash].album);
                    }
                }
                artistNames.insert(artist.name);
            }
        }

        for (const DMusic::ArtistInfo &artist : allArtistInfos()) {
            if (!artist.musicinfos.isEmpty() && artistNames.contains(artist.name)) {
                artists.append(artist);
                m_data->m_searchArtists.append(artist.name);
            }
        }
        if (!m_data->m_searchArtists.isEmpty()) {
            int index = playlistIndexFromHash("artistResult");
            if (index >= 0 || index < m_data->m_allPlaylist.size()) {
                sortPlaylist(m_data->m_allPlaylist[index].sortType, "artistResult", false);
            }
        }
        for (const DMusic::AlbumInfo &album : allAlbumInfos()) {
            if (!album.musicinfos.isEmpty() && albumNames.contains(album.name)) {
                albums.append(album);
                m_data->m_searchAlbums.append(album.name);
            }
        }
        if (!m_data->m_searchArtists.isEmpty()) {
            int index = playlistIndexFromHash("albumResult");
            if (index >= 0 || index < m_data->m_allPlaylist.size()) {
                sortPlaylist(m_data->m_allPlaylist[index].sortType, "albumResult", false);
            }
        }
    }
}

QList<AlbumInfo> DataManager::searchedAlbumInfos()
{
    QList<AlbumInfo> searchedInfos;
    for (QString name : m_data->m_searchAlbums) {
        for (const DMusic::AlbumInfo &album : allAlbumInfos()) {
            if (!album.musicinfos.isEmpty() && album.name == name) {
                searchedInfos.append(album);
                break;
            }
        }
    }
    return searchedInfos;
}

QList<ArtistInfo> DataManager::searchedArtistInfos()
{
    QList<ArtistInfo> searchedInfos;
    for (QString name : m_data->m_searchArtists) {
        for (const DMusic::ArtistInfo &artist : allArtistInfos()) {
            if (!artist.musicinfos.isEmpty() && artist.name == name) {
                searchedInfos.append(artist);
                break;
            }
        }
    }
    return searchedInfos;
}

QVariantList DataManager::searchedAlbumVariantList()
{
    QVariantList allList;
    for (AlbumInfo info : searchedAlbumInfos()) {
        allList.append(Utils::albumToVariantMap(info));
    }
    return allList;
}

QVariantList DataManager::searchedArtistVariantList()
{
    QVariantList allList;
    for (ArtistInfo info : searchedArtistInfos()) {
        allList.append(Utils::artistToVariantMap(info));
    }
    return allList;
}

void DataManager::syncToSettings()
{
    m_data->m_settings->sync();
}

void DataManager::resetToSettings()
{
    m_data->m_settings->reset();
}

QVariant DataManager::valueFromSettings(const QString &key)
{
    return m_data->m_settings->value(key);
}

void DataManager::setValueToSettings(const QString &key, const QVariant &value, const bool &empty)
{
    if (empty || (!empty && !value.isNull()))
        m_data->m_settings->setValue(key, value);
}

void DataManager::slotAddOneMeta(QStringList playlistHashs, MediaMeta meta)
{
    MediaMeta curMeta = playlistHashs.contains("all") ? meta : metaFromHash(meta.hash);
    for (PlaylistInfo &playlist : m_data->m_allPlaylist) {
        for (QString hash : playlistHashs) {
            if (hash == playlist.uuid) {
                if (hash == "all") {
                    m_data->m_allMetas.append(curMeta);
                    addMetaToAlbum(curMeta);
                    addMetaToArtist(curMeta);
                }
                playlist.sortMetas.append(curMeta.hash);
                playlist.sortCustomMetas.append(curMeta.hash);
            }
        }
    }
    emit signalAddOneMeta(playlistHashs, curMeta, true);
}
void DataManager::slotLazyLoadDatabase()
{
    loadMetasDB();
    loadPlaylistMetasDB();
    QStringList playlistHashs;
    for (PlaylistInfo &playlist : m_data->m_allPlaylist) {
        if (playlist.saveFalg)
            playlistHashs.append(playlist.uuid);
    }
    emit signalAddMetaFinished(playlistHashs);
}

void DataManager::initPlaylist()
{
    PlaylistInfo playlistMeta;
    playlistMeta.uuid = "album";
    playlistMeta.displayName = "Albums";
    playlistMeta.icon = "album";
    playlistMeta.readonly = true;
    playlistMeta.hide = false;
    playlistMeta.sortID = 1;
    playlistMeta.sortType = DmGlobal::SortByAddTimeASC;
    playlistMeta.saveFalg = true;
    m_data->m_allPlaylist << playlistMeta;

    playlistMeta.uuid = "artist";
    playlistMeta.displayName = "Artists";
    playlistMeta.icon = "artist";
    playlistMeta.readonly = true;
    playlistMeta.hide = false;
    playlistMeta.sortID = 2;
    playlistMeta.saveFalg = true;
    m_data->m_allPlaylist << playlistMeta;

    playlistMeta.uuid = "all";
    playlistMeta.displayName = "All Music";
    playlistMeta.icon = "all";
    playlistMeta.readonly = true;
    playlistMeta.hide = false;
    playlistMeta.sortID = 3;
    playlistMeta.saveFalg = true;
    m_data->m_allPlaylist << playlistMeta;

    playlistMeta.uuid = "fav";
    playlistMeta.displayName = "My Favorites";
    playlistMeta.icon = "fav";
    playlistMeta.readonly = true;
    playlistMeta.hide = false;
    playlistMeta.sortType = DmGlobal::SortByCustomASC;
    playlistMeta.sortID = 4;
    playlistMeta.saveFalg = true;
    m_data->m_allPlaylist << playlistMeta;

    playlistMeta.uuid = "play";
    playlistMeta.displayName = "Playlist";
    playlistMeta.icon = "play";
    playlistMeta.readonly = true;
    playlistMeta.hide = true;
    playlistMeta.sortType = DmGlobal::SortByCustomASC;
    playlistMeta.sortID = 5;
    playlistMeta.saveFalg = true;
    m_data->m_allPlaylist << playlistMeta;

    playlistMeta.uuid = "search";
    playlistMeta.displayName = "Search result";
    playlistMeta.icon = "search";
    playlistMeta.readonly = true;
    playlistMeta.hide = true;
    playlistMeta.sortType = DmGlobal::SortByAddTimeASC;
    playlistMeta.sortID = 6;
    playlistMeta.saveFalg = false;
    m_data->m_allPlaylist << playlistMeta;

    playlistMeta.uuid = "musicCand";
    playlistMeta.displayName = "Music";
    playlistMeta.icon = "musicCand";
    playlistMeta.readonly = true;
    playlistMeta.hide = true;
    playlistMeta.sortID = 7;
    playlistMeta.saveFalg = false;
    m_data->m_allPlaylist << playlistMeta;

    playlistMeta.uuid = "albumCand";
    playlistMeta.displayName = "Album";
    playlistMeta.icon = "albumCand";
    playlistMeta.readonly = true;
    playlistMeta.hide = true;
    playlistMeta.sortID = 8;
    playlistMeta.saveFalg = false;
    m_data->m_allPlaylist << playlistMeta;

    playlistMeta.uuid = "artistCand";
    playlistMeta.displayName = "Artist";
    playlistMeta.icon = "artistCand";
    playlistMeta.readonly = true;
    playlistMeta.hide = true;
    playlistMeta.sortID = 9;
    playlistMeta.saveFalg = false;
    m_data->m_allPlaylist << playlistMeta;

    playlistMeta.uuid = "musicResult";
    playlistMeta.displayName = "Music";
    playlistMeta.icon = "musicResult";
    playlistMeta.readonly = true;
    playlistMeta.hide = true;
    playlistMeta.sortID = 10;
    playlistMeta.saveFalg = false;
    m_data->m_allPlaylist << playlistMeta;

    playlistMeta.uuid = "albumResult";
    playlistMeta.displayName = "Album";
    playlistMeta.icon = "albumResult";
    playlistMeta.readonly = true;
    playlistMeta.hide = true;
    playlistMeta.sortID = 11;
    playlistMeta.saveFalg = false;
    m_data->m_allPlaylist << playlistMeta;

    playlistMeta.uuid = "artistResult";
    playlistMeta.displayName = "Artist";
    playlistMeta.icon = "artistResult";
    playlistMeta.readonly = true;
    playlistMeta.hide = true;
    playlistMeta.sortID = 12;
    playlistMeta.saveFalg = false;
    m_data->m_allPlaylist << playlistMeta;

    QString dbPath = DmGlobal::cachePath() + "/mediameta.sqlite";
    qDebug() << "dbPath:" << dbPath;
    m_data->m_database = QSqlDatabase::addDatabase("QSQLITE");
    m_data->m_database.setDatabaseName(dbPath);
    if (!m_data->m_database.open()) {
        qDebug() << __FUNCTION__ << m_data->m_database.lastError();
    }

    QSqlQuery query(m_data->m_database);
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
                         "cuepath VARCHAR(4096), "
                         "orititle VARCHAR(256), oriartist VARCHAR(256), orialbum VARCHAR(256) )"
                        );

    // 添加列
    isExec &= query.exec("ALTER TABLE musicNew ADD orititle VARCHAR(256)");
    isExec &= query.exec("ALTER TABLE musicNew ADD oriartist VARCHAR(256)");
    isExec &= query.exec("ALTER TABLE musicNew ADD orialbum VARCHAR(256)");

//    isExec &= query.exec("ALTER TABLE musicNew ADD (orititle VARCHAR(256), "
//                         "oriartist VARCHAR(256), orialbum VARCHAR(256) )"
//                         "alter table musicNew add oriartist VARCHAR(256),"
//                         "alter table musicNew add orialbum VARCHAR(256) )"
//    );
//    isExec &= query.exec("ALTER TABLE musicNew ADD orititle VARCHAR(256)");

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

    // 初始化数据库
    if (!isPlaylistExistDB("album")) {
        for (PlaylistInfo playlist : m_data->m_allPlaylist) {
            addPlaylistDB(playlist);
        }
    } else {
        loadPlaylistDB();

        // 延迟加载
        if (allMusicCountDB() > FirstLoadCount && false) {
            loadCurrentMetasDB();
            QTimer::singleShot(500, this, &DataManager::slotLazyLoadDatabase);
        } else {
            loadMetasDB();
            loadPlaylistMetasDB();
        }
    }
}
