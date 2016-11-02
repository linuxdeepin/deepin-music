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

MediaDatabaseWriter::MediaDatabaseWriter(QObject *parent) : QObject(parent)
{

}

void MediaDatabaseWriter::addMusicMetaList(const MusicMetaList &metalist)
{
//    qDebug() << "addMusicMetaList beign";
    QSqlDatabase::database().transaction();
    QSqlQuery query;

    for (auto &meta : metalist) {
        addMusicMeta(meta);
    }
    QSqlDatabase::database().commit();
//    qDebug() << "addMusicMetaList end";
}

void MediaDatabaseWriter::addMusicMeta(const MusicMeta &metalist)
{
//    qDebug() << "addMusicMeta beign";
    QSqlQuery query;
    query.prepare("INSERT INTO music ("
                  "hash, timestamp, title, artist, album, "
                  "filetype, size, track, offset, favourite, localpath, length, "
                  "py_title, py_title_short, py_artist, py_artist_short, "
                  "py_album, py_album_short"
                  ") "
                  "VALUES ("
                  ":hash, :timestamp, :title, :artist, :album, "
                  ":filetype, :size, :track, :offset, :favourite, :localpath, :length, "
                  ":py_title, :py_title_short, :py_artist, :py_artist_short, "
                  ":py_album, :py_album_short "
                  ")");
    query.bindValue(":hash", metalist.hash);
    query.bindValue(":timestamp", metalist.timestamp);
    query.bindValue(":title", metalist.title);
    query.bindValue(":artist", metalist.artist);
    query.bindValue(":album", metalist.album);
    query.bindValue(":filetype", metalist.filetype);
    query.bindValue(":size", metalist.size);
    query.bindValue(":track", metalist.track);
    query.bindValue(":offset", metalist.offset);
    query.bindValue(":favourite", metalist.favourite);
    query.bindValue(":localpath", metalist.localpath);
    query.bindValue(":length", metalist.length);
    query.bindValue(":py_title", metalist.pinyinTitle);
    query.bindValue(":py_title_short", metalist.pinyinTitleShort);
    query.bindValue(":py_artist", metalist.pinyinArtist);
    query.bindValue(":py_artist_short", metalist.pinyinArtistShort);
    query.bindValue(":py_album", metalist.pinyinAlbum);
    query.bindValue(":py_album_short", metalist.pinyinAlbumShort);

    if (! query.exec()) {
        qCritical() << query.lastError();
        return;
    }
//    qDebug() << "addMusicMeta end";
}


void MediaDatabaseWriter::insertMusic(const MusicMeta &meta,
                                      const PlaylistMeta &palylistMeta)
{
//    qDebug() << "insertMusic begin";
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
        qCritical() << query.lastError() << sqlstring;
        return;
    }
//    qDebug() << "insertMusic end";
}

void MediaDatabaseWriter::insertMusicList(const MusicMetaList &metalist, const PlaylistMeta &palylistMeta)
{
//    qDebug() << "insertMusicList beign";
    QSqlDatabase::database().transaction();
    QSqlQuery query;

    for (auto &meta : metalist) {
        insertMusic(meta, palylistMeta);
    }
    QSqlDatabase::database().commit();
//    qDebug() << "insertMusicList end";
}
