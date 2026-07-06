// Copyright (C) 2020 ~ 2026 Uniontech Software Technology Co., Ltd.
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
#include <QSet>
#include <QRegularExpression>
#include <QTimer>
#include <QDebug>
#include <QCoreApplication>

#include "audioanalysis.h"
#include "utils.h"
#include "dboperate.h"
#include "musicsettings.h"
#include "util/log.h"

using namespace DMusic;

static  QString strcmpArtistName = "";
static  QString strcmpAlbumName = "";
static int FirstLoadCount = 15;

// Whitelist check for playlist uuid used as a SQL table name suffix.
// Qt QSqlQuery cannot bind table names via placeholders, so uuid is concatenated
// into SQL. Reject any uuid containing characters outside [A-Za-z0-9_-] to block
// SQL injection (e.g. "all; DROP TABLE ..."). Legitimate uuids are builtin
// keywords (all/album/artist/play/fav) or hex QUuid strings.
static bool isValidPlaylistUuid(const QString &uuid)
{
    static const QRegularExpression re("^[A-Za-z0-9_-]+$");
    return re.match(uuid).hasMatch();
}

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
    DataManagerPrivate(QStringList supportedSuffixs, DataManager *parent, const QString &dbPath)
        : m_parent(parent), m_dbPath(dbPath)
    {
        qCDebug(dmMusic) << "Initializing DataManagerPrivate with supported suffixes:" << supportedSuffixs;
        m_settings = new MusicSettings(m_parent);
        m_currentHash = m_settings->value("base.play.last_playlist").toString();
        if (m_currentHash.isEmpty()) m_currentHash = "all";
        m_dbOperate = new DBOperate(supportedSuffixs);
        m_workerThread = new QThread(m_parent);
        m_dbOperate->moveToThread(m_workerThread);
        qCDebug(dmMusic) << "DataManagerPrivate initialized with current playlist:" << m_currentHash;
    }
    ~DataManagerPrivate()
    {
        qCDebug(dmMusic) << "Destroying DataManagerPrivate";
        stopWorkerAndDrain();
    }

    void stopWorkerAndDrain()
    {
        if (m_workerStopped) return;
        m_workerStopped = true;

        if (m_dbOperate) {
            // 让 m_dbOperate 在自己 affinity 的 worker 线程里析构。
            //    deleteLater 在 worker 的事件循环里 post 一个 DeferredDelete 事件；
            //    quit() 后 worker 的 exec() 在返回前会先处理 DeferredDelete，从而在
            //    正确的线程上 delete，满足 Qt 线程亲和性约束。
            m_dbOperate->deleteLater();
            m_dbOperate = nullptr;
        }
        // 请求事件循环退出 + 等线程真的结束。保持 DBOperate->DataManager 连接到
        // worker 停止后，再排空主线程上已 posted 的封面回填事件。
        if (m_workerThread) {
            m_workerThread->quit();
            m_workerThread->wait();
            delete m_workerThread;
            m_workerThread = nullptr;
        }
        // Worker 已停，排空主线程上此前已 posted 给 DataManager 的 metacall
        // （如 signalMetaCoverReady/signalCoverBatchFinished）。此时 m_parent 仍有效，
        // slot 可安全更新 m_importedMetas，随后 DataManager 析构会统一保存。
        QCoreApplication::sendPostedEvents(m_parent, 0);
    }
private:
    friend class DataManager;
    DataManager                      *m_parent;
    QThread                          *m_workerThread     = nullptr;
    DBOperate                        *m_dbOperate        = nullptr;
    MusicSettings                    *m_settings         = nullptr;
    bool                               m_workerStopped    = false;
    QSqlDatabase                      m_database;
    QString                           m_dbPath;           // 可注入的 DB 路径，空=默认 cachePath/mediameta.sqlite
    QString                           m_currentHash;
    QList<DMusic::MediaMeta>          m_allMetas;
    QHash<QString, int>               m_metaHashIndex;  // hash -> m_allMetas index, for O(1) metaIndexFromHash
    QList<DMusic::MediaMeta>          m_importedMetas;   // newly imported metas this batch; consumed and cleared by upsertMetasDB
    bool                               m_dirty = false;   // true if persistent in-memory model changed and needs full saveDataToDB on exit
    QList<DMusic::AlbumInfo>          m_allAlbums;
    QHash<QString, int>               m_albumNameIndex;   // album name -> m_allAlbums index
    QList<DMusic::ArtistInfo>         m_allArtists;
    QHash<QString, int>               m_artistNameIndex;  // artist name -> m_allArtists index
    QList<DMusic::PlaylistInfo>       m_allPlaylist;
    QList<QString>                    m_searchMetas;
    QList<QString>                    m_searchArtists;
    QList<QString>                    m_searchAlbums;
};

DataManager::DataManager(QStringList supportedSuffixs, QObject *parent, const QString &dbPath)
    : QObject(parent), m_data(new DataManagerPrivate(supportedSuffixs, this, dbPath))
{
    qCDebug(dmMusic) << "Initializing DataManager with supported suffixes:" << supportedSuffixs;
    initPlaylist();

    connect(this, &DataManager::signalImportMetas, m_data->m_dbOperate, &DBOperate::slotImportMetas, Qt::QueuedConnection);
    connect(m_data->m_dbOperate, &DBOperate::signalAddOneMeta, this, &DataManager::slotAddOneMeta, Qt::QueuedConnection);
    connect(m_data->m_dbOperate, &DBOperate::signalImportFinished, this, &DataManager::signalImportFinished, Qt::QueuedConnection);
    connect(m_data->m_dbOperate, &DBOperate::signalCoverBatchFinished, this, &DataManager::slotCoverBatchFinished, Qt::QueuedConnection);
    connect(m_data->m_dbOperate, &DBOperate::signalMetaCoverReady, this, &DataManager::slotMetaCoverReady, Qt::QueuedConnection);
    connect(this, &DataManager::signalClearImportingHash, m_data->m_dbOperate, &DBOperate::slotClearImportingHash, Qt::QueuedConnection);

    m_data->m_workerThread->start();
    qCDebug(dmMusic) << "DataManager initialized with worker thread";
}

DataManager::~DataManager()
{
    qCDebug(dmMusic) << "Destroying DataManager";
    if (m_data) {
        m_data->stopWorkerAndDrain();
        saveDataToDB();
        // 关键：触发 DataManagerPrivate 析构走安全销毁路径（disconnect + deleteLater + quit + wait）
        delete m_data;
        m_data = nullptr;
    }
    qCDebug(dmMusic) << "DataManager destroyed";
}

void DataManager::setCurrentPlayliHash(const QString &hash)
{
    qCInfo(dmMusic) << "Setting current playlist hash from" << m_data->m_currentHash << "to" << hash;
    m_data->m_currentHash = hash;
    m_data->m_settings->setValue("base.play.last_playlist", hash);
    emit signalCurrentPlaylistSChanged(hash);
}

QString DataManager::currentPlayliHash()
{
    qCInfo(dmMusic) << "Getting current playlist hash:" << m_data->m_currentHash;
    return m_data->m_currentHash;
}

int DataManager::metaIndexFromHash(const QString &hash)
{
    qCDebug(dmMusic) << "Looking up meta index for hash:" << hash;
    if (hash.isEmpty()) return -1;

    auto it = m_data->m_metaHashIndex.constFind(hash);
    if (it == m_data->m_metaHashIndex.constEnd()) {
        return -1;  // miss, keep O(1) (import new songs mostly miss)
    }

    int idx = it.value();
    if (idx >= 0 && idx < m_data->m_allMetas.size()
            && m_data->m_allMetas[idx].hash == hash) {
        return idx;
    }

    // hit but index entry stale (rare): rebuild and recheck
    rebuildMetaHashIndex();
    it = m_data->m_metaHashIndex.constFind(hash);
    if (it == m_data->m_metaHashIndex.constEnd()) {
        return -1;
    }
    idx = it.value();
    if (idx >= 0 && idx < m_data->m_allMetas.size()
            && m_data->m_allMetas[idx].hash == hash) {
        return idx;
    }
    return -1;
}

void DataManager::rebuildMetaHashIndex()
{
    m_data->m_metaHashIndex.clear();
    m_data->m_metaHashIndex.reserve(m_data->m_allMetas.size());
    for (int i = 0; i < m_data->m_allMetas.size(); ++i) {
        // duplicate hash: first wins, matching original linear-scan behavior
        if (!m_data->m_metaHashIndex.contains(m_data->m_allMetas[i].hash)) {
            m_data->m_metaHashIndex.insert(m_data->m_allMetas[i].hash, i);
        }
    }
}

void DataManager::rebuildAlbumNameIndex()
{
    m_data->m_albumNameIndex.clear();
    m_data->m_albumNameIndex.reserve(m_data->m_allAlbums.size());
    for (int i = 0; i < m_data->m_allAlbums.size(); ++i) {
        if (!m_data->m_albumNameIndex.contains(m_data->m_allAlbums[i].name)) {
            m_data->m_albumNameIndex.insert(m_data->m_allAlbums[i].name, i);
        }
    }
}

void DataManager::rebuildArtistNameIndex()
{
    m_data->m_artistNameIndex.clear();
    m_data->m_artistNameIndex.reserve(m_data->m_allArtists.size());
    for (int i = 0; i < m_data->m_allArtists.size(); ++i) {
        if (!m_data->m_artistNameIndex.contains(m_data->m_allArtists[i].name)) {
            m_data->m_artistNameIndex.insert(m_data->m_allArtists[i].name, i);
        }
    }
}

int DataManager::albumIndexFromName(const QString &name)
{
    auto it = m_data->m_albumNameIndex.constFind(name);
    if (it == m_data->m_albumNameIndex.constEnd()) {
        return -1;  // normal miss, keep O(1)
    }
    int idx = it.value();
    if (idx >= 0 && idx < m_data->m_allAlbums.size()
            && m_data->m_allAlbums[idx].name == name) {
        return idx;
    }
    rebuildAlbumNameIndex();  // stale hit only: rebuild and recheck
    it = m_data->m_albumNameIndex.constFind(name);
    if (it == m_data->m_albumNameIndex.constEnd()) {
        return -1;
    }
    idx = it.value();
    if (idx >= 0 && idx < m_data->m_allAlbums.size()
            && m_data->m_allAlbums[idx].name == name) {
        return idx;
    }
    return -1;
}

int DataManager::artistIndexFromName(const QString &name)
{
    auto it = m_data->m_artistNameIndex.constFind(name);
    if (it == m_data->m_artistNameIndex.constEnd()) {
        return -1;
    }
    int idx = it.value();
    if (idx >= 0 && idx < m_data->m_allArtists.size()
            && m_data->m_allArtists[idx].name == name) {
        return idx;
    }
    rebuildArtistNameIndex();
    it = m_data->m_artistNameIndex.constFind(name);
    if (it == m_data->m_artistNameIndex.constEnd()) {
        return -1;
    }
    idx = it.value();
    if (idx >= 0 && idx < m_data->m_allArtists.size()
            && m_data->m_allArtists[idx].name == name) {
        return idx;
    }
    return -1;
}

int DataManager::playlistIndexFromHash(const QString &hash)
{
    qCDebug(dmMusic) << "Looking up playlist index for hash:" << hash;
    int index = -1;
    if (!hash.isEmpty()) {
        qCDebug(dmMusic) << "Total playlists:" << m_data->m_allPlaylist.size();
        for (int i = 0; i < m_data->m_allPlaylist.size(); ++i) {
            if (m_data->m_allPlaylist[i].uuid == hash) {
                index = i;
                break;
            }
        }
    }
    qCDebug(dmMusic) << "Found playlist index:" << index << "for hash:" << hash;
    return index;
}

void DataManager::deleteMetaFromAllMetas(const QStringList &hashs)
{
    qCDebug(dmMusic) << "Deleting metas with hashes:" << hashs;
    QStringList allHashs = hashs;
    for (int i = m_data->m_allMetas.size() - 1; i >= 0; --i) {
        if (allHashs.contains(m_data->m_allMetas[i].hash)) {
            QString deletedHash = m_data->m_allMetas[i].hash;
            allHashs.removeOne(deletedHash);
            m_data->m_allMetas.removeAt(i);
            // 通知 DBOperate 清理 m_importingHashes 中对应的 hash，允许重新导入
            emit signalClearImportingHash(deletedHash);
            if (allHashs.isEmpty()) break;
        }
    }
    rebuildMetaHashIndex();  // removeAt shifts indices; full rebuild is simplest
    qCDebug(dmMusic) << "Finished deleting metas";
}

void DataManager::addMetaToAlbum(const MediaMeta &meta)
{
    qCDebug(dmMusic) << "Adding meta to album:" << meta.album << "artist:" << meta.artist;
    const int albumIndex = albumIndexFromName(meta.album);
    if (albumIndex >= 0) {
        auto &album = m_data->m_allAlbums[albumIndex];
        album.musicinfos[meta.hash] = meta;
        if (meta.timestamp < album.timestamp) {
            album.timestamp = meta.timestamp;
        }
        qCDebug(dmMusic) << "Updated existing album:" << meta.album;
    } else {
        AlbumInfo albumNew;
        albumNew.pinyin = meta.pinyinAlbum;
        albumNew.name = meta.album;
        albumNew.artist = meta.artist;
        albumNew.musicinfos[meta.hash] = meta;
        albumNew.timestamp = meta.timestamp;
        if (!m_data->m_albumNameIndex.contains(meta.album)) {
            m_data->m_albumNameIndex.insert(meta.album, m_data->m_allAlbums.size());
        }
        m_data->m_allAlbums.append(albumNew);
        qCDebug(dmMusic) << "Created new album:" << meta.album;
    }
}

void DataManager::deleteMetaFromAlbum(const QString &metaHash, const QString &name)
{
    qCDebug(dmMusic) << "Deleting meta" << metaHash << "from album:" << name;
    for (int i = 0; i < m_data->m_allAlbums.size(); ++i) {
        if (m_data->m_allAlbums[i].name == name) {
            m_data->m_allAlbums[i].musicinfos.remove(metaHash);
            if (m_data->m_allAlbums[i].musicinfos.isEmpty()) {
                m_data->m_allAlbums.removeAt(i);
                qCDebug(dmMusic) << "Removed empty album:" << name;
            }
            break;
        }
    }
    rebuildAlbumNameIndex();  // removeAt shifts indices
}

void DataManager::addMetaToArtist(const MediaMeta &meta)
{
    qCDebug(dmMusic) << "Adding meta to artist:" << meta.artist;
    const int artistIndex = artistIndexFromName(meta.artist);
    if (artistIndex >= 0) {
        auto &artist = m_data->m_allArtists[artistIndex];
        artist.musicinfos[meta.hash] = meta;
        if (meta.timestamp < artist.timestamp) {
            artist.timestamp = meta.timestamp;
        }
        qCDebug(dmMusic) << "Updated existing artist:" << meta.artist;
    } else {
        ArtistInfo artist;
        artist.pinyin = meta.pinyinArtist;
        artist.name = meta.artist;
        artist.musicinfos[meta.hash] = meta;
        artist.timestamp = meta.timestamp;
        if (!m_data->m_artistNameIndex.contains(meta.artist)) {
            m_data->m_artistNameIndex.insert(meta.artist, m_data->m_allArtists.size());
        }
        m_data->m_allArtists.append(artist);
        qCDebug(dmMusic) << "Created new artist:" << meta.artist;
    }
}

void DataManager::deleteMetaFromArtist(const QString &metaHash, const QString &name)
{
    qCDebug(dmMusic) << "Deleting meta" << metaHash << "from artist:" << name;
    for (int i = 0; i < m_data->m_allArtists.size(); ++i) {
        if (m_data->m_allArtists[i].name == name) {
            m_data->m_allArtists[i].musicinfos.remove(metaHash);
            if (m_data->m_allArtists[i].musicinfos.isEmpty()) {
                m_data->m_allArtists.removeAt(i);
                qCDebug(dmMusic) << "Removed empty artist:" << name;
            }
            break;
        }
    }
    rebuildArtistNameIndex();  // removeAt shifts indices
}

int DataManager::allMusicCountDB()
{
    qCDebug(dmMusic) << "Getting total music count from database";
    int count = 0;
    QString queryString = QString("SELECT count(*) FROM musicNew");
    QSqlQuery queryNew(m_data->m_database);
    if (!queryNew.prepare(queryString)) {
        qCCritical(dmMusic) << "Failed to prepare query:" << queryNew.lastError();
        return 0;
    }
    if (!queryNew.exec()) {
        qCCritical(dmMusic) << "Failed to execute query:" << queryNew.lastError();
        count = 0;
    }
    while (queryNew.next()) {
        count = queryNew.value(0).toInt();
    }
    qCDebug(dmMusic) << "Total music count:" << count;
    return count;
}

bool DataManager::loadCurrentMetasDB()
{
    qCDebug(dmMusic) << "Loading current playlist metas from database";
    int index = playlistIndexFromHash(m_data->m_currentHash);
    if (index < 0 || index >= m_data->m_allPlaylist.size()) {
        qCWarning(dmMusic) << "Invalid playlist index for hash:" << m_data->m_currentHash;
        return false;
    }

    QSqlQuery query(m_data->m_database);
    // 加载歌单数据
    if (!query.prepare(QString("SELECT music_id FROM playlist_%1 order by sort_id ASC LIMIT %2")
                       .arg(m_data->m_currentHash).arg(FirstLoadCount))) {
        qCWarning(dmMusic) << "Failed to prepare query:" << query.lastError();
        return false;
    }
    if (!query.exec()) {
        qCWarning(dmMusic) << "Failed to execute query:" << query.lastError();
        return false;
    }
    m_data->m_allPlaylist[index].sortMetas.clear();

    while (query.next()) {
        m_data->m_allPlaylist[index].sortMetas.append(query.value(0).toString());
//        m_data->m_allPlaylist[index].sortCustomMetas.append(query.value(0).toString());
    }

    qCDebug(dmMusic) << "Loading song data for" << m_data->m_allPlaylist[index].sortMetas.size() << "metas";
    //加载歌曲数据
    // 防御性编程：使用参数化查询代替字符串拼接
    const QString queryString = "SELECT hash, localpath, title, artist, album, "
                                "filetype, track, offset, length, size, "
                                "timestamp, invalid, search_id, cuepath, "
                                "lyricPath, codec, py_title, py_artist, py_album, hasimage, orititle, oriartist, orialbum "
                                "FROM musicNew WHERE hash=:hash";
    for (const QString &metaStr : m_data->m_allPlaylist[index].sortMetas) {
        if (!query.prepare(queryString)) {
            qCCritical(dmMusic) << "Failed to prepare meta query for hash:" << metaStr << "error:" << query.lastError();
            continue;
        }
        query.bindValue(":hash", metaStr);
        if (!query.exec()) {
            qCCritical(dmMusic) << "Failed to execute meta query for hash:" << metaStr << "error:" << query.lastError();
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
            qCDebug(dmMusic) << "Loaded meta from database:" << meta.title << "artist:" << meta.artist << "album:" << meta.album;
        }
    }

    rebuildMetaHashIndex();
    rebuildAlbumNameIndex();
    rebuildArtistNameIndex();
    qCInfo(dmMusic) << "Successfully loaded" << m_data->m_allMetas.size() << "metas from database";
    return true;
}

bool DataManager::loadMetasDB()
{
    qCDebug(dmMusic) << "Loading all metas from database";
    QString queryString("SELECT hash, localpath, title, artist, album, "
                        "filetype, track, offset, length, size, "
                        "timestamp, invalid, search_id, cuepath, "
                        "lyricPath, codec, py_title, py_artist, py_album, hasimage, orititle, oriartist, orialbum "
                        "FROM musicNew");

    QSqlQuery queryNew(m_data->m_database);
    if (!queryNew.prepare(queryString)) {
        qCCritical(dmMusic) << "Failed to prepare query:" << queryNew.lastError();
        return false;
    }
    if (! queryNew.exec()) {
        qCCritical(dmMusic) << "Failed to execute query:" << queryNew.lastError();
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

    rebuildMetaHashIndex();
    rebuildAlbumNameIndex();
    rebuildArtistNameIndex();
    qCInfo(dmMusic) << "Successfully loaded";
    return true;
}

bool DataManager::loadPlaylistDB()
{
    qCDebug(dmMusic) << "Loading playlists from database";
    QSqlQuery query(m_data->m_database);
    bool isPrepare = query.prepare("SELECT uuid, displayname, icon, readonly, hide, "
                                   "sort_type, order_type, sort_id FROM playlist order by sort_id ASC");

    if ((!isPrepare) || (! query.exec())) {
        qCWarning(dmMusic) << "Failed to load playlists:" << query.lastError();
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

    qCInfo(dmMusic) << "Successfully loaded";
    return true;
}

bool DataManager::loadPlaylistMetasDB()
{
    qCDebug(dmMusic) << "Loading playlist metas from database";
    QSqlQuery query(m_data->m_database);
    // 加载数据
    for (int i = 0; i < m_data->m_allPlaylist.size(); ++i) {
        if (!m_data->m_allPlaylist[i].saveFalg) continue;
        
        if (!query.prepare(QString("SELECT music_id FROM playlist_%1 order by sort_id ASC")
                           .arg(m_data->m_allPlaylist[i].uuid))) {
            qCWarning(dmMusic) << "Failed to prepare query for playlist" << m_data->m_allPlaylist[i].uuid << ":" << query.lastError();
            continue;
        }
        if (!query.exec()) {
            qCWarning(dmMusic) << "Failed to execute query for playlist" << m_data->m_allPlaylist[i].uuid << ":" << query.lastError();
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
        qCWarning(dmMusic) << "Failed to check playlist existence:" << query.lastError();
        return false;
    }
    query.first();

    return query.value(0).toInt() > 0;
}

bool DataManager::addPlaylistDB(const DMusic::PlaylistInfo &playlist)
{
    if (!playlist.saveFalg) return true;

    qCDebug(dmMusic) << "Adding playlist to database:" << playlist.uuid << playlist.displayName;
    QSqlQuery query(m_data->m_database);
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
        qCWarning(dmMusic) << "Failed to add playlist to database:" << query.lastError();
        return false;
    }

    QString sqlstring = QString("CREATE TABLE IF NOT EXISTS playlist_%1 ("
                                "music_id TEXT primary key not null, "
                                "playlist_id TEXT, sort_id INTEGER"
                                ")").arg(playlist.uuid);
    if (! query.exec(sqlstring)) {
        qCWarning(dmMusic) << "Failed to create playlist table:" << query.lastError();
        return false;
    }

    qCDebug(dmMusic) << "Successfully added playlist to database:" << playlist.uuid;
    return true;
}

bool DataManager::deletePlaylistDB(const QString &uuid)
{
    qCDebug(dmMusic) << "Deleting playlist from database:" << uuid;
    QSqlQuery query(m_data->m_database);
    QString sqlstring = QString("DROP TABLE IF EXISTS playlist_%1").arg(uuid);
    if (! query.exec(sqlstring)) {
        qCWarning(dmMusic) << "Failed to drop playlist table:" << query.lastError();
        return false;
    }

    sqlstring = QString("DELETE FROM playlist WHERE uuid = '%1'").arg(uuid);
    if (! query.exec(sqlstring)) {
        qCWarning(dmMusic) << "Failed to delete playlist record:" << query.lastError();
        return false;
    }
    qCDebug(dmMusic) << "Successfully deleted playlist:" << uuid;
    return true;
}

bool DataManager::deleteAllPlaylistDB()
{
    qCDebug(dmMusic) << "Deleting all playlists from database";
    QSqlQuery query(m_data->m_database);
    QString sqlstring = QString("SELECT uuid FROM playlist");
    if (! query.exec(sqlstring)) {
        qCWarning(dmMusic) << "Failed to get playlist list:" << query.lastError();
        return false;
    }
    QStringList allPlaylistIDs;
    while (query.next()) {
        allPlaylistIDs.append(query.value(0).toString());
    }
    bool ok = true;
    for (QString id : allPlaylistIDs) {
        if (!deletePlaylistDB(id)) ok = false;
    }

    return ok;
}

int DataManager::addMetasToPlaylistDB(const QString &uuid, const QList<MediaMeta> &metas)
{
    qCDebug(dmMusic) << "Adding" << metas.size() << "metas to playlist:" << uuid;

    // Reject suspicious uuid to prevent SQL injection via table-name concatenation.
    if (!isValidPlaylistUuid(uuid)) {
        qCCritical(dmMusic) << "addMetasToPlaylistDB: rejected invalid uuid:" << uuid;
        return -1;
    }

    int insert_count = 0;
    bool anyFail = false;  // any failure -> return -1 so caller (saveDataToDB) can感知

    // sort_id base: query MAX once outside the loop instead of per-meta SELECT.
    // Only custom playlists need sort_id; album/artist/all use default 0.
    int nextSortId = 0;
    if (uuid != "album" && uuid != "artist" && uuid != "all") {
        QSqlQuery maxQuery(m_data->m_database);
        QString maxSql = QString("SELECT MAX(sort_id) FROM playlist_%1").arg(uuid);
        if (maxQuery.prepare(maxSql) && maxQuery.exec()) {
            while (maxQuery.next()) {
                nextSortId = maxQuery.value(0).toInt() + 1;
            }
        } else {
            qCCritical(dmMusic) << "Failed to get max sort_id:" << maxQuery.lastError();
            anyFail = true;
        }
    }

    // Dedup set: SELECT music_id once outside the loop instead of per-meta SELECT *.
    QSet<QString> existingHashs;
    {
        QSqlQuery existQuery(m_data->m_database);
        QString existSql = QString("SELECT music_id FROM playlist_%1").arg(uuid);
        if (existQuery.prepare(existSql) && existQuery.exec()) {
            while (existQuery.next()) {
                existingHashs.insert(existQuery.value(0).toString());
            }
        } else {
            qCCritical(dmMusic) << "Failed to load existing music_id:" << existQuery.lastError();
            anyFail = true;
        }
    }

    // Reuse a single prepared INSERT; loop only bindValue + exec.
    QSqlQuery insertQuery(m_data->m_database);
    QString insertSql = QString("INSERT INTO playlist_%1 "
                                "(music_id, playlist_id, sort_id) "
                                "VALUES (:music_id, :playlist_id, :sort_id)").arg(uuid);
    bool isPrepare = insertQuery.prepare(insertSql);
    if (!isPrepare) {
        qCCritical(dmMusic) << "Failed to prepare insert:" << insertQuery.lastError() << insertSql;
        return -1;
    }

    for (const MediaMeta &meta : metas) {
        if (existingHashs.contains(meta.hash)) {
            continue;  // already exists, skip
        }

        int sortId = 0;
        if (uuid != "album" && uuid != "artist" && uuid != "all") {
            sortId = nextSortId++;
        }

        insertQuery.bindValue(":music_id", meta.hash);
        insertQuery.bindValue(":playlist_id", uuid);
        insertQuery.bindValue(":sort_id", sortId);
        if (insertQuery.exec()) {
            existingHashs.insert(meta.hash);  // guard against duplicate hash in same batch
            insert_count++;
            qCDebug(dmMusic) << "Added meta" << meta.title << "to playlist" << uuid;
        } else {
            qCCritical(dmMusic) << "Failed to add meta to playlist:" << insertQuery.lastError() << insertSql;
            anyFail = true;
        }
    }

    qCInfo(dmMusic) << "Successfully added" << insert_count << "metas to playlist" << uuid;
    return anyFail ? -1 : insert_count;
}

bool DataManager::upsertMetasDB()
{
    if (m_data->m_importedMetas.isEmpty()) {
        qCDebug(dmMusic) << "upsertMetasDB: no imported metas, skip";
        return true;
    }

    qCDebug(dmMusic) << "upsertMetasDB: incrementally writing" << m_data->m_importedMetas.size() << "metas";
    m_data->m_database.transaction();

    QSqlQuery query(m_data->m_database);
    bool isPrepare = query.prepare("INSERT OR REPLACE INTO musicNew ("
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
    if (!isPrepare) {
        qCCritical(dmMusic) << "upsertMetasDB: prepare failed:" << query.lastError();
        m_data->m_database.rollback();
        return false;
    }

    int idx = 0;
    for (const MediaMeta &meta : m_data->m_importedMetas) {
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

        if (!query.exec()) {
            // Any single failure aborts the whole batch: rollback guarantees
            // atomicity (no partial/silent loss). Exit-time full saveDataToDB
            // will still persist on next shutdown.
            qCCritical(dmMusic) << "upsertMetasDB: exec failed at index" << idx
                                << "meta:" << meta.title << "error:" << query.lastError();
            m_data->m_database.rollback();
            return false;
        }
        idx++;
    }

    if (!m_data->m_database.commit()) {
        qCCritical(dmMusic) << "upsertMetasDB: commit failed:" << m_data->m_database.lastError();
        m_data->m_database.rollback();
        return false;
    }

    qCInfo(dmMusic) << "upsertMetasDB: wrote" << m_data->m_importedMetas.size() << "metas";
    m_data->m_importedMetas.clear();
    return true;
}

void DataManager::saveDataToDB()
{
    // Guard: skip full rewrite if in-memory model unchanged since load.
    if (!m_data->m_dirty) {
        qCDebug(dmMusic) << "saveDataToDB: not dirty, skip full rewrite";
        return;
    }

    qCDebug(dmMusic) << "Starting to save data to database";
    if (!m_data->m_database.transaction()) {
        qCCritical(dmMusic) << "saveDataToDB: transaction failed:" << m_data->m_database.lastError();
        return;  // dirty 保留
    }

    bool ok = true;
    QSqlQuery query(m_data->m_database);

    // 删除歌曲
    QString sqlStr = QString("DELETE FROM musicNew");
    bool isPrepare = query.prepare(sqlStr);
    if ((!isPrepare) || (! query.exec())) {
        qCCritical(dmMusic) << "Failed to clear musicNew table:" << query.lastError() << sqlStr;
        ok = false;
    }

    // 重插歌曲
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
            qCWarning(dmMusic) << "Failed to save meta to database:" << meta.title << "error:" << query.lastError();
            ok = false;
        }
    }

    // 歌单：deleteAllPlaylistDB 已清全表+各 playlist_<uuid> 表，循环内直接重建。
    // 四个返回值全部纳入 ok（歌单关系是 dirty 兜底核心）。
    if (!deleteAllPlaylistDB()) ok = false;
    for (const PlaylistInfo &info : m_data->m_allPlaylist) {
        if (!info.saveFalg) continue;

        if (!addPlaylistDB(info)) ok = false;
        if (addMetasToPlaylistDB(info.uuid, getPlaylistMetas(info.uuid)) < 0) ok = false;
    }

    // 任一步失败：rollback 并保留 dirty，下次退出重试。
    if (!ok) {
        qCCritical(dmMusic) << "saveDataToDB: errors occurred, rollback, keep dirty";
        m_data->m_database.rollback();
        return;
    }
    if (!m_data->m_database.commit()) {
        qCCritical(dmMusic) << "saveDataToDB: commit failed:" << m_data->m_database.lastError();
        m_data->m_database.rollback();
        return;  // dirty 保留
    }

    m_data->m_dirty = false;  // 全程成功才清
    qCInfo(dmMusic) << "Data saved successfully";
}

MediaMeta DataManager::metaFromHash(const QString &hash)
{
    qCDebug(dmMusic) << "Looking up meta for hash:" << hash;
    MediaMeta mata;
    int index = metaIndexFromHash(hash);
    if (index >= 0 && index < m_data->m_allMetas.size()) mata = m_data->m_allMetas[index];
    return mata;
}

PlaylistInfo DataManager::playlistFromHash(const QString &hash)
{
    qCDebug(dmMusic) << "Looking up playlist for hash:" << hash;
    PlaylistInfo playlist;
    int index = playlistIndexFromHash(hash);
    if (index >= 0 && index < m_data->m_allPlaylist.size()) playlist = m_data->m_allPlaylist[index];

    if (hash == "musicResult")
        playlist.sortMetas = m_data->m_searchMetas;
    return playlist;
}

QList<DMusic::MediaMeta> DataManager::getPlaylistMetas(const QString &hash, int count)
{
    qCDebug(dmMusic) << "Looking up playlist metas for hash:" << hash << "count:" << count;
    QString curHash = !hash.isEmpty() ? hash : "all";
    QList<DMusic::MediaMeta> metas;
    int index = playlistIndexFromHash(curHash);
    if (index < 0 || index >= m_data->m_allPlaylist.size())  return metas;
    int favIndex = playlistIndexFromHash("fav");
    bool favExist = (favIndex >= 0 && favIndex < m_data->m_allPlaylist.size());

    if (hash == "all" && m_data->m_allPlaylist[index].sortMetas.isEmpty()) {
        qCDebug(dmMusic) << "Playlist metas is empty, using all metas";
        for (const DMusic::MediaMeta &meta : m_data->m_allMetas) {
            DMusic::MediaMeta curMeta = meta;
            if (favExist && m_data->m_allPlaylist[favIndex].sortMetas.contains(curMeta.hash)) curMeta.favourite = true;
            if (!meta.hash.isEmpty())
                metas.append(curMeta);
            if (count >= 0 && count == metas.size()) break;
        }
    } else {
        qCDebug(dmMusic) << "Playlist metas is not empty, using custom metas";
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

    qCDebug(dmMusic) << "Returning" << metas.size() << "playlist metas";
    return metas;
}

QList<PlaylistInfo> DataManager::allPlaylistInfos()
{
    qCDebug(dmMusic) << "Getting all playlist infos, count:" << m_data->m_allPlaylist.size();
    return m_data->m_allPlaylist;
}

QVariantList DataManager::allPlaylistVariantList()
{
    qCDebug(dmMusic) << "Getting all playlist variant list";
    QVariantList allList;
    for (const PlaylistInfo &info : m_data->m_allPlaylist) {
        allList.append(Utils::playlistToVariantMap(info));
    }
    qCDebug(dmMusic) << "Retrieved" << allList.size() << "playlist variants";
    return allList;
}

QList<PlaylistInfo> DataManager::customPlaylistInfos()
{
    qCDebug(dmMusic) << "Getting custom playlist infos";
    QList<PlaylistInfo> playlists;
    for (const PlaylistInfo &info : m_data->m_allPlaylist) {
        if (info.readonly) continue;
        playlists.append(info);
    }
    qCDebug(dmMusic) << "Retrieved" << playlists.size() << "custom playlists";
    return playlists;
}

QVariantList DataManager::customPlaylistVariantList()
{
    qCDebug(dmMusic) << "Getting custom playlist variant list";
    QVariantList allList;
    for (const PlaylistInfo &info : customPlaylistInfos()) {
        allList.append(Utils::playlistToVariantMap(info));
    }
    qCDebug(dmMusic) << "Retrieved" << allList.size() << "custom playlist variants";
    return allList;
}

QList<AlbumInfo> DataManager::allAlbumInfos()
{
    qCDebug(dmMusic) << "Getting all album infos";
    m_data->m_allAlbums.clear();
    m_data->m_albumNameIndex.clear();
    for (MediaMeta &meta : getPlaylistMetas()) {
        const int albumIndex = albumIndexFromName(meta.album);
        if (albumIndex >= 0) {
            auto &album = m_data->m_allAlbums[albumIndex];
            album.musicinfos[meta.hash] = meta;
            if (meta.timestamp < album.timestamp) {
                album.timestamp = meta.timestamp;
            }
            qCDebug(dmMusic) << "Updated existing album:" << meta.album;
        } else {
            AlbumInfo albumNew;
            albumNew.name = meta.album;
            albumNew.artist = meta.artist;
            albumNew.pinyin = meta.pinyinAlbum;
            albumNew.musicinfos[meta.hash] = meta;
            albumNew.timestamp = meta.timestamp;
            if (!m_data->m_albumNameIndex.contains(meta.album)) {
                m_data->m_albumNameIndex.insert(meta.album, m_data->m_allAlbums.size());
            }
            m_data->m_allAlbums.append(albumNew);
            qCDebug(dmMusic) << "Created new album:" << meta.album;
        }
    }

    // 排序
    int index = playlistIndexFromHash("album");
    if (index >= 0 && index < m_data->m_allPlaylist.size()) {
        PlaylistInfo &playlistMeta = m_data->m_allPlaylist[index];
        sortPlaylist(playlistMeta.sortType, playlistMeta.uuid, false);
    }

    qCDebug(dmMusic) << "Retrieved" << m_data->m_allAlbums.size() << "albums";
    return m_data->m_allAlbums;
}

QVariantList DataManager::allAlbumVariantList()
{
    qCDebug(dmMusic) << "Getting all album variant list";
    QVariantList allList;
    for (AlbumInfo info : allAlbumInfos()) {
        allList.append(Utils::albumToVariantMap(info));
    }
    qCDebug(dmMusic) << "Retrieved" << allList.size() << "album variants";
    return allList;
}

QList<ArtistInfo> DataManager::allArtistInfos()
{
    qCDebug(dmMusic) << "Getting all artist infos";
    m_data->m_allArtists.clear();
    m_data->m_artistNameIndex.clear();
    for (MediaMeta &meta : getPlaylistMetas()) {
        const int artistIndex = artistIndexFromName(meta.artist);
        if (artistIndex >= 0) {
            auto &artist = m_data->m_allArtists[artistIndex];
            artist.musicinfos[meta.hash] = meta;
            if (meta.timestamp < artist.timestamp) {
                artist.timestamp = meta.timestamp;
            }
            qCDebug(dmMusic) << "Updated existing artist:" << meta.artist;
        } else {
            ArtistInfo artist;
            artist.name = meta.artist;
            artist.pinyin = meta.pinyinArtist;
            artist.musicinfos[meta.hash] = meta;
            artist.timestamp = meta.timestamp;
            if (!m_data->m_artistNameIndex.contains(meta.artist)) {
                m_data->m_artistNameIndex.insert(meta.artist, m_data->m_allArtists.size());
            }
            m_data->m_allArtists.append(artist);
            qCDebug(dmMusic) << "Created new artist:" << meta.artist;
        }
    }
    // 排序
    int index = playlistIndexFromHash("artist");
    if (index >= 0 && index < m_data->m_allPlaylist.size()) {
        PlaylistInfo &playlistMeta = m_data->m_allPlaylist[index];
        sortPlaylist(playlistMeta.sortType, playlistMeta.uuid, false);
    }

    qCDebug(dmMusic) << "Retrieved" << m_data->m_allArtists.size() << "artists";
    return m_data->m_allArtists;
}

QVariantList DataManager::allArtistVariantList()
{
    qCDebug(dmMusic) << "Getting all artist variant list";
    QVariantList allList;
    for (ArtistInfo info : allArtistInfos()) {
        allList.append(Utils::artistToVariantMap(info));
    }
    qCDebug(dmMusic) << "Returning" << allList.size() << "artist variants";
    return allList;
}

void DataManager::importMetas(const QStringList &urls, const QString &playlistHash, const bool &playFalg)
{
    qCDebug(dmMusic) << "Importing metas to playlist:" << playlistHash << "urls count:" << urls.size() << "playFlag:" << playFalg;
    QString targetPlaylistHash = playlistHash;
    if (targetPlaylistHash == "album" || targetPlaylistHash == "artist")
        targetPlaylistHash = "all";

    QSet<QString> metaHashs, playMetaHashs, allMetaHashs;
    for (MediaMeta &meta : m_data->m_allMetas) {
        allMetaHashs.insert(meta.hash);
    }
    if (!targetPlaylistHash.isEmpty()) {
        for (PlaylistInfo &playlist : m_data->m_allPlaylist) {
            if (playlist.uuid == targetPlaylistHash) {
                for (QString &hash : playlist.sortMetas) {
                    metaHashs.insert(hash);
                }
                break;
            }
        }
    }
    QString curPlaylistHash = targetPlaylistHash.isEmpty() ? "all" : targetPlaylistHash;
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
    qCDebug(dmMusic) << "Emitting import metas signal with" << metaHashs.size() << "existing metas";
    emit signalImportMetas(urls, metaHashs, importPlay, playMetaHashs, allMetaHashs, targetPlaylistHash, playFalg);
}

void DataManager::addMetasToPlayList(const QList<QString> &metaHash,
                                     const QString &playlistHash, const bool &addToPlay)
{
    qCDebug(dmMusic) << "Adding" << metaHash.size() << "metas to playlist:" << playlistHash << "addToPlay:" << addToPlay;
    if (playlistHash.isEmpty() || metaHash.isEmpty()) {
        qCWarning(dmMusic) << "Invalid parameters: playlistHash or metaHash is empty";
        return;
    }

    int index = playlistIndexFromHash(playlistHash);
    if (index < 0 || index >= m_data->m_allPlaylist.size()) {
        qCWarning(dmMusic) << "Invalid playlist index for hash:" << playlistHash;
        return;
    }

    PlaylistInfo &curPlaylist = m_data->m_allPlaylist[index];
    PlaylistInfo &favPlaylist = m_data->m_allPlaylist[playlistIndexFromHash("fav")];

    QSet<QString> allPlaylistHashs;
    if (m_data->m_currentHash == playlistHash && playlistHash != "play") {
        qCDebug(dmMusic) << "Adding metas to current playlist";
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
                qCDebug(dmMusic) << "Added meta" << meta.title << "to playlists:" << playlistHashs;
                emit signalAddOneMeta(playlistHashs, meta, addToPlay);
            }
        }
    } else {
        qCDebug(dmMusic) << "Adding metas to custom playlist";
        for (const QString &hash : metaHash) {
            if (!curPlaylist.sortMetas.contains(hash)) {
                curPlaylist.sortMetas.append(hash);
                QStringList playlistHashs;
                playlistHashs << playlistHash;
                allPlaylistHashs << playlistHash;
                MediaMeta meta = metaFromHash(hash);
                if (playlistHash == "fav" || favPlaylist.sortMetas.contains(meta.hash))
                    meta.favourite = true;
                qCDebug(dmMusic) << "Added meta" << meta.title << "to playlist:" << playlistHash;
                emit signalAddOneMeta(playlistHashs, meta, addToPlay);
            }
        }
    }
    curPlaylist.sortCustomMetas = curPlaylist.sortMetas;
    qCDebug(dmMusic) << "Finished adding metas to playlists:" << allPlaylistHashs;
    if (!allPlaylistHashs.isEmpty()) m_data->m_dirty = true;  // real membership change
    emit signalAddMetaFinished(allPlaylistHashs.values());
    qCDebug(dmMusic) << "Adding metas to playlist finished";
}

void DataManager::addMetasToPlayList(const QList<MediaMeta> &metas, const QString &playlistHash, const bool &addToPlay)
{
    qCDebug(dmMusic) << "Adding" << metas.size() << "metas to playlist:" << playlistHash << "addToPlay:" << addToPlay;
    if (playlistHash.isEmpty() || metas.isEmpty()) {
        qCWarning(dmMusic) << "Invalid parameters: playlistHash or metas is empty";
        return;
    }

    int index = playlistIndexFromHash(playlistHash);
    if (index < 0 || index >= m_data->m_allPlaylist.size()) {
        qCWarning(dmMusic) << "Invalid playlist index for hash:" << playlistHash;
        return;
    }

    PlaylistInfo &curPlaylist = m_data->m_allPlaylist[index];
    PlaylistInfo &favPlaylist = m_data->m_allPlaylist[playlistIndexFromHash("fav")];

    QSet<QString> allPlaylistHashs;
    if (m_data->m_currentHash == playlistHash && playlistHash != "play") {
        qCDebug(dmMusic) << "Adding metas to current playlist";
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
        qCDebug(dmMusic) << "Adding metas to custom playlist";
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
    qCDebug(dmMusic) << "Finished adding metas to playlists:" << allPlaylistHashs;
    if (!allPlaylistHashs.isEmpty()) m_data->m_dirty = true;  // real membership change
    emit signalAddMetaFinished(allPlaylistHashs.values());
    qCDebug(dmMusic) << "Adding metas to playlist finished";
}

void DataManager::clearPlayList(const QString &playlistHash, const bool &addToPlay)
{
    qCDebug(dmMusic) << "Clearing playlist:" << playlistHash << "addToPlay:" << addToPlay;
    QString curHash = playlistHash.isEmpty() ? "play" : playlistHash;
    int index = playlistIndexFromHash(curHash);
    if (index < 0 || index >= m_data->m_allPlaylist.size()) {
        qCWarning(dmMusic) << "Invalid playlist index for hash:" << curHash;
        return;
    }

    PlaylistInfo &curPlaylist = m_data->m_allPlaylist[index];
    if (curPlaylist.sortMetas.isEmpty() && curPlaylist.sortCustomMetas.isEmpty()) {
        qCDebug(dmMusic) << "Playlist already empty:" << curHash;
        emit signalDeleteFinished(QStringList() << playlistHash);
        return;  // no real change, not dirty
    }
    curPlaylist.sortMetas.clear();
    curPlaylist.sortCustomMetas.clear();
    m_data->m_dirty = true;  // real membership change
    qCDebug(dmMusic) << "Cleared playlist:" << curHash;
    emit signalDeleteFinished(QStringList() << playlistHash);
}

void DataManager::removeFromPlayList(const QStringList listToDel, const QString &playlistHash,
                                     bool delFlag)
{
    qCDebug(dmMusic) << "Removing" << listToDel.size() << "metas from playlist:" << playlistHash << "delFlag:" << delFlag;
    QString curHash = playlistHash.isEmpty() ? "play" : playlistHash;
    QStringList allHashs;
    if (playlistHash != "all" && playlistHash != "album" && playlistHash != "artist" && !delFlag) {
        qCDebug(dmMusic) << "not all & not album & not artist & not delFlag";
        int index = playlistIndexFromHash(curHash);
        if (index < 0 || index >= m_data->m_allPlaylist.size()) {
            qCWarning(dmMusic) << "Invalid playlist index for hash:" << curHash;
            return;
        }

        PlaylistInfo &curPlaylist = m_data->m_allPlaylist[index];
        for (const QString &hash : listToDel) {
            int curIndex = curPlaylist.sortMetas.indexOf(hash);
            if (curIndex >= 0 && curIndex < curPlaylist.sortMetas.size()) {
                curPlaylist.sortMetas.removeAt(curIndex);
                curPlaylist.sortCustomMetas.removeOne(hash);
                QStringList playlistHashs;
                playlistHashs << playlistHash;
                if (!allHashs.contains(hash)){
                    allHashs << hash;
                }
                qCDebug(dmMusic) << "Removed meta" << hash << "from playlist:" << playlistHash;
                emit signalDeleteOneMeta(playlistHashs, hash, true);
            }
        }
    } else {
        qCDebug(dmMusic) << "Removing metas from custom playlist";
        for (const QString &hash : listToDel) {
            QStringList playlistHashs;
            for (PlaylistInfo &playlist : m_data->m_allPlaylist) {
                int curIndex = playlist.sortMetas.indexOf(hash);
                if (curIndex >= 0 && curIndex < playlist.sortMetas.size()) {
                    playlist.sortMetas.removeAt(curIndex);
                    if (curIndex >= 0 && curIndex < playlist.sortCustomMetas.size())
                        playlist.sortCustomMetas.removeAt(curIndex);
                    playlistHashs << playlist.uuid;
                    if (!allHashs.contains(hash)){
                        allHashs << hash;
                    }
                }
            }
            if (!playlistHashs.isEmpty()) {
                DMusic::MediaMeta meta = metaFromHash(hash);
                if (delFlag) {
                    qCDebug(dmMusic) << "Deleting file:" << meta.localPath;
                    QFile::remove(meta.localPath);
                }
                deleteMetaFromAllMetas(QStringList() << meta.hash);
                deleteMetaFromAlbum(meta.hash, meta.album);
                deleteMetaFromArtist(meta.hash, meta.artist);
                qCDebug(dmMusic) << "Removed meta" << meta.title << "from playlists:" << playlistHashs;
                emit signalDeleteOneMeta(playlistHashs, hash, true);
            }
        }
    }
    if (!allHashs.isEmpty()){
        m_data->m_dirty = true;  // real membership change
        qCDebug(dmMusic) << "Finished removing metas:" << allHashs;
        emit signalDeleteFinished(allHashs);
    }
    qCDebug(dmMusic) << "Removing metas finished";
}

bool DataManager::moveMetasPlayList(const QStringList &metaHashs, const QString &playlistHash, const QString &nextHash)
{
    qCDebug(dmMusic) << "Moving" << metaHashs.size() << "metas in playlist:" << playlistHash << "before:" << nextHash;
    if (playlistHash == "all" || playlistHash == "album" || playlistHash == "artist") {
        qCWarning(dmMusic) << "Cannot move metas in system playlists";
        return false;
    }
    if (nextHash.isEmpty() && metaHashs.contains(nextHash)) {
        qCWarning(dmMusic) << "Invalid nextHash parameter";
        return false;
    }

    QString curHash = playlistHash.isEmpty() ? "play" : playlistHash;
    int index = playlistIndexFromHash(curHash);
    int customIndex = index;
    if (index < 0 || index >= m_data->m_allPlaylist.size()) {
        qCWarning(dmMusic) << "Invalid playlist index for hash:" << curHash;
        return false;
    }

    PlaylistInfo &curPlaylist = m_data->m_allPlaylist[index];
    if (curPlaylist.sortType != DmGlobal::SortByCustomASC
            && curPlaylist.sortType != DmGlobal::SortByCustomDES) {
        qCWarning(dmMusic) << "Cannot move metas in non-custom sorted playlist";
        return false;
    }

    QStringList curMetas;
    index = -1;
    for (int i = curPlaylist.sortMetas.size() - 1; i >= 0; i--) {
        if (metaHashs.contains(curPlaylist.sortMetas[i])) {
            curMetas.insert(0, curPlaylist.sortMetas[i]);
            curPlaylist.sortMetas.removeAt(i);
        }
    }
    if (curMetas.isEmpty()) {
        qCWarning(dmMusic) << "No metas to move";
        return false;
    }

    index = curPlaylist.sortMetas.size() - 1;
    if (!nextHash.isEmpty()) {
        qCDebug(dmMusic) << "Moving metas before" << nextHash;
        for (int i = curPlaylist.sortMetas.size() - 1; i >= 0; i--) {
            if (curPlaylist.sortMetas[i]  == nextHash) {
                index = i;
                break;
            }
        }
    }
    if (index == curPlaylist.sortMetas.size() - 1) {
        qCDebug(dmMusic) << "Moving metas to end of playlist";
        curPlaylist.sortMetas += curMetas;
    } else {
        qCDebug(dmMusic) << "Moving metas before" << index;
        for (int i = curMetas.size() - 1; i >= 0; i--) {
            curPlaylist.sortMetas.insert(index, curMetas[i]);
        }
    }

    m_data->m_allPlaylist[customIndex].sortCustomMetas.clear();
    m_data->m_allPlaylist[customIndex].sortCustomMetas = curPlaylist.sortMetas;
    m_data->m_dirty = true;  // real order change
    qCDebug(dmMusic) << "Successfully moved" << curMetas.size() << "metas in playlist:" << playlistHash;
    return true;
}

bool DataManager::isExistMeta()
{
    bool exist = !m_data->m_allMetas.isEmpty();
    qCDebug(dmMusic) << "Checking existence of metas:" << exist;
    return exist;
}

PlaylistInfo DataManager::addPlayList(const QString &name)
{
    qCDebug(dmMusic) << "Adding new playlist with name:" << name;
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
        qCDebug(dmMusic) << "Name already exists, using modified name:" << curName;
    }

    PlaylistInfo playlistMeta;
    playlistMeta.uuid = QUuid::createUuid().toString().remove("{").remove("}").remove("-");
    playlistMeta.displayName = curName;
    playlistMeta.sortID = (uint)m_data->m_allPlaylist.size() + 1;
    playlistMeta.sortType = DmGlobal::SortByCustomASC;
    m_data->m_allPlaylist << playlistMeta;

    qCDebug(dmMusic) << "Created new playlist with UUID:" << playlistMeta.uuid << "name:" << curName;
    m_data->m_dirty = true;  // new persistent playlist created
    return playlistMeta;
}

void DataManager::sortPlaylist(const int &type, const QString &hash, bool signalFlag)
{
    qCDebug(dmMusic) << "Sorting playlist:" << hash << "type:" << type << "signalFlag:" << signalFlag;
    int index = playlistIndexFromHash(hash);
    if (index < 0 || index >= m_data->m_allPlaylist.size()) {
        qCWarning(dmMusic) << "Invalid playlist index for hash:" << hash;
        return;
    }
    PlaylistInfo &playlistMeta = m_data->m_allPlaylist[index];

    int sortType = DmGlobal::SortByAddTimeASC;
    int oldSortType = playlistMeta.sortType;  // capture before mutation for dirty check
    const bool trackSortMetasChange = playlistMeta.saveFalg
                                      && playlistMeta.uuid != "album"
                                      && playlistMeta.uuid != "artist";
    const QStringList oldSortMetas = trackSortMetasChange ? playlistMeta.sortMetas : QStringList();
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
            qCDebug(dmMusic) << "Sorting by add time, new sort type:" << sortType;
            break;
        }
        case DmGlobal::SortByTitle: {
            if (playlistMeta.sortType == DmGlobal::SortByTitleASC) {
                sortType = DmGlobal::SortByTitleDES;
            } else {
                sortType = DmGlobal::SortByTitleASC;
            }
            qCDebug(dmMusic) << "Sorting by title, new sort type:" << sortType;
            break;
        }
        case DmGlobal::SortByArtist: {
            if (playlistMeta.sortType == DmGlobal::SortByArtistASC) {
                sortType = DmGlobal::SortByArtistDES;
            } else {
                sortType = DmGlobal::SortByArtistASC;
            }
            qCDebug(dmMusic) << "Sorting by artist, new sort type:" << sortType;
            break;
        }
        case DmGlobal::SortByAblum: {
            if (playlistMeta.sortType == DmGlobal::SortByAblumASC) {
                sortType = DmGlobal::SortByAblumDES;
            } else {
                sortType = DmGlobal::SortByAblumASC;
            }
            qCDebug(dmMusic) << "Sorting by album, new sort type:" << sortType;
            break;
        }
        case DmGlobal::SortByCustom: {
            sortType = DmGlobal::SortByCustomASC;
            playlistMeta.sortType = sortType;
            qCDebug(dmMusic) << "Using custom sort type:" << sortType;
            emit signalPlaylistSortChanged(playlistMeta.uuid);
            break;
        }
        default:
            qCDebug(dmMusic) << "Using default sort type:" << sortType;
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
        qCDebug(dmMusic) << "Sorting" << allMetas.size() << "metas in playlist";

        switch (sortType) {
        case DmGlobal::SortByAddTimeASC: {
            std::sort(allMetas.begin(), allMetas.end(), moreThanTimestampASC);
            qCDebug(dmMusic) << "Sorted by timestamp ascending";
        }
        break;
        case DmGlobal::SortByTitleASC: {
            std::sort(allMetas.begin(), allMetas.end(), moreThanTitleASC);
            qCDebug(dmMusic) << "Sorted by title ascending";
        }
        break;
        case DmGlobal::SortByArtistASC: {
            std::sort(allMetas.begin(), allMetas.end(), moreThanArtistASC);
            qCDebug(dmMusic) << "Sorted by artist ascending";
        }
        break;
        case DmGlobal::SortByAblumASC: {
            std::sort(allMetas.begin(), allMetas.end(), moreThanAblumASC);
            qCDebug(dmMusic) << "Sorted by album ascending";
        }
        break;
        case DmGlobal::SortByAddTimeDES: {
            std::sort(allMetas.begin(), allMetas.end(), moreThanTimestampDES);
            qCDebug(dmMusic) << "Sorted by timestamp descending";
        }
        break;
        case DmGlobal::SortByTitleDES: {
            std::sort(allMetas.begin(), allMetas.end(), moreThanTitleDES);
            qCDebug(dmMusic) << "Sorted by title descending";
        }
        break;
        case DmGlobal::SortByArtistDES: {
            std::sort(allMetas.begin(), allMetas.end(), moreThanArtistDES);
            qCDebug(dmMusic) << "Sorted by artist descending";
        }
        break;
        case DmGlobal::SortByAblumDES: {
            std::sort(allMetas.begin(), allMetas.end(), moreThanAblumDES);
            qCDebug(dmMusic) << "Sorted by album descending";
        }
        break;
        default:
            sortFlag = false;
            qCDebug(dmMusic) << "No sorting performed";
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
            if (signalFlag) {
                qCDebug(dmMusic) << "Emitting playlist sort changed signal for:" << playlistMeta.uuid;
                emit signalPlaylistSortChanged(playlistMeta.uuid);
            }
        }
    } else {
        if (playlistMeta.uuid == "album") {
            qCDebug(dmMusic) << "Sorting album list";
            bool sortFlag = true;
            switch (sortType) {
            case DmGlobal::SortByAddTimeASC: {
                std::sort(m_data->m_allAlbums.begin(), m_data->m_allAlbums.end(), moreThanAlbumTimestampASC);
                qCDebug(dmMusic) << "Sorted albums by timestamp ascending";
            }
            break;
            case DmGlobal::SortByAblumASC: {
                std::sort(m_data->m_allAlbums.begin(), m_data->m_allAlbums.end(), moreThanAlbumTitleASC);
                qCDebug(dmMusic) << "Sorted albums by title ascending";
            }
            break;
            case DmGlobal::SortByAddTimeDES: {
                std::sort(m_data->m_allAlbums.begin(), m_data->m_allAlbums.end(), moreThanAlbumTimestampDES);
                qCDebug(dmMusic) << "Sorted albums by timestamp descending";
            }
            break;
            case DmGlobal::SortByAblumDES: {
                std::sort(m_data->m_allAlbums.begin(), m_data->m_allAlbums.end(), moreThanAlbumTitleDES);
                qCDebug(dmMusic) << "Sorted albums by title descending";
            }
            break;
            default:
                sortFlag = false;
                qCDebug(dmMusic) << "No album sorting performed";
                break;
            }
            if (sortFlag && signalFlag) {
                qCDebug(dmMusic) << "Emitting album sort changed signal";
                emit signalPlaylistSortChanged(playlistMeta.uuid);
            }
        } else if (playlistMeta.uuid == "artist") {
            qCDebug(dmMusic) << "Sorting artist list";
            bool sortFlag = true;
            switch (sortType) {
            case DmGlobal::SortByAddTimeASC: {
                std::sort(m_data->m_allArtists.begin(), m_data->m_allArtists.end(), moreThanArtistTimestampASC);
                qCDebug(dmMusic) << "Sorted artists by timestamp ascending";
            }
            break;
            case DmGlobal::SortByArtistASC: {
                std::sort(m_data->m_allArtists.begin(), m_data->m_allArtists.end(), moreThanArtistTitleASC);
                qCDebug(dmMusic) << "Sorted artists by name ascending";
            }
            break;
            case DmGlobal::SortByAddTimeDES: {
                std::sort(m_data->m_allArtists.begin(), m_data->m_allArtists.end(), moreThanArtistTimestampDES);
                qCDebug(dmMusic) << "Sorted artists by timestamp descending";
            }
            break;
            case DmGlobal::SortByArtistDES: {
                std::sort(m_data->m_allArtists.begin(), m_data->m_allArtists.end(), moreThanArtistTitleDES);
                qCDebug(dmMusic) << "Sorted artists by name descending";
            }
            break;
            default:
                sortFlag = false;
                qCDebug(dmMusic) << "No artist sorting performed";
                break;
            }
            if (sortFlag && signalFlag) {
                qCDebug(dmMusic) << "Emitting artist sort changed signal";
                emit signalPlaylistSortChanged(playlistMeta.uuid);
            }
        } else if (playlistMeta.uuid == "albumResult") {
            qCDebug(dmMusic) << "Sorting album search results";
            bool sortFlag = true;
            QList<DMusic::AlbumInfo> searchAlbums = searchedAlbumInfos();
            switch (sortType) {
            case DmGlobal::SortByAddTimeASC: {
                std::sort(searchAlbums.begin(), searchAlbums.end(), moreThanAlbumTimestampASC);
                qCDebug(dmMusic) << "Sorted album results by timestamp ascending";
            }
            break;
            case DmGlobal::SortByAblumASC: {
                std::sort(searchAlbums.begin(), searchAlbums.end(), moreThanAlbumTitleASC);
                qCDebug(dmMusic) << "Sorted album results by title ascending";
            }
            break;
            case DmGlobal::SortByAddTimeDES: {
                std::sort(searchAlbums.begin(), searchAlbums.end(), moreThanAlbumTimestampDES);
                qCDebug(dmMusic) << "Sorted album results by timestamp descending";
            }
            break;
            case DmGlobal::SortByAblumDES: {
                std::sort(searchAlbums.begin(), searchAlbums.end(), moreThanAlbumTitleDES);
                qCDebug(dmMusic) << "Sorted album results by title descending";
            }
            break;
            default:
                sortFlag = false;
                qCDebug(dmMusic) << "No album result sorting performed";
                break;
            }
            if (sortFlag && signalFlag) {
                m_data->m_searchAlbums.clear();
                for (const DMusic::AlbumInfo &info : searchAlbums) {
                    m_data->m_searchAlbums.append(info.name);
                }
                qCDebug(dmMusic) << "Updated sorted album search results, count:" << m_data->m_searchAlbums.size();
                emit signalPlaylistSortChanged(playlistMeta.uuid);
            }
        } else {
            qCDebug(dmMusic) << "Sorting artist search results";
            bool sortFlag = true;
            QList<DMusic::ArtistInfo> searchArtists = searchedArtistInfos();
            switch (sortType) {
            case DmGlobal::SortByAddTimeASC: {
                std::sort(searchArtists.begin(), searchArtists.end(), moreThanArtistTimestampASC);
                qCDebug(dmMusic) << "Sorted artist results by timestamp ascending";
            }
            break;
            case DmGlobal::SortByArtistASC: {
                std::sort(searchArtists.begin(), searchArtists.end(), moreThanArtistTitleASC);
                qCDebug(dmMusic) << "Sorted artist results by name ascending";
            }
            break;
            case DmGlobal::SortByAddTimeDES: {
                std::sort(searchArtists.begin(), searchArtists.end(), moreThanArtistTimestampDES);
                qCDebug(dmMusic) << "Sorted artist results by timestamp descending";
            }
            break;
            case DmGlobal::SortByArtistDES: {
                std::sort(searchArtists.begin(), searchArtists.end(), moreThanArtistTitleDES);
                qCDebug(dmMusic) << "Sorted artist results by name descending";
            }
            break;
            default:
                sortFlag = false;
                qCDebug(dmMusic) << "No artist result sorting performed";
                break;
            }
            if (sortFlag && signalFlag) {
                m_data->m_searchArtists.clear();
                for (const DMusic::ArtistInfo &info : searchArtists) {
                    m_data->m_searchArtists.append(info.name);
                }
                qCDebug(dmMusic) << "Updated sorted artist search results, count:" << m_data->m_searchArtists.size();
                emit signalPlaylistSortChanged(playlistMeta.uuid);
            }
        }
    }
    if (playlistMeta.saveFalg
            && (oldSortType != playlistMeta.sortType
                || (trackSortMetasChange && oldSortMetas != playlistMeta.sortMetas))) {
        m_data->m_dirty = true;
    }
    // sorting m_allAlbums/m_allArtists invalidates name->index; rebuild both
    rebuildAlbumNameIndex();
    rebuildArtistNameIndex();
}

bool DataManager::deletePlaylist(QString playlistHash)
{
    qCDebug(dmMusic) << "Deleting playlist:" << playlistHash;
    int index = playlistIndexFromHash(playlistHash);
    if (index < 0 || index >= m_data->m_allPlaylist.size()) {
        qCWarning(dmMusic) << "Invalid playlist index for hash:" << playlistHash;
        return false;
    }
    m_data->m_allPlaylist.removeAt(index);
    if (currentPlayliHash() == playlistHash) {
        qCDebug(dmMusic) << "Resetting current playlist hash as deleted playlist was current";
        setCurrentPlayliHash("");
    }
    m_data->m_dirty = true;  // persistent playlist removed
    qCDebug(dmMusic) << "Successfully deleted playlist:" << playlistHash;
    return true;
}

bool DataManager::renamePlaylist(const QString &name, const QString &playlistHash)
{
    qCDebug(dmMusic) << "Renaming playlist" << playlistHash << "to:" << name;
    int index = playlistIndexFromHash(playlistHash);
    if (index < 0 || index >= m_data->m_allPlaylist.size()) {
        qCWarning(dmMusic) << "Invalid playlist index for hash:" << playlistHash;
        return false;
    }

    QStringList existNames;
    for (PlaylistInfo &playlist : m_data->m_allPlaylist) {
        existNames.append(playlist.displayName);
    }
    if (existNames.contains(name)) {
        qCWarning(dmMusic) << "Playlist name already exists:" << name;
        return false;
    }

    m_data->m_allPlaylist[index].displayName = name;
    m_data->m_dirty = true;  // playlist display name changed
    qCDebug(dmMusic) << "Successfully renamed playlist" << playlistHash << "to:" << name;
    return true;
}

void DataManager::movePlaylist(const QString &hash, const QString &nextHash)
{
    qCDebug(dmMusic) << "Moving playlist" << hash << "before:" << nextHash;
    int index = playlistIndexFromHash(hash);
    if (index < 0 || index >= m_data->m_allPlaylist.size()) {
        qCWarning(dmMusic) << "Invalid playlist index for hash:" << hash;
        return;
    }

    DMusic::PlaylistInfo playlist = m_data->m_allPlaylist[index];
    m_data->m_allPlaylist.removeAt(index);

    int nextIndex = playlistIndexFromHash(nextHash);
    if (nextIndex < 0 || nextIndex >= m_data->m_allPlaylist.size()) {
        m_data->m_allPlaylist.append(playlist);
        qCDebug(dmMusic) << "Appended playlist to end as next index was invalid";
    } else {
        m_data->m_allPlaylist.insert(nextIndex, playlist);
        qCDebug(dmMusic) << "Inserted playlist at index:" << nextIndex;
    }

    uint num = 12;
    for (int i = 0; i < m_data->m_allPlaylist.size(); ++i) {
        if (m_data->m_allPlaylist[i].sortID > 12) {
            m_data->m_allPlaylist[i].sortID = (++num);
        }
    }
    m_data->m_dirty = true;  // playlist order changed
    qCDebug(dmMusic) << "Updated sort IDs for all playlists";
}

bool DataManager::isExistMeta(const QString &metaHash, const QString &playlistHash)
{
    qCDebug(dmMusic) << "Checking if meta" << metaHash << "exists in playlist:" << playlistHash;
    int index = playlistIndexFromHash(playlistHash);
    if (index < 0 || index >= m_data->m_allPlaylist.size()) {
        qCWarning(dmMusic) << "Invalid playlist index for hash:" << playlistHash;
        return false;
    }

    bool exists = m_data->m_allPlaylist[index].sortMetas.contains(metaHash);
    qCDebug(dmMusic) << "Meta exists in playlist:" << exists;
    return exists;
}

void DataManager::updateMetaCodec(const MediaMeta &meta)
{
    qCDebug(dmMusic) << "Updating meta codec for:" << meta.hash;
    int index = metaIndexFromHash(meta.hash);
    if (index < 0 || index >= m_data->m_allMetas.size()) {
        qCWarning(dmMusic) << "Invalid meta index for hash:" << meta.hash;
        return;
    }
    m_data->m_allMetas[index] = meta;
    m_data->m_dirty = true;  // meta content updated

    QString preAlbum, preArtist;
    for (int i = 0; i < m_data->m_allAlbums.size(); ++i) {
        if (m_data->m_allAlbums[i].musicinfos.contains(meta.hash)) {
            bool existFla = false;
            preAlbum = m_data->m_allAlbums[i].name;
            for (int j = 0; j < m_data->m_allAlbums.size(); ++j) {
                if (m_data->m_allAlbums[j].name == meta.album) {
                    m_data->m_allAlbums[j].musicinfos[meta.hash] = meta;
                    existFla = true;
                    qCDebug(dmMusic) << "Updated meta in existing album:" << meta.album;
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
                qCDebug(dmMusic) << "Created new album for meta:" << meta.album;
            }
            m_data->m_allAlbums[i].musicinfos.remove(meta.hash);
            // 踢出空专辑
            if (m_data->m_allAlbums[i].musicinfos.isEmpty()) {
                m_data->m_allAlbums.removeAt(i);
                qCDebug(dmMusic) << "Removed empty album:" << preAlbum;
            }
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
                    qCDebug(dmMusic) << "Updated meta in existing artist:" << meta.artist;
                }
            }
            if (!existFla) {
                ArtistInfo artist;
                artist.pinyin = meta.pinyinArtist;
                artist.name = meta.artist;
                artist.musicinfos[meta.hash] = meta;
                artist.timestamp = meta.timestamp;
                m_data->m_allArtists.append(artist);
                qCDebug(dmMusic) << "Created new artist for meta:" << meta.artist;
            }
            m_data->m_allArtists[i].musicinfos.remove(meta.hash);
            // 踢出空艺人
            if (m_data->m_allArtists[i].musicinfos.isEmpty()) {
                m_data->m_allArtists.removeAt(i);
                qCDebug(dmMusic) << "Removed empty artist:" << preArtist;
            }
            break;
        }
    }
    qCDebug(dmMusic) << "Finished updating meta codec, emitting signal";
    rebuildAlbumNameIndex();    // meta may have moved to new album/artist
    rebuildArtistNameIndex();
    emit signalUpdatedMetaCodec(meta, preAlbum, preArtist);
}

void DataManager::quickSearchText(const QString &text, QStringList &metaTitles,
                                  QList<QPair<QString, QString> > &albums, QList<QPair<QString, QString> > &artists)
{
    qCDebug(dmMusic) << "Starting quick search for text:" << text;
    // 查找名称
    for (const MediaMeta &meta : m_data->m_allMetas) {
        if (Utils::containsStr(text, meta.title)) metaTitles.append(meta.title);
        if (metaTitles.size() > 4) break;
    }
    qCDebug(dmMusic) << "Found" << metaTitles.size() << "matching titles";

    for (const DMusic::AlbumInfo &album : allAlbumInfos()) {
        if (!album.musicinfos.isEmpty() && Utils::containsStr(text, album.name))
            albums.append(qMakePair(album.name, album.musicinfos.first().coverUrl));
        if (albums.size() > 2) break;
    }
    qCDebug(dmMusic) << "Found" << albums.size() << "matching albums";

    for (const DMusic::ArtistInfo &artist : allArtistInfos()) {
        if (!artist.musicinfos.isEmpty() && Utils::containsStr(text, artist.name))
            artists.append(qMakePair(artist.name, artist.musicinfos.first().coverUrl));
        if (artists.size() > 2) break;
    }
    qCDebug(dmMusic) << "Found" << artists.size() << "matching artists";
}

void DataManager::searchText(const QString &text, QList<MediaMeta> &metas,
                             QList<AlbumInfo> &albums, QList<ArtistInfo> &artists, const QString &type)
{
    qCDebug(dmMusic) << "Starting detailed search for text:" << text << "type:" << type;
    // 查找名称
    m_data->m_searchMetas.clear();
    m_data->m_searchAlbums.clear();
    m_data->m_searchArtists.clear();

    if (type == "album") {
        qCDebug(dmMusic) << "Searching in album mode";
        QSet<QString> artistNames;
        for (const DMusic::AlbumInfo &album : allAlbumInfos()) {
            if (!album.musicinfos.isEmpty() && Utils::containsStr(text, album.name)) {
                albums.append(album);
                qCDebug(dmMusic) << "Found matching album:" << album.name;

                for (QString metaHash : album.musicinfos.keys()) {
                    if (!m_data->m_searchMetas.contains(metaHash)) {
                        metas.append(album.musicinfos[metaHash]);
                        m_data->m_searchMetas.append(metaHash);
                        artistNames.insert(album.musicinfos[metaHash].artist);
                    }
                }
            }
        }
        qCDebug(dmMusic) << "Found" << albums.size() << "albums and" << metas.size() << "tracks";

        for (const DMusic::ArtistInfo &artist : allArtistInfos()) {
            if (!artist.musicinfos.isEmpty() && artistNames.contains(artist.name)) {
                artists.append(artist);
            }
        }
        qCDebug(dmMusic) << "Found" << artists.size() << "related artists";

    } else if (type == "artist") {
        qCDebug(dmMusic) << "Searching in artist mode";
        QSet<QString> albumNames;
        for (const DMusic::ArtistInfo &artist : allArtistInfos()) {
            if (!artist.musicinfos.isEmpty() && Utils::containsStr(text, artist.name)) {
                artists.append(artist);
                qCDebug(dmMusic) << "Found matching artist:" << artist.name;

                for (QString metaHash : artist.musicinfos.keys()) {
                    if (!m_data->m_searchMetas.contains(metaHash)) {
                        metas.append(artist.musicinfos[metaHash]);
                        m_data->m_searchMetas.append(metaHash);
                        albumNames.insert(artist.musicinfos[metaHash].album);
                    }
                }
            }
        }
        qCDebug(dmMusic) << "Found" << artists.size() << "artists and" << metas.size() << "tracks";

        for (const DMusic::AlbumInfo &album : allAlbumInfos()) {
            if (!album.musicinfos.isEmpty() && albumNames.contains(album.name)) {
                albums.append(album);
            }
        }
        qCDebug(dmMusic) << "Found" << albums.size() << "related albums";

    } else if (type == "music") {
        qCDebug(dmMusic) << "Searching in music mode";
        QSet<QString> albumNames, artistNames;
        for (const MediaMeta &meta : m_data->m_allMetas) {
            if (Utils::containsStr(text, meta.title)) {
                m_data->m_searchMetas.append(meta.hash);
                metas.append(meta);
                albumNames.insert(meta.album);
                artistNames.insert(meta.artist);
                qCDebug(dmMusic) << "Found matching track:" << meta.title;
            }
        }
        qCDebug(dmMusic) << "Found" << metas.size() << "matching tracks";

        for (const DMusic::ArtistInfo &artist : allArtistInfos()) {
            if (!artist.musicinfos.isEmpty() && artistNames.contains(artist.name))
                artists.append(artist);
        }
        qCDebug(dmMusic) << "Found" << artists.size() << "related artists";

        for (const DMusic::AlbumInfo &album : allAlbumInfos()) {
            if (!album.musicinfos.isEmpty() && albumNames.contains(album.name)) {
                albums.append(album);
            }
        }
        qCDebug(dmMusic) << "Found" << albums.size() << "related albums";

    } else {
        qCDebug(dmMusic) << "Searching in all modes";
        QSet<QString> albumNames, artistNames;
        for (const MediaMeta &meta : m_data->m_allMetas) {
            if (Utils::containsStr(text, meta.title)) {
                m_data->m_searchMetas.append(meta.hash);
                metas.append(meta);
                albumNames.insert(meta.album);
                artistNames.insert(meta.artist);
            }
        }
        qCDebug(dmMusic) << "Found" << metas.size() << "matching tracks";

        // Search in albums
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
        qCDebug(dmMusic) << "Found" << artists.size() << "related artists";

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
        qCDebug(dmMusic) << "Found" << albums.size() << "related albums";

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
    qCDebug(dmMusic) << "Getting searched album infos, count:" << m_data->m_searchAlbums.size();
    QList<AlbumInfo> searchedInfos;
    for (QString name : m_data->m_searchAlbums) {
        for (const DMusic::AlbumInfo &album : allAlbumInfos()) {
            if (!album.musicinfos.isEmpty() && album.name == name) {
                searchedInfos.append(album);
                qCDebug(dmMusic) << "Found album:" << album.name;
                break;
            }
        }
    }
    return searchedInfos;
}

QList<ArtistInfo> DataManager::searchedArtistInfos()
{
    qCDebug(dmMusic) << "Getting searched artist infos, count:" << m_data->m_searchArtists.size();
    QList<ArtistInfo> searchedInfos;
    for (QString name : m_data->m_searchArtists) {
        for (const DMusic::ArtistInfo &artist : allArtistInfos()) {
            if (!artist.musicinfos.isEmpty() && artist.name == name) {
                searchedInfos.append(artist);
                qCDebug(dmMusic) << "Found artist:" << artist.name;
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
    qCDebug(dmMusic) << "Syncing settings to storage";
    m_data->m_settings->sync();
}

void DataManager::resetToSettings()
{
    qCDebug(dmMusic) << "Resetting settings to defaults";
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
    qCDebug(dmMusic) << "Adding meta" << meta.title << "to playlists:" << playlistHashs;
    MediaMeta curMeta = playlistHashs.contains("all") ? meta : metaFromHash(meta.hash);
    
    // 检查 meta 是否已存在于 m_allMetas 中
    int existingIndex = metaIndexFromHash(curMeta.hash);
    if (existingIndex >= 0) {
        // 如果已存在，使用已存在的 meta（可能包含更完整的信息）
        curMeta = m_data->m_allMetas[existingIndex];
        qCDebug(dmMusic) << "Meta already exists in allMetas, using existing:" << curMeta.title;
    }
    
    for (PlaylistInfo &playlist : m_data->m_allPlaylist) {
        for (QString hash : playlistHashs) {
            if (hash == playlist.uuid) {
                if (hash == "all") {
                    // 检查是否已存在，避免重复添加
                    if (existingIndex < 0) {
                        if (!m_data->m_metaHashIndex.contains(curMeta.hash)) {
                            m_data->m_metaHashIndex.insert(curMeta.hash, m_data->m_allMetas.size());
                        }
                        m_data->m_allMetas.append(curMeta);
                        m_data->m_importedMetas.append(curMeta);
                        addMetaToAlbum(curMeta);
                        addMetaToArtist(curMeta);
                        m_data->m_dirty = true;  // new meta appended -> persistent model changed
                        qCDebug(dmMusic) << "Added meta to all collections";
                    } else {
                        qCDebug(dmMusic) << "Meta already exists in allMetas, skipping add to all:" << curMeta.title;
                    }
                }
                // 检查歌单中是否已存在，避免重复添加
                if (!playlist.sortMetas.contains(curMeta.hash)) {
                    playlist.sortMetas.append(curMeta.hash);
                    playlist.sortCustomMetas.append(curMeta.hash);
                    m_data->m_dirty = true;  // playlist membership changed
                    qCDebug(dmMusic) << "Added meta to playlist:" << playlist.displayName;
                } else {
                    qCDebug(dmMusic) << "Meta already exists in playlist:" << playlist.displayName << "hash:" << curMeta.hash;
                }
            }
        }
    }
    emit signalAddOneMeta(playlistHashs, curMeta, true);
}

void DataManager::slotMetaCoverReady(DMusic::MediaMeta meta)
{
    int index = metaIndexFromHash(meta.hash);
    if (index < 0) return;

    // 1. m_allMetas
    m_data->m_allMetas[index].coverUrl = meta.coverUrl;
    m_data->m_allMetas[index].hasimage = meta.hasimage;
    m_data->m_allMetas[index].lyricPath = meta.lyricPath;

    // 2. m_importedMetas (upsertMetasDB reads it; sync or it writes stale hasimage/coverUrl)
    for (auto &im : m_data->m_importedMetas) {
        if (im.hash == meta.hash) {
            im.coverUrl = meta.coverUrl;
            im.hasimage = meta.hasimage;
            im.lyricPath = meta.lyricPath;
            break;
        }
    }

    // 3. album/artist musicinfos copy (no top-level coverUrl field; UI derives from musicinfos)
    for (auto &alb : m_data->m_allAlbums) {
        if (alb.musicinfos.contains(meta.hash)) {
            alb.musicinfos[meta.hash] = m_data->m_allMetas[index];
        }
    }
    for (auto &art : m_data->m_allArtists) {
        if (art.musicinfos.contains(meta.hash)) {
            art.musicinfos[meta.hash] = m_data->m_allMetas[index];
        }
    }

    m_data->m_dirty = true;  // hasimage/coverUrl/lyricPath affect DB row
    emit signalMetaCoverReady(m_data->m_allMetas[index]);
}

void DataManager::slotCoverBatchFinished()
{
    // 导入成功提示已在 signalImportFinished 发出；这里仅在封面/歌词补齐后
    // 增量落库，避免 musicNew 先写入 stale coverUrl/hasimage/lyricPath。
    upsertMetasDB();
    emit signalCoverBatchFinished();
}

void DataManager::slotLazyLoadDatabase()
{
    qCDebug(dmMusic) << "Starting lazy load of database";
    loadMetasDB();
    loadPlaylistMetasDB();
    QStringList playlistHashs;
    for (PlaylistInfo &playlist : m_data->m_allPlaylist) {
        if (playlist.saveFalg)
            playlistHashs.append(playlist.uuid);
    }
    qCDebug(dmMusic) << "Lazy load complete, found" << playlistHashs.size() << "playlists";
    emit signalAddMetaFinished(playlistHashs);
}

void DataManager::initPlaylist()
{
    qCDebug(dmMusic) << "Initializing playlists";
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

    // DB 路径白名单（安全）：仅允许测试用 ":memory:"（内存库，无文件系统副作用）；
    // 空=默认 cachePath/mediameta.sqlite；其他任何路径一律拒绝并回退默认，
    // 防止路径遍历导致任意文件创建/覆盖。
    QString dbPath;
    if (m_data->m_dbPath == ":memory:") {
        dbPath = m_data->m_dbPath;
    } else if (m_data->m_dbPath.isEmpty()) {
        dbPath = DmGlobal::cachePath() + "/mediameta.sqlite";
    } else {
        qCWarning(dmMusic) << "Security: injected dbPath is not ':memory:' or empty, "
                              "rejected to prevent path traversal, falling back to default.";
        dbPath = DmGlobal::cachePath() + "/mediameta.sqlite";
    }
    qCDebug(dmMusic) << "Opening database at:" << dbPath;
    m_data->m_database = QSqlDatabase::addDatabase("QSQLITE");
    m_data->m_database.setDatabaseName(dbPath);
    if (!m_data->m_database.open()) {
        qCCritical(dmMusic) << "Failed to open database:" << m_data->m_database.lastError();
    } else {
        qCDebug(dmMusic) << "Successfully opened database";
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
        qCDebug(dmMusic) << "Adding hasimage column with default value 1";
        isExec &= query.exec(QString("ALTER TABLE \"musicNew\" ADD COLUMN \"hasimage\" INTEGER default \"%1\"")
                             .arg("1"));
    }

    // Create other tables
    qCDebug(dmMusic) << "Creating additional tables";
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

    if (!isExec) {
        qCCritical(dmMusic) << "Failed to create one or more tables";
    }

    // Initialize database
    if (!isPlaylistExistDB("album")) {
        qCDebug(dmMusic) << "First time initialization - creating default playlists";
        for (PlaylistInfo playlist : m_data->m_allPlaylist) {
            addPlaylistDB(playlist);
        }
    } else {
        qCDebug(dmMusic) << "Loading existing playlists";
        loadPlaylistDB();

        // 延迟加载
        if (allMusicCountDB() > FirstLoadCount && false) {
            qCDebug(dmMusic) << "Using delayed loading for large library";
            loadCurrentMetasDB();
            QTimer::singleShot(500, this, &DataManager::slotLazyLoadDatabase);
        } else {
            qCDebug(dmMusic) << "Loading entire library immediately";
            loadMetasDB();
            loadPlaylistMetasDB();
        }
    }
}
