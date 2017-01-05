/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "mediadatabase.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QDir>
#include <QThread>

#include "music.h"
#include "../musicapp.h"

static bool createConnection()
{
    QDir cacheDir(MusicApp::cachePath());
    if (!cacheDir.exists()) {
        cacheDir.mkpath(".");
    }
    QString cachePath = MusicApp::cachePath() + "/mediameta.sqlite";
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(cachePath);

    if (!db.open()) {
        qCritical() << db.lastError()
                    << MusicApp::cachePath()
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
               "invalid INTEGER(32), "
               "cuepath VARCHAR(4096) )"
              );

    query.exec("CREATE TABLE IF NOT EXISTS ablum (id int primary key, "
               "name VARCHAR(20), localpath VARCHAR(4096), url VARCHAR(4096))");

    query.exec("CREATE TABLE IF NOT EXISTS artist (id int primary key, "
               "name VARCHAR(20))");

    query.exec("CREATE TABLE IF NOT EXISTS playlist (uuid TEXT primary key not null, "
               "displayname VARCHAR(4096), "
               "icon VARCHAR(256), readonly INTEGER,"
               "hide INTEGER, sortType INTEGER)");

    return true;
}

MediaDatabase::MediaDatabase(QObject *parent) : QObject(parent)
{
    createConnection();

    m_writer = new MediaDatabaseWriter;
    auto work = new QThread;
    m_writer->moveToThread(work);
    work->start();

    bind();

    QSqlDatabase::database().transaction();

    QSqlQuery query;
    query.prepare("ALTER TABLE music ADD COLUMN cuepath VARCHAR(4096);");
    if (query.exec()) {
        qWarning() << "sql upgrade with out error:" << query.lastError();
    }

    query.prepare("ALTER TABLE music ADD COLUMN invalid INTEGER(32);");
    if (query.exec()) {
        qWarning() << "sql upgrade with out error:" << query.lastError();
    }

    PlaylistMeta playlistMeta;
    playlistMeta.uuid = "all";
    playlistMeta.displayName = "All Music";
    playlistMeta.icon = "all";
    playlistMeta.readonly = true;
    playlistMeta.hide = false;
    if (!playlistExist("all")) {
        addPlaylist(playlistMeta);
    }

    playlistMeta.displayName = "Favourite Music";
    playlistMeta.uuid = "fav";
    playlistMeta.icon = "fav";
    playlistMeta.readonly = true;
    playlistMeta.hide = false;
    if (!playlistExist("fav")) {
        addPlaylist(playlistMeta);
    }

    playlistMeta.displayName = "Search Music";
    playlistMeta.uuid = "search";
    playlistMeta.icon = "search";
    playlistMeta.readonly = true;
    playlistMeta.hide = true;
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

QList<PlaylistMeta> MediaDatabase::allPlaylist()
{
    QList<PlaylistMeta> list;
    QSqlQuery query;
    query.prepare("SELECT uuid, displayname, icon, readonly, hide, sortType FROM playlist");

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
        list << playlistMeta;
    }
    return list;
}

static QList<MusicMeta> searchTitle(const QString &queryString)
{
    QList<MusicMeta> list;
    QSqlQuery query;
    query.prepare(queryString);
    if (! query.exec()) {
        qCritical() << query.lastError();
        return list;
    }

    while (query.next()) {
        MusicMeta musicMeta;
        musicMeta.hash = query.value(0).toString();
        musicMeta.localPath = query.value(1).toString();
        musicMeta.title = query.value(2).toString();
        musicMeta.artist = query.value(3).toString();
        musicMeta.album = query.value(4).toString();
        musicMeta.filetype = query.value(5).toString();
        musicMeta.track = query.value(6).toInt();
        musicMeta.offset = query.value(7).toInt();
        musicMeta.length = query.value(8).toInt();
        musicMeta.size = query.value(9).toInt();
        musicMeta.timestamp = query.value(10).toInt();
        list << musicMeta;
    }

    return list;
}

QList<MusicMeta> MediaDatabase::searchMusicTitle(const QString &title, int limit)
{
    auto matchReg = QString("\"%%1%\" ").arg(title);
    QString queryString = QString("SELECT hash, localpath, title, artist, album, "
                                  "filetype, track, offset, length, size, timestamp "
                                  "FROM music WHERE "
                                  "title LIKE  " + matchReg +
                                  "OR py_title LIKE  " + matchReg +
                                  "OR py_title_short LIKE  " + matchReg +
                                  "LIMIT " + QString("%1").arg(limit));

    return searchTitle(queryString);
}

QList<MusicMeta> MediaDatabase::searchMusicMeta(const QString &title, int limit)
{
    auto matchReg = QString("\"%%1%\" ").arg(title);
    QString queryString = QString("SELECT hash, localpath, title, artist, album, "
                                  "filetype, track, offset, length, size, timestamp "
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

QList<MusicMeta> MediaDatabase::searchMusicPath(const QString &path, int limit)
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
                  "sortType "
                  ") "
                  "VALUES ("
                  ":uuid, :displayname, :icon, :readonly, :hide, "
                  ":sortType "
                  ")");
    query.bindValue(":uuid", playlistMeta.uuid);
    query.bindValue(":displayname", playlistMeta.displayName);
    query.bindValue(":icon", playlistMeta.icon);
    query.bindValue(":readonly", playlistMeta.readonly);
    query.bindValue(":hide", playlistMeta.hide);
    query.bindValue(":sortType", playlistMeta.sortType);

    if (! query.exec()) {
        qWarning() << query.lastError();
        return;
    }

    QString sqlstring = QString("CREATE TABLE IF NOT EXISTS playlist_%1 ("
                                "music_id TEXT primary key not null, "
                                "playlist_id TEXT, sortid INTEGER"
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
                  "sortType = :sortType "
                  "WHERE uuid = :uuid;");
    query.bindValue(":uuid", playlistMeta.uuid);
    query.bindValue(":displayname", playlistMeta.displayName);
    query.bindValue(":icon", playlistMeta.icon);
    query.bindValue(":readonly", playlistMeta.readonly);
    query.bindValue(":hide", playlistMeta.hide);
    query.bindValue(":sortType", playlistMeta.sortType);

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

void MediaDatabase::deleteMusic(const MusicMeta &meta, const PlaylistMeta &playlistMeta)
{
    QSqlQuery query;
    QString sqlstring = QString("DELETE FROM playlist_%1 WHERE music_id = '%2'")
                        .arg(playlistMeta.uuid).arg(meta.hash);
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

bool MediaDatabase::musicMetaExist(const QString &hash)
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

void MediaDatabase::bind()
{
    connect(this, &MediaDatabase::addMusicMeta,
            m_writer, &MediaDatabaseWriter::addMusicMeta);
    connect(this, &MediaDatabase::addMusicMetaList,
            m_writer, &MediaDatabaseWriter::addMusicMetaList);
    connect(this, &MediaDatabase::updateMusicMeta,
            m_writer, &MediaDatabaseWriter::updateMusicMeta);
    connect(this, &MediaDatabase::updateMusicMetaList,
            m_writer, &MediaDatabaseWriter::updateMusicMetaList);
    connect(this, &MediaDatabase::insertMusic,
            m_writer, &MediaDatabaseWriter::insertMusic);
    connect(this, &MediaDatabase::insertMusicList,
            m_writer, &MediaDatabaseWriter::insertMusicList);
    connect(this, &MediaDatabase::removeMusicMetaList,
            m_writer, &MediaDatabaseWriter::removeMusicMetaList);
}


