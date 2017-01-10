/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "mediadatabasewriter.h"
#include "util/musicmeta.h"

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
    for (auto &meta : metalist) {
        addMusicMeta(meta);
    }
    QSqlDatabase::database().commit();
    //    qDebug() << "addMusicMetaList end";
}

void MediaDatabaseWriter::updateMusicMeta(const MusicMeta &meta)
{
    MusicMeta saveMeta = meta;
    MusicMetaName::pinyinIndex(saveMeta);

    qDebug() << "updateMusicMeta beign";
    QSqlQuery query;

    query.prepare("UPDATE music set "
                  "invalid=:invalid, length=:length, "
                  "title=:title, artist=:artist, album=:album, "
                  "py_title=:py_title, py_title_short=:py_title_short, py_artist=:py_artist, "
                  "py_artist_short=:py_artist_short, py_album=:py_album, py_album_short=:py_album_short "
                  "where hash=:hash");

    query.bindValue(":invalid", saveMeta.invalid);
    query.bindValue(":length", saveMeta.length);
    query.bindValue(":title", saveMeta.title);
    query.bindValue(":artist", saveMeta.artist);
    query.bindValue(":album", saveMeta.album);
    query.bindValue(":py_title", saveMeta.pinyinTitle);
    query.bindValue(":py_title_short", saveMeta.pinyinTitleShort);
    query.bindValue(":py_artist", saveMeta.pinyinArtist);
    query.bindValue(":py_artist_short", saveMeta.pinyinArtistShort);
    query.bindValue(":py_album", saveMeta.pinyinAlbum);
    query.bindValue(":py_album_short", saveMeta.pinyinAlbumShort);
    query.bindValue(":hash", saveMeta.hash);

    if (! query.exec()) {
        qCritical() << query.lastError();
        return;
    }
    qDebug() << "updateMusicMeta end";
}

void MediaDatabaseWriter::updateMusicMetaList(const MusicMetaList &metalist)
{

}

void MediaDatabaseWriter::removeMusicMeta(const MusicMeta &meta)
{
    QSqlQuery query;
    QString sqlstring = QString("DELETE FROM music WHERE hash = '%1'").arg(meta.hash);
    if (! query.exec(sqlstring)) {
        qWarning() << query.lastError();
        return;
    }
}

void MediaDatabaseWriter::removeMusicMetaList(const MusicMetaList &metalist)
{
    QSqlDatabase::database().transaction();
    for (auto &meta : metalist) {
        removeMusicMeta(meta);
    }
    QSqlDatabase::database().commit();
}


void MediaDatabaseWriter::addMusicMeta(const MusicMeta &metalist)
{
//    qDebug() << "addMusicMeta beign";
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
    query.bindValue(":localpath", metalist.localPath);
    query.bindValue(":length", metalist.length);
    query.bindValue(":py_title", metalist.pinyinTitle);
    query.bindValue(":py_title_short", metalist.pinyinTitleShort);
    query.bindValue(":py_artist", metalist.pinyinArtist);
    query.bindValue(":py_artist_short", metalist.pinyinArtistShort);
    query.bindValue(":py_album", metalist.pinyinAlbum);
    query.bindValue(":py_album_short", metalist.pinyinAlbumShort);
    query.bindValue(":cuepath", metalist.cuePath);

    if (! query.exec()) {
        qCritical() << query.lastError();
        return;
    }
//    qDebug() << "addMusicMeta end";
}


void MediaDatabaseWriter::insertMusic(const MusicMeta &meta,
                                      const PlaylistMeta &playlistMeta)
{
//    qDebug() << "insertMusic begin";
    QSqlQuery query;
    QString sqlstring = QString("INSERT INTO playlist_%1 "
                                "(music_id, playlist_id, sortid) "
                                "SELECT :music_id, :playlist_id, :sort_id "
                                "WHERE NOT EXISTS("
                                "SELECT * FROM playlist_%1 "
                                "WHERE music_id = :music_id)").arg(playlistMeta.uuid);
    query.prepare(sqlstring);
    query.bindValue(":playlist_id", playlistMeta.uuid);
    query.bindValue(":music_id", meta.hash);
    query.bindValue(":sort_id", 0);

    if (! query.exec()) {
        qCritical() << query.lastError() << sqlstring;
        return;
    }
//    qDebug() << "insertMusic end";
}

void MediaDatabaseWriter::insertMusicList(const MusicMetaList &metalist, const PlaylistMeta &playlistMeta)
{
//    qDebug() << "insertMusicList beign";
    QSqlDatabase::database().transaction();
    for (auto &meta : metalist) {
        insertMusic(meta, playlistMeta);
    }
    QSqlDatabase::database().commit();
//    qDebug() << "insertMusicList end";
}
