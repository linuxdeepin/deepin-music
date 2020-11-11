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

#include "mediadatabase.h"

#include <QDebug>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QUuid>
#include <QDir>

#include <mediameta.h>

#include "util/global.h"
#include  "util/threadpool.h"
#include "medialibrary.h"

static bool createConnection()
{
    QDir cacheDir(Global::cacheDir());
    if (!cacheDir.exists()) {
        cacheDir.mkpath(".");
    }
    QString cachePath = Global::cacheDir() + "/mediameta.sqlite";
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(cachePath);

    if (!db.open()) {
        qCritical() << db.lastError()
                    << Global::cacheDir()
                    << cachePath;
        return false;
    }

    QSqlQuery query;
    query.exec("CREATE TABLE IF NOT EXISTS music (hash TEXT primary key not null, "
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
    query.exec("CREATE TABLE IF NOT EXISTS musicNew (hash TEXT primary key not null, "
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
               "cuepath VARCHAR(4096) )"
              );



    query.exec("CREATE TABLE IF NOT EXISTS ablum (id int primary key, "
               "name VARCHAR(20), localpath VARCHAR(4096), url VARCHAR(4096))");

    query.exec("CREATE TABLE IF NOT EXISTS artist (id int primary key, "
               "name VARCHAR(20))");

    query.exec("CREATE TABLE IF NOT EXISTS playlist (uuid TEXT primary key not null, "
               "displayname VARCHAR(4096), "
               "icon VARCHAR(256), readonly INTEGER, "
               "hide INTEGER, sort_type INTEGER, "
               "sort_id INTEGER, "
               "order_type INTEGER )");

    query.exec("CREATE TABLE IF NOT EXISTS info (uuid TEXT primary key not null, "
               "version INTEGER )");

    return true;
}

static const QString DatabaseUUID = "0fcbd091-2356-161c-9026-f49779f9c71c40";

int databaseVersion()
{
    QSqlQuery query;
    query.prepare("SELECT version FROM info where uuid = :uuid;");
    query.bindValue(":uuid", DatabaseUUID);
    if (!query.exec()) {
        qWarning() << query.lastError();
        return -1;
    }

    while (query.next()) {
        auto version =  query.value(0).toInt();
        return version;
    }
    return -1;
}

int updateDatabaseVersion(int version)
{
    QSqlQuery query;

    query.prepare("INSERT INTO info ("
                  "uuid, version "
                  ") "
                  "VALUES ("
                  ":uuid, :version "
                  ")");
    query.bindValue(":version", version);
    query.bindValue(":uuid", DatabaseUUID);
    query.exec();
    qWarning() << query.lastError();

    query.prepare("UPDATE info SET version = :version where uuid = :uuid; ");
    query.bindValue(":version", version);
    query.bindValue(":uuid", DatabaseUUID);

    if (!query.exec()) {
        qWarning() << query.lastError();
        return -1;
    }

    return version;
}

void megrateToVserion_0()
{
    // FIXME: remove old
    QSqlDatabase::database().transaction();
    QSqlQuery query;
    qWarning() << "sql upgrade with error:" << query.lastError().type();
    query.prepare("ALTER TABLE music ADD COLUMN cuepath VARCHAR(4096);");
    if (!query.exec()) {
        qWarning() << "sql upgrade with error:" << query.lastError().type();
    }

    query.prepare("ALTER TABLE music ADD COLUMN invalid INTEGER(32);");
    if (!query.exec()) {
        qWarning() << "sql upgrade with error:" << query.lastError().type();
    }

    query.prepare("ALTER TABLE music ADD COLUMN search_id VARCHAR(256);");
    if (!query.exec()) {
        qWarning() << "sql upgrade with error:" << query.lastError().type();
    }

    query.prepare("ALTER TABLE playlist ADD COLUMN order_type INTEGER(32);");
    if (!query.exec()) {
        qWarning() << "sql upgrade with error:" << query.lastError().type();
    }

    query.prepare("ALTER TABLE playlist ADD COLUMN sort_type INTEGER(32);");
    if (!query.exec()) {
        qWarning() << "sql upgrade with error:" << query.lastError().type();
    }

    query.prepare("ALTER TABLE playlist ADD COLUMN sort_id INTEGER(32);");
    if (!query.exec()) {
        qWarning() << "sql upgrade with error:" << query.lastError().type();
    }

    QStringList list;
    query.prepare("SELECT uuid FROM playlist;");
    if (!query.exec()) {
        qWarning() << "sql upgrade with error:" << query.lastError().type();
    }
    while (query.next()) {
        list <<  query.value(0).toString();
    }

    for (auto uuid : list) {
        auto sqlStr = QString("ALTER TABLE playlist_%1  ADD COLUMN sort_id INTEGER(32);").arg(uuid);
        query.prepare(sqlStr);
        if (!query.exec()) {
            qWarning() << "sql upgrade playlist with error:" << query.lastError().type();
        }
    }

    updateDatabaseVersion(0);
    QSqlDatabase::database().commit();
}

void megrateToVserion_1()
{
    // FIXME: remove old
    QSqlDatabase::database().transaction();
    QSqlQuery query;

    query.prepare("ALTER TABLE playlist ADD COLUMN sort_id INTEGER(32);");
    if (!query.exec()) {
        qWarning() << "sql upgrade with error:" << query.lastError().type();
    }

    updateDatabaseVersion(1);
    QSqlDatabase::database().commit();
}

typedef void (*MargeFunctionn)();

void margeDatabase()
{
    QMap<int, MargeFunctionn> margeFuncs;
    margeFuncs.insert(0, megrateToVserion_0);
    margeFuncs.insert(1, megrateToVserion_1);

    int currentVersion = databaseVersion();

    QList<int> sortVer = margeFuncs.keys();
    qSort(sortVer.begin(), sortVer.end());

    for (auto ver : sortVer) {
        if (ver > currentVersion) {
            margeFuncs.value(ver)();
        }
    }
}

MediaDatabase::MediaDatabase(QObject *parent) : QObject(parent), m_writer(nullptr)
{
}

void MediaDatabase::init()
{
    createConnection();
    margeDatabase();

    // sqlite must run in one thread!!!
    m_writer = new MediaDatabaseWriter;
    ThreadPool::instance()->moveToNewThread(m_writer);//将读写耗时操作放到子线程操作
    connect(this, &MediaDatabase::initWrter,
            m_writer, &MediaDatabaseWriter::initDataBase);
    Q_EMIT initWrter();
    bind();


    QSqlDatabase::database().transaction();
    PlaylistMeta playlistMeta;
    playlistMeta.uuid = "album";
    playlistMeta.displayName = "Album";
    playlistMeta.icon = "album";
    playlistMeta.readonly = true;
    playlistMeta.hide = false;
    playlistMeta.sortID = 1;
    if (!playlistExist("album")) {
        addPlaylist(playlistMeta);
    }

    playlistMeta.uuid = "artist";
    playlistMeta.displayName = "Artist";
    playlistMeta.icon = "artist";
    playlistMeta.readonly = true;
    playlistMeta.hide = false;
    playlistMeta.sortID = 2;
    if (!playlistExist("artist")) {
        addPlaylist(playlistMeta);
    }

    playlistMeta.uuid = "all";
    playlistMeta.displayName = "All Music";
    playlistMeta.icon = "all";
    playlistMeta.readonly = true;
    playlistMeta.hide = false;
    playlistMeta.sortID = 3;
    if (!playlistExist("all")) {
        addPlaylist(playlistMeta);
    }

    playlistMeta.displayName = "My favorites";
    playlistMeta.uuid = "fav";
    playlistMeta.icon = "fav";
    playlistMeta.readonly = true;
    playlistMeta.hide = false;
    playlistMeta.sortID = 4;
    if (!playlistExist("fav")) {
        addPlaylist(playlistMeta);
    }

    playlistMeta.displayName = "Playlist";
    playlistMeta.uuid = "play";
    playlistMeta.icon = "play";
    playlistMeta.readonly = true;
    playlistMeta.hide = true;
    playlistMeta.sortID = 5;
    if (!playlistExist("play")) {
        addPlaylist(playlistMeta);
    }

    playlistMeta.displayName = "Search result";
    playlistMeta.uuid = "search";
    playlistMeta.icon = "search";
    playlistMeta.readonly = true;
    playlistMeta.hide = true;
    playlistMeta.sortID = 0;
    if (!playlistExist("search")) {
        addPlaylist(playlistMeta);
    }

    playlistMeta.displayName = "Music";
    playlistMeta.uuid = "musicCand";
    playlistMeta.icon = "musicCand";
    playlistMeta.readonly = true;
    playlistMeta.hide = true;
    playlistMeta.sortID = 0;
    if (!playlistExist("musicCand")) {
        addPlaylist(playlistMeta);
    }

    playlistMeta.displayName = "Album";
    playlistMeta.uuid = "albumCand";
    playlistMeta.icon = "albumCand";
    playlistMeta.readonly = true;
    playlistMeta.hide = true;
    playlistMeta.sortID = 0;
    if (!playlistExist("albumCand")) {
        addPlaylist(playlistMeta);
    }

    playlistMeta.displayName = "Artist";
    playlistMeta.uuid = "artistCand";
    playlistMeta.icon = "artistCand";
    playlistMeta.readonly = true;
    playlistMeta.hide = true;
    playlistMeta.sortID = 0;
    if (!playlistExist("artistCand")) {
        addPlaylist(playlistMeta);
    }

    playlistMeta.displayName = "Music";
    playlistMeta.uuid = "musicResult";
    playlistMeta.icon = "musicResult";
    playlistMeta.readonly = true;
    playlistMeta.hide = true;
    playlistMeta.sortID = 0;
    if (!playlistExist("musicResult")) {
        addPlaylist(playlistMeta);
    }

    playlistMeta.displayName = "Album";
    playlistMeta.uuid = "albumResult";
    playlistMeta.icon = "albumResult";
    playlistMeta.readonly = true;
    playlistMeta.hide = true;
    playlistMeta.sortID = 0;
    if (!playlistExist("albumResult")) {
        addPlaylist(playlistMeta);
    }

    playlistMeta.displayName = "Artist";
    playlistMeta.uuid = "artistResult";
    playlistMeta.icon = "artistResult";
    playlistMeta.readonly = true;
    playlistMeta.hide = true;
    playlistMeta.sortID = 0;
    if (!playlistExist("artistResult")) {
        addPlaylist(playlistMeta);
    }

    QSqlDatabase::database().commit();
}

QStringList MediaDatabase::allPlaylistDisplayName()
{
    QStringList list;
    QSqlQuery query;
    query.prepare("SELECT displayname FROM playlist");

    if (!query.exec()) {
        qWarning() << query.lastError();
        return list;
    }

    while (query.next()) {
        list <<  query.value(0).toString();
    }
    return list;
}

QList<PlaylistMeta> MediaDatabase::allPlaylistMeta()
{
    QList<PlaylistMeta> list;
    QSqlQuery query;
    query.prepare("SELECT uuid, displayname, icon, readonly, hide, "
                  "sort_type, order_type, sort_id FROM playlist");

    if (!query.exec()) {
        qWarning() << query.lastError();
        return list;
    }

    while (query.next()) {
        PlaylistMeta playlistMeta;
        playlistMeta.uuid = query.value(0).toString();
        playlistMeta.displayName = query.value(1).toString();
        playlistMeta.icon = query.value(2).toString();
        playlistMeta.readonly = query.value(3).toBool();
        playlistMeta.hide = query.value(4).toBool();
        playlistMeta.sortType = query.value(5).toInt();
        playlistMeta.orderType = query.value(6).toInt();
        playlistMeta.sortID = query.value(7).toUInt();
        list << playlistMeta;
    }
    return list;
}

static MetaPtrList searchTitle(const QString &queryString)
{
    MetaPtrList metalist;

    QSqlQuery query;
    query.prepare(queryString);
    if (! query.exec()) {
        qCritical() << query.lastError();
        return metalist;
    }

    while (query.next()) {
        auto hash = query.value(0).toString();
        auto meta = MediaLibrary::instance()->meta(hash);
        if (meta.isNull()) {
            continue;
        }
        metalist << meta;
    }

    return metalist;
}

MetaPtrList MediaDatabase::searchMediaTitle(const QString &title, int limit)
{
    qDebug() << "search title" << title;
    auto matchReg = QString("\"%%1%\" ").arg(title);
    QString queryString = QString("SELECT hash, localpath, title, artist, album, "
                                  "filetype, track, offset, length, size, "
                                  "timestamp, invalid "
                                  "FROM music WHERE "
                                  "title LIKE  " + matchReg +
                                  "OR py_title LIKE  " + matchReg +
                                  "OR py_title_short LIKE  " + matchReg +
                                  "LIMIT " + QString("%1").arg(limit));

    return searchTitle(queryString);
}

MetaPtrList MediaDatabase::searchMediaMeta(const QString &title, int limit)
{
    auto matchReg = QString("\"%%1%\" ").arg(title);
    QString queryString = QString("SELECT hash, localpath, title, artist, album, "
                                  "filetype, track, offset, length, size, "
                                  "timestamp, invalid "
                                  "FROM music WHERE "
                                  "title LIKE  " + matchReg +
                                  "OR py_title LIKE  " + matchReg +
                                  "OR py_title_short LIKE  " + matchReg +
                                  "OR py_artist LIKE  " + matchReg +
                                  "OR py_artist_short LIKE  " + matchReg +
                                  "OR py_album LIKE  " + matchReg +
                                  "OR py_album_short LIKE  " + matchReg +
                                  "OR artist LIKE " + matchReg +
                                  "OR album LIKE " + matchReg +
                                  "LIMIT " + QString("%1").arg(limit));

    return searchTitle(queryString);
}

MetaPtrList MediaDatabase::searchMediaPath(const QString &path, int limit)
{
    auto matchReg = QString("\"%%1%\" ").arg(path);
    QString queryString = QString("SELECT hash, localpath, title, artist, album, "
                                  "filetype, track, offset, length, size, timestamp "
                                  "FROM music WHERE "
                                  "localpath LIKE  " + matchReg +
                                  "OR cuepath LIKE  " + matchReg +
                                  "LIMIT " + QString("%1").arg(limit));

    return searchTitle(queryString);
}

void MediaDatabase::addPlaylist(const PlaylistMeta &playlistMeta)
{
    QSqlQuery query;
    query.prepare("INSERT INTO playlist ("
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

    if (! query.exec()) {
        qWarning() << query.lastError();
        return;
    }

    QString sqlstring = QString("CREATE TABLE IF NOT EXISTS playlist_%1 ("
                                "music_id TEXT primary key not null, "
                                "playlist_id TEXT, sort_id INTEGER"
                                ")").arg(playlistMeta.uuid);
    if (! query.exec(sqlstring)) {
        qWarning() << query.lastError();
        return;
    }
}

void MediaDatabase::updatePlaylist(const PlaylistMeta &playlistMeta)
{
    QSqlQuery query;
    query.prepare("UPDATE playlist "
                  "SET displayname = :displayname, icon = :icon, "
                  "readonly = :readonly, hide = :hide, "
                  "order_type = :order_type, "
                  "sort_type = :sort_type "
                  "WHERE uuid = :uuid;");
    query.bindValue(":uuid", playlistMeta.uuid);
    query.bindValue(":displayname", playlistMeta.displayName);
    query.bindValue(":icon", playlistMeta.icon);
    query.bindValue(":readonly", playlistMeta.readonly);
    query.bindValue(":hide", playlistMeta.hide);
    query.bindValue(":order_type", playlistMeta.orderType);
    query.bindValue(":sort_type", playlistMeta.sortType);

    if (! query.exec()) {
        qWarning() << query.lastError();
        return;
    }
}

void MediaDatabase::removePlaylist(const PlaylistMeta &playlistMeta)
{
    QSqlQuery query;
    QString sqlstring = QString("DROP TABLE IF EXISTS playlist_%1").arg(playlistMeta.uuid);
    if (! query.exec(sqlstring)) {
        qWarning() << query.lastError();
        return;
    }

    sqlstring = QString("DELETE FROM playlist WHERE uuid = '%1'").arg(playlistMeta.uuid);
    if (! query.exec(sqlstring)) {
        qWarning() << query.lastError();
        return;
    }
}

void MediaDatabase::deleteMusic(const MetaPtr meta, const PlaylistMeta &playlistMeta)
{
    if (meta.isNull()) {
        return;
    }

    QSqlQuery query;
    QString sqlstring = QString("DELETE FROM playlist_%1 WHERE music_id = '%2'")
                        .arg(playlistMeta.uuid).arg(meta->hash);
    if (! query.exec(sqlstring)) {
        qWarning() << query.lastError();
        return;
    }
}

bool MediaDatabase::playlistExist(const QString &uuid)
{
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM playlist where uuid = :uuid");
    query.bindValue(":uuid", uuid);

    if (!query.exec()) {
        qWarning() << query.lastError();
        return false;
    }
    query.first();
    return query.value(0).toInt() > 0;
}

bool MediaDatabase::mediaMetaExist(const QString &hash)
{
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM music where hash = :hash");
    query.bindValue(":hash", hash);

    if (!query.exec()) {
        qWarning() << query.lastError();
        return false;
    }
    query.first();
    return query.value(0).toInt() > 0;
}

QList<MediaMeta> MediaDatabase::allmetas()
{
    QList<MediaMeta> metalist;
    QList<QString> allhash;
    QVariantList removeHash;
    QSqlQuery querysql;
    QString queryStringAll = QString("select music_id from playlist_all");

    querysql.prepare(queryStringAll);
    if (!querysql.exec()) {
        qCritical() << querysql.lastError();
        return metalist;
    }

    while (querysql.next()) {
        allhash << querysql.value(0).toString();
    }

    QString queryStringNew = QString("SELECT hash, localpath, title, artist, album, "
                                     "filetype, track, offset, length, size, "
                                     "timestamp, invalid, search_id, cuepath, "
                                     "lyricPath, codec "
                                     "FROM musicNew");
    querysql.prepare(queryStringNew);
    if (! querysql.exec()) {
        qCritical() << querysql.lastError();
        return metalist;
    }

    while (querysql.next()) {
        QString musicHash = querysql.value(0).toString();
        if (!allhash.contains(musicHash)) {
            removeHash.append(musicHash);
            continue;
        }

        MediaMeta meta;
        meta.hash = querysql.value(0).toString();
        meta.localPath = querysql.value(1).toString();
        meta.title = querysql.value(2).toString();
        meta.artist = querysql.value(3).toString();
        meta.album = querysql.value(4).toString();
        meta.filetype = querysql.value(5).toString();
        meta.track = querysql.value(6).toLongLong();
        meta.offset = querysql.value(7).toLongLong();
        meta.length = querysql.value(8).toLongLong();
        meta.size = querysql.value(9).toLongLong();
        meta.timestamp = querysql.value(10).toLongLong();
        meta.invalid = querysql.value(11).toBool();
        meta.searchID = querysql.value(12).toString();
        meta.cuePath = querysql.value(13).toString();
        meta.lyricPath = querysql.value(14).toString();
        meta.codec = querysql.value(15).toString();
        metalist << meta;
    }

    if (removeHash.size() > 0) {
        //remove from musicNew
        for (QVariant var : removeHash) {
            QString queryStringRemove = QString("delete from musicNew where hash=%1").arg(var.toString());
            querysql.prepare(queryStringRemove);
            if (!querysql.exec()) {
                qCritical() << querysql.lastError();
            }
        }
    }

    return metalist;
}

void MediaDatabase::bind()
{
    connect(this, &MediaDatabase::addMediaMeta,
            m_writer, &MediaDatabaseWriter::addMediaMeta, Qt::UniqueConnection);
    connect(this, &MediaDatabase::addMediaMetaList,
            m_writer, &MediaDatabaseWriter::addMediaMetaList, Qt::UniqueConnection);
    connect(this, &MediaDatabase::updateMediaMeta,
            m_writer, &MediaDatabaseWriter::updateMediaMeta, Qt::UniqueConnection);
    connect(this, &MediaDatabase::updateMediaMetaList,
            m_writer, &MediaDatabaseWriter::updateMediaMetaList, Qt::UniqueConnection);
    connect(this, &MediaDatabase::insertMusic,
            m_writer, &MediaDatabaseWriter::insertMusic, Qt::UniqueConnection);
    connect(this, &MediaDatabase::insertMusicList,
            m_writer, &MediaDatabaseWriter::insertMusicList, Qt::UniqueConnection);
    connect(this, &MediaDatabase::removeMediaMetaList,
            m_writer, &MediaDatabaseWriter::removeMediaMetaList, Qt::UniqueConnection);
}


