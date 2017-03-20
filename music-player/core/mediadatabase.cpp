/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "mediadatabase.h"

#include <QDebug>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
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

MediaDatabase::MediaDatabase(QObject *parent) : QObject(parent)
{
}

void MediaDatabase::init()
{
    createConnection();

    m_writer = new MediaDatabaseWriter;
    auto work = ThreadPool::instance()->newThread();
    m_writer->moveToThread(work);
    work->start();

    bind();

    margeDatabase();

    QSqlDatabase::database().transaction();
    PlaylistMeta playlistMeta;
    playlistMeta.uuid = "all";
    playlistMeta.displayName = "All Music";
    playlistMeta.icon = "all";
    playlistMeta.readonly = true;
    playlistMeta.hide = false;
    playlistMeta.sortID = 1;
    if (!playlistExist("all")) {
        addPlaylist(playlistMeta);
    }

    playlistMeta.displayName = "My favorites";
    playlistMeta.uuid = "fav";
    playlistMeta.icon = "fav";
    playlistMeta.readonly = true;
    playlistMeta.hide = false;
    playlistMeta.sortID = 2;
    if (!playlistExist("fav")) {
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
    QString queryString = QString("SELECT hash, localpath, title, artist, album, "
                                  "filetype, track, offset, length, size, "
                                  "timestamp, invalid, search_id, cuepath "
                                  "FROM music");

    QSqlQuery query;
    query.prepare(queryString);
    if (! query.exec()) {
        qCritical() << query.lastError();
        return metalist;
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
        meta.offset = query.value(7).toInt();
        meta.length = query.value(8).toInt();
        meta.size = query.value(9).toInt();
        meta.timestamp = query.value(10).toInt();
        meta.invalid = query.value(11).toBool();
        meta.searchID = query.value(12).toString();
        meta.cuePath = query.value(13).toString();
        metalist << meta;
    }

    return metalist;
}

void MediaDatabase::bind()
{
    connect(this, &MediaDatabase::addMediaMeta,
            m_writer, &MediaDatabaseWriter::addMediaMeta);
    connect(this, &MediaDatabase::addMediaMetaList,
            m_writer, &MediaDatabaseWriter::addMediaMetaList);
    connect(this, &MediaDatabase::updateMediaMeta,
            m_writer, &MediaDatabaseWriter::updateMediaMeta);
    connect(this, &MediaDatabase::updateMediaMetaList,
            m_writer, &MediaDatabaseWriter::updateMediaMetaList);
    connect(this, &MediaDatabase::insertMusic,
            m_writer, &MediaDatabaseWriter::insertMusic);
    connect(this, &MediaDatabase::insertMusicList,
            m_writer, &MediaDatabaseWriter::insertMusicList);
    connect(this, &MediaDatabase::removeMediaMetaList,
            m_writer, &MediaDatabaseWriter::removeMediaMetaList);
}


