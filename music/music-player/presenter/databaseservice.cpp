/*
* Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
*
* Author: Zhang Hao<zhanghao@uniontech.com>
*
* Maintainer: Zhang Hao <zhanghao@uniontech.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program. If not, see <http://www.gnu.org/licenses/>.
*/
#include "databaseservice.h"
#include <QDebug>
#include <QDir>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
//#include <QSharedPointer>

#include "../util/global.h"
#include "mediameta.h"
//#include "playlistmeta.h"
static const QString DatabaseUUID = "0fcbd091-2356-161c-9026-f49779f9c71c40";

int databaseVersionNew();

int updateDatabaseVersionNew(int version);

void megrateToVserionNew_0();

void megrateToVserionNew_1();

typedef void (*MargeFunctionn)();

void margeDatabaseNew();

DataBaseService *DataBaseService::instance = nullptr;
DataBaseService *DataBaseService::getInstance()
{
    if (nullptr == instance) {
        instance = new DataBaseService();
    }
    return instance;
}

QList<MediaMeta> DataBaseService::allMusicInfos()
{
    if (m_MediaMeta.size() > 0) {
        return m_MediaMeta;
    } else {
        QSqlQuery queryNew;
        QList<QString> allHash;
        QVariantList removeHash;
        QString queryStringAll = QString("select music_id from playlist_all");

        queryNew.prepare(queryStringAll);
        if (!queryNew.exec()) {
            qCritical() << queryNew.lastError();
        }

        while (queryNew.next()) {
            allHash << queryNew.value(0).toString();
        }

        QString queryStringNew = QString("SELECT hash, localpath, title, artist, album, "
                                         "filetype, track, offset, length, size, "
                                         "timestamp, invalid, search_id, cuepath, "
                                         "lyricPath, codec "
                                         "FROM musicNew");

        queryNew.prepare(queryStringNew);
        if (! queryNew.exec()) {
            qCritical() << queryNew.lastError();
            return m_MediaMeta;
        }

        while (queryNew.next()) {
            QString musicHash = queryNew.value(0).toString();
            if (!allHash.contains(musicHash)) {
                removeHash.append(musicHash);
                continue;
            }
            MediaMeta meta;
            meta.hash = queryNew.value(0).toString();
            meta.localPath = queryNew.value(1).toString();
            meta.title = queryNew.value(2).toString();
            meta.artist = queryNew.value(3).toString();
            meta.album = queryNew.value(4).toString();
            meta.filetype = queryNew.value(5).toString();
            meta.track = queryNew.value(6).toLongLong();
            meta.offset = queryNew.value(7).toLongLong();
            meta.length = queryNew.value(8).toLongLong();
            meta.size = queryNew.value(9).toLongLong();
            meta.timestamp = queryNew.value(10).toLongLong();
            meta.invalid = queryNew.value(11).toBool();
            meta.searchID = queryNew.value(12).toString();
            meta.cuePath = queryNew.value(13).toString();
            meta.lyricPath = queryNew.value(14).toString();
            meta.codec = queryNew.value(15).toString();
            m_MediaMeta << meta;
            m_MediaMetaMap[meta.hash] = meta;
        }

        if (removeHash.size() > 0) {
            //remove from musicNew
            for (QVariant var : removeHash) {
                QString queryStringRemove = QString("delete from musicNew where hash=%1").arg(var.toString());
                queryNew.prepare(queryStringRemove);
                if (!queryNew.exec()) {
                    qCritical() << queryNew.lastError();
                }
            }
        }
    }
    return m_MediaMeta;
}

QList<DataBaseService::PlaylistData> DataBaseService::allPlaylistMeta()
{
    if (m_PlaylistMeta.size() > 0) {
        return m_PlaylistMeta;
    } else {
        m_PlaylistMeta.clear();
        QSqlQuery query;
        query.prepare("SELECT uuid, displayname, icon, readonly, hide, "
                      "sort_type, order_type, sort_id FROM playlist");

        if (!query.exec()) {
            qWarning() << query.lastError();
            return m_PlaylistMeta;
        }

        while (query.next()) {
            PlaylistData playlistMeta;
            playlistMeta.uuid = query.value(0).toString();
            playlistMeta.displayName = query.value(1).toString();
            playlistMeta.icon = query.value(2).toString();
            playlistMeta.readonly = query.value(3).toBool();
            playlistMeta.hide = query.value(4).toBool();
            playlistMeta.sortType = query.value(5).toInt();
            playlistMeta.orderType = query.value(6).toInt();
            playlistMeta.sortID = query.value(7).toUInt();
            m_PlaylistMeta << playlistMeta;
        }
        return m_PlaylistMeta;
    }
}

bool DataBaseService::createConnection()
{
    QDir cacheDir(Global::cacheDir());
    if (!cacheDir.exists()) {
        cacheDir.mkpath(".");
    }
    QString cachePath = Global::cacheDir() + "/mediameta.sqlite";
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
//    db = QSqlDatabase::addDatabase("QSQLITE", "writer");
    db.setDatabaseName(cachePath);

    if (!db.open()) {
        qDebug() << db.lastError()
                 << Global::cacheDir()
                 << cachePath;
        return false;
    }
    return true;
}

DataBaseService::DataBaseService()
{
//    m_MediaMetaMap.clear();
    m_PlaylistMeta.clear();
    m_MediaMeta.clear();
    createConnection();
    margeDatabaseNew();
}

void margeDatabaseNew()
{
    QMap<int, MargeFunctionn> margeFuncs;
    margeFuncs.insert(0, megrateToVserionNew_0);
    margeFuncs.insert(1, megrateToVserionNew_1);

    int currentVersion = databaseVersionNew();

    QList<int> sortVer = margeFuncs.keys();
    qSort(sortVer.begin(), sortVer.end());

    for (auto ver : sortVer) {
        if (ver > currentVersion) {
            margeFuncs.value(ver)();
        }
    }
}

void megrateToVserionNew_1()
{
    // FIXME: remove old
    QSqlDatabase::database().transaction();
    QSqlQuery query;

    query.prepare("ALTER TABLE playlist ADD COLUMN sort_id INTEGER(32);");
    if (!query.exec()) {
        qWarning() << "sql upgrade with error:" << query.lastError().type();
    }

    updateDatabaseVersionNew(1);
    QSqlDatabase::database().commit();
}

void megrateToVserionNew_0()
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

    updateDatabaseVersionNew(0);
    QSqlDatabase::database().commit();
}

int updateDatabaseVersionNew(int version)
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

int databaseVersionNew()
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
