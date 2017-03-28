/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "mediadatabasewriter.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QDir>
#include <QThread>

#include <mediameta.h>

MediaDatabaseWriter::MediaDatabaseWriter(QObject *parent) : QObject(parent)
{

}

void MediaDatabaseWriter::addMediaMetaList(const MetaPtrList metalist)
{
//    qDebug() << "addMediaMetaList begin";
    QSqlDatabase::database().transaction();
    for (auto &meta : metalist) {
        addMediaMeta(meta);
    }
    QSqlDatabase::database().commit();
    //    qDebug() << "addMediaMetaList end";
}

void MediaDatabaseWriter::updateMediaMeta(const MetaPtr meta)
{
    qDebug() << "updateMediaMeta begin";
    QSqlQuery query;

    query.prepare("UPDATE music set "
                  "invalid=:invalid, length=:length, search_id=:search_id, "
                  "title=:title, artist=:artist, album=:album, "
                  "py_title=:py_title, py_title_short=:py_title_short, py_artist=:py_artist, "
                  "py_artist_short=:py_artist_short, py_album=:py_album, py_album_short=:py_album_short "
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
        updateMediaMeta(meta);
    }
    QSqlDatabase::database().commit();
}

void MediaDatabaseWriter::removeMediaMeta(const MetaPtr meta)
{
    QSqlQuery query;
    QString sqlstring = QString("DELETE FROM music WHERE hash = '%1'").arg(meta->hash);
    if (! query.exec(sqlstring)) {
        qWarning() << query.lastError();
        return;
    }
}

void MediaDatabaseWriter::removeMediaMetaList(const MetaPtrList metalist)
{
    QSqlDatabase::database().transaction();
    for (auto &meta : metalist) {
        removeMediaMeta(meta);
    }
    QSqlDatabase::database().commit();
}


void MediaDatabaseWriter::addMediaMeta(const MetaPtr meta)
{
//    qDebug() << "addMediaMeta begin";
    QSqlQuery query;
    query.prepare("INSERT INTO music ("
                  "hash, timestamp, title, artist, album, "
                  "filetype, size, track, offset, favourite, localpath, length, "
                  "py_title, py_title_short, py_artist, py_artist_short, "
                  "py_album, py_album_short, cuepath "
                  ") "
                  "VALUES ("
                  ":hash, :timestamp, :title, :artist, :album, "
                  ":filetype, :size, :track, :offset, :favourite, :localpath, :length, "
                  ":py_title, :py_title_short, :py_artist, :py_artist_short, "
                  ":py_album, :py_album_short, :cuepath "
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
    query.bindValue(":cuepath", meta->cuePath);

    if (! query.exec()) {
        qCritical() << query.lastError();
        return;
    }
//    qDebug() << "addMediaMeta end";
}


void MediaDatabaseWriter::insertMusic(const MetaPtr meta,
                                      const PlaylistMeta &playlistMeta)
{
//    qDebug() << "insertMusic begin";
    QSqlQuery query;
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
        insertMusic(meta, playlistMeta);
    }
    QSqlDatabase::database().commit();
//    qDebug() << "insertMusicList end";
}

