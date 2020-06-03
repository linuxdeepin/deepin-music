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

#include "mediadatabasewriter.h"

#include <QUuid>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QDir>
#include <QThread>
#include "util/global.h"
#include <mediameta.h>


bool MediaDatabaseWriter::createConnection()
{
    QDir cacheDir(Global::cacheDir());
    if (!cacheDir.exists()) {
        cacheDir.mkpath(".");
    }
    QString cachePath = Global::cacheDir() + "/mediameta.sqlite";
    db = QSqlDatabase::addDatabase("QSQLITE","writer");
    db.setDatabaseName(cachePath);

    if (!db.open()) {
        qCritical() << db.lastError()
                    << Global::cacheDir()
                    << cachePath;
        return false;
    }
}

void MediaDatabaseWriter::initDataBase()
{
    createConnection();
}

MediaDatabaseWriter::MediaDatabaseWriter(QObject *parent) : QObject(parent)
{
}

void MediaDatabaseWriter::addMediaMetaList(const MetaPtrList metalist)
{
    QSqlDatabase::database().transaction();
    for (auto &meta : metalist) {
        QThread::msleep(1);
        addMediaMeta(meta);
    }
    db.commit();
}

void MediaDatabaseWriter::updateMediaMeta(const MetaPtr meta)
{
    qDebug() << "updateMediaMeta begin";
    QSqlQuery query(QSqlDatabase::database("writer"));

    query.prepare("UPDATE musicNew set "
                  "invalid=:invalid, length=:length, search_id=:search_id, "
                  "title=:title, artist=:artist, album=:album, "
                  "py_title=:py_title, py_title_short=:py_title_short, py_artist=:py_artist, "
                  "py_artist_short=:py_artist_short, py_album=:py_album, py_album_short=:py_album_short, "
                  "lyricPath=:lyricPath, codec=:codec "
                  "where hash=:hash");

    query.bindValue(":search_id", meta->searchID);
    query.bindValue(":invalid", meta->invalid);
    query.bindValue(":length", meta->length);
    query.bindValue(":title", meta->title);
    query.bindValue(":artist", meta->artist);
    query.bindValue(":album", meta->album);
    query.bindValue(":py_title", meta->pinyinTitle);
    query.bindValue(":py_title_short", meta->pinyinTitleShort);
    query.bindValue(":py_artist", meta->pinyinArtist);
    query.bindValue(":py_artist_short", meta->pinyinArtistShort);
    query.bindValue(":py_album", meta->pinyinAlbum);
    query.bindValue(":py_album_short", meta->pinyinAlbumShort);
    query.bindValue(":lyricPath", meta->lyricPath);
    query.bindValue(":codec", meta->codec);
    query.bindValue(":hash", meta->hash);

    if (! query.exec()) {
        qCritical() << query.lastError();
        return;
    }
    qDebug() << "updateMediaMeta end";

}

void MediaDatabaseWriter::updateMediaMetaList(const MetaPtrList metalist)
{
    QSqlDatabase::database().transaction();
    for (auto &meta : metalist) {
        QThread::msleep(1);
        updateMediaMeta(meta);
    }
    QSqlDatabase::database().commit();
}

void MediaDatabaseWriter::removeMediaMeta(const MetaPtr meta)
{
    QSqlQuery query(QSqlDatabase::database("writer"));
    QString sqlstring;

    sqlstring = QString("DELETE FROM musicNew WHERE hash = '%1'").arg(meta->hash);
    if (! query.exec(sqlstring)) {
        qWarning() << query.lastError();
        return;
    }
}

void MediaDatabaseWriter::removeMediaMetaList(const MetaPtrList metalist)
{
    QSqlDatabase::database().transaction();
    for (auto &meta : metalist) {
        QThread::msleep(1);
        removeMediaMeta(meta);
    }
    db.commit();
}


void MediaDatabaseWriter::addMediaMeta(const MetaPtr meta)
{
    QSqlQuery query(QSqlDatabase::database("writer"));

    query.prepare("INSERT INTO musicNew ("
                  "hash, timestamp, title, artist, album, "
                  "filetype, size, track, offset, favourite, localpath, length, "
                  "py_title, py_title_short, py_artist, py_artist_short, "
                  "py_album, py_album_short, lyricPath, codec, cuepath "
                  ") "
                  "VALUES ("
                  ":hash, :timestamp, :title, :artist, :album, "
                  ":filetype, :size, :track, :offset, :favourite, :localpath, :length, "
                  ":py_title, :py_title_short, :py_artist, :py_artist_short, "
                  ":py_album, :py_album_short, :lyricPath, :codec, :cuepath "
                  ")");

    query.bindValue(":hash", meta->hash);
    query.bindValue(":timestamp", meta->timestamp);
    query.bindValue(":title", meta->title);
    query.bindValue(":artist", meta->artist);
    query.bindValue(":album", meta->album);
    query.bindValue(":filetype", meta->filetype);
    query.bindValue(":size", meta->size);
    query.bindValue(":track", meta->track);
    query.bindValue(":offset", meta->offset);
    query.bindValue(":favourite", meta->favourite);
    query.bindValue(":localpath", meta->localPath);
    query.bindValue(":length", meta->length);
    query.bindValue(":py_title", meta->pinyinTitle);
    query.bindValue(":py_title_short", meta->pinyinTitleShort);
    query.bindValue(":py_artist", meta->pinyinArtist);
    query.bindValue(":py_artist_short", meta->pinyinArtistShort);
    query.bindValue(":py_album", meta->pinyinAlbum);
    query.bindValue(":py_album_short", meta->pinyinAlbumShort);
    query.bindValue(":lyricPath", meta->lyricPath);
    query.bindValue(":codec", meta->codec);
    query.bindValue(":cuepath", meta->cuePath);

    if (! query.exec()) {
        qCritical() << query.lastError();
        return;
    }
}

void MediaDatabaseWriter::insertMusic(const MetaPtr meta,
                                      const PlaylistMeta &playlistMeta)
{
//    qDebug() << "insertMusic begin";
    QSqlQuery query(QSqlDatabase::database("writer"));
    QString sqlstring = QString("INSERT INTO playlist_%1 "
                                "(music_id, playlist_id, sort_id) "
                                "SELECT :music_id, :playlist_id, :sort_id "
                                "WHERE NOT EXISTS("
                                "SELECT * FROM playlist_%1 "
                                "WHERE music_id = :music_id)").arg(playlistMeta.uuid);
    query.prepare(sqlstring);
    query.bindValue(":playlist_id", playlistMeta.uuid);
    query.bindValue(":music_id", meta->hash);
    query.bindValue(":sort_id", 0);

    if (! query.exec()) {
        qCritical() << query.lastError() << sqlstring;
        return;
    }
//    qDebug() << "insertMusic end";
}

void MediaDatabaseWriter::insertMusicList(const MetaPtrList metalist, const PlaylistMeta &playlistMeta)
{
//    qDebug() << "insertMusicList begin";
    QSqlDatabase::database().transaction();
    for (auto &meta : metalist) {
        QThread::msleep(1);
        insertMusic(meta, playlistMeta);
    }
    db.commit();
//    qDebug() << "insertMusicList end";
}

