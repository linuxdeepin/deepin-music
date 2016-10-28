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

#include "../model/musiclistmodel.h"
#include "../musicapp.h"

static bool createConnection()
{
    QDir cacheDir(MusicApp::cachePath());
    if (!cacheDir.exists()) {
        cacheDir.mkdir(".");
    }
    QString cachePath = MusicApp::cachePath() + "/mediameta.sqlite";
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(cachePath);

    if (!db.open()) {
        qDebug() << db.lastError()
                 << MusicApp::cachePath()
                 << cachePath;
        return false;
    }

    QSqlQuery query;
    query.exec("CREATE TABLE IF NOT EXISTS music (hash TEXT primary key not null, "
               "timestamp INTEGER,"
               "title VARCHAR(256), artist VARCHAR(256),"
               "album VARCHAR(256), filetype VARCHAR(32),"
               "size INTEGER, track INTEGER,"
               "favourite INTEGER(32),"
               "localpath VARCHAR(4096), length INTEGER)"
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

    PlaylistMeta palylistMeta;
    palylistMeta.uuid = "all";
    palylistMeta.displayName = "All Music";
    palylistMeta.icon = "all";
    palylistMeta.readonly = true;
    palylistMeta.hide = false;
    if (!playlistExist("all")) {
        addPlaylist(palylistMeta);
    }

    palylistMeta.displayName = "Favourite Music";
    palylistMeta.uuid = "fav";
    palylistMeta.icon = "fav";
    palylistMeta.readonly = true;
    palylistMeta.hide = false;
    if (!playlistExist("fav")) {
        addPlaylist(palylistMeta);
    }

    palylistMeta.displayName = "Search Music";
    palylistMeta.uuid = "search";
    palylistMeta.icon = "search";
    palylistMeta.readonly = true;
    palylistMeta.hide = true;
    if (!playlistExist("search")) {
        addPlaylist(palylistMeta);
    }
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
        PlaylistMeta palylistMeta;
        palylistMeta.uuid = query.value(0).toString();
        palylistMeta.displayName = query.value(1).toString();
        palylistMeta.icon = query.value(2).toString();
        palylistMeta.readonly = query.value(3).toBool();
        palylistMeta.hide = query.value(4).toBool();
        palylistMeta.sortType = query.value(5).toInt();
        list << palylistMeta;
    }
    return list;
}

void MediaDatabase::addPlaylist(const PlaylistMeta &palylistMeta)
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
    query.bindValue(":uuid", palylistMeta.uuid);
    query.bindValue(":displayname", palylistMeta.displayName);
    query.bindValue(":icon", palylistMeta.icon);
    query.bindValue(":readonly", palylistMeta.readonly);
    query.bindValue(":hide", palylistMeta.hide);
    query.bindValue(":sortType", palylistMeta.sortType);

    if (! query.exec()) {
        qWarning() << query.lastError();
        return;
    }

    QString sqlstring = QString("CREATE TABLE IF NOT EXISTS playlist_%1 ("
                                "music_id TEXT primary key not null, "
                                "playlist_id TEXT, sortid INTEGER"
                                ")").arg(palylistMeta.uuid);
    if (! query.exec(sqlstring)) {
        qWarning() << query.lastError();
        return;
    }
}

void MediaDatabase::updatePlaylist(const PlaylistMeta &palylistMeta)
{
    QSqlQuery query;
    query.prepare("UPDATE playlist "
                  "SET displayname = :displayname, icon = :icon, "
                  "readonly = :readonly, hide = :hide, "
                  "sortType = :sortType "
                  "WHERE uuid = :uuid;");
    query.bindValue(":uuid", palylistMeta.uuid);
    query.bindValue(":displayname", palylistMeta.displayName);
    query.bindValue(":icon", palylistMeta.icon);
    query.bindValue(":readonly", palylistMeta.readonly);
    query.bindValue(":hide", palylistMeta.hide);
    query.bindValue(":sortType", palylistMeta.sortType);

    if (! query.exec()) {
        qWarning() << query.lastError();
        return;
    }
}

void MediaDatabase::removePlaylist(const PlaylistMeta &palylistMeta)
{
    qDebug() << "remove";
    QSqlQuery query;
    QString sqlstring = QString("DROP TABLE IF EXISTS playlist_%1").arg(palylistMeta.uuid);
    if (! query.exec(sqlstring)) {
        qWarning() << query.lastError();
        return;
    }

    sqlstring = QString("DELETE FROM playlist WHERE uuid = '%1'").arg(palylistMeta.uuid);
    if (! query.exec(sqlstring)) {
        qWarning() << query.lastError();
        return;
    }
}

void MediaDatabase::insertMusic(const MusicMeta &meta, const PlaylistMeta &palylistMeta)
{
    QSqlQuery query;
    QString sqlstring = QString("INSERT INTO playlist_%1 "
                                "(music_id, playlist_id, sortid) "
                                "SELECT :music_id, :playlist_id, :sort_id "
                                "WHERE NOT EXISTS("
                                "SELECT * FROM playlist_%1 "
                                "WHERE music_id = :music_id)").arg(palylistMeta.uuid);
    query.prepare(sqlstring);
    query.bindValue(":playlist_id", palylistMeta.uuid);
    query.bindValue(":music_id", meta.hash);
    query.bindValue(":sort_id", 0);

    if (! query.exec()) {
        qDebug() << query.lastError() << sqlstring;
        return;
    }
}

void MediaDatabase::deleteMusic(const MusicMeta &meta, const PlaylistMeta &palylistMeta)
{
    QSqlQuery query;
    QString sqlstring = QString("DELETE FROM playlist_%1 WHERE music_id = '%2'")
                        .arg(palylistMeta.uuid).arg(meta.hash);
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

void MediaDatabase::addMusicMeta(const MusicMeta &meta)
{
    QSqlQuery query;
    query.prepare("INSERT INTO music ("
                  "hash, timestamp, title, artist, album, "
                  "filetype, size, track, favourite, localpath, length"
                  ") "
                  "VALUES ("
                  ":hash, :timestamp, :title, :artist, :album, "
                  ":filetype, :size, :track, :favourite, :localpath, :length"
                  ")");
    query.bindValue(":hash", meta.hash);
    query.bindValue(":timestamp", meta.timestamp);
    query.bindValue(":title", meta.title);
    query.bindValue(":artist", meta.artist);
    query.bindValue(":album", meta.album);
    query.bindValue(":filetype", meta.filetype);
    query.bindValue(":size", meta.size);
    query.bindValue(":track", meta.track);
    query.bindValue(":favourite", meta.favourite);
    query.bindValue(":localpath", meta.localpath);
    query.bindValue(":length", meta.length);

    if (! query.exec()) {
        qDebug() << query.lastError();
        return;
    }
}

void MediaDatabase::removeMusicMeta(const MusicMeta &meta)
{
    QSqlQuery query;
    QString sqlstring = QString("DELETE FROM music WHERE hash = '%1'").arg(meta.hash);
    if (! query.exec(sqlstring)) {
        qWarning() << query.lastError();
        return;
    }
}

