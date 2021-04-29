/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     ZouYa <zouya@uniontech.com>
 *
 * Maintainer: WangYu <wangyu@uniontech.com>
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

#include "dboperate.h"
#include <QDebug>
#include <QDir>
#include <QMutex>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QThread>
#include <QDirIterator>

#include "mediameta.h"
#include "databaseservice.h"
#include "player.h"
#include "medialibrary.h"
#include "global.h"

#define SLEEPTIME 10

DBOperate::DBOperate(QObject *parent)
{
    Q_UNUSED(parent);
    QString cachePath = Global::cacheDir() + "/mediameta.sqlite";
    m_db = QSqlDatabase::addDatabase("QSQLITE", "QSQLITEWRITE");
    m_db.setDatabaseName(cachePath);
    if (!m_db.open()) {
        qDebug() << __FUNCTION__ << m_db.lastError() << Global::cacheDir();
    }
}

DBOperate::~DBOperate()
{

}

void DBOperate::stop()
{
    m_mutex.lock();
    m_needStop = true;
    m_mutex.unlock();
}

void DBOperate::setNeedSleep()
{
    m_mutex.lock();
    m_needSleep = true;
    m_mutex.unlock();
}

void DBOperate::slotImportMedias(const QString &importHash, const QStringList &urllist)
{
    m_importHash = importHash;
    m_successCount = 0;
    m_exsitCount = 0;
    if (m_mediaLibrary == nullptr) {
        m_mediaLibrary = MediaLibrary::getInstance();
        m_mediaLibrary->init();
    }
    // 统计总共需要加载的数量
    int allCount = 0;
    for (auto &filepath : urllist) {
        if (filepath.isEmpty()) {
            continue;
        }
        QFileInfo fileInfo(filepath);
        if (fileInfo.isDir()) {
            QDirIterator it(filepath, m_mediaLibrary->getSupportedSuffixs(),
                            QDir::Files, QDirIterator::Subdirectories);
            while (it.hasNext()) {
                QString  strtp = it.next();
                allCount++;
            }
        } else {
            allCount++;
        }
    }
    qDebug() << __FUNCTION__ << "allCount = " << allCount;

    m_importFailCount = 0;
    // 包含导入成功和失败的
    double importedCount = 0;
    for (auto &filepath : urllist) {
        if (filepath.isEmpty()) {
            continue;
        }
        if (m_needStop) {
            break;
        }
        if (m_needSleep) {
            QThread::msleep(SLEEPTIME);
            m_needSleep = false;
        }
        QFileInfo fileInfo(filepath);
        if (fileInfo.isDir()) {
            QDirIterator it(filepath, m_mediaLibrary->getSupportedSuffixs(),
                            QDir::Files, QDirIterator::Subdirectories);
            while (it.hasNext()) {
                if (m_needStop) {
                    break;
                }
                if (m_needSleep) {
                    QThread::msleep(SLEEPTIME);
                    m_needSleep = false;
                }
                QString  strtp = it.next();
                MediaMeta mediaMeta = m_mediaLibrary->creatMediaMeta(strtp);
                if (mediaMeta.length <= 0) {
                    m_importFailCount++;
                    importedCount++;
                } else {
                    mediaMeta.updateSearchIndex();
                    if (mediaMeta.album.isEmpty()) {
                        mediaMeta.album = tr("Unknown album");
                    }
                    if (mediaMeta.singer.isEmpty()) {
                        mediaMeta.singer = tr("Unknown artist");
                    }
                    // 导入数据库
                    addMediaMetaToDB(mediaMeta);
                    importedCount++;
                    // 已导入百分比
                    int value = static_cast<int>(importedCount / allCount * 100);
                    if (value <= 100) {
                        emit signalImportedPercent(value);
                    }
                }
            }
        } else {
            if (m_needStop) {
                break;
            }
            if (m_needSleep) {
                QThread::msleep(SLEEPTIME);
                m_needSleep = false;
            }
            QString strtp = filepath;

            if (!m_mediaLibrary->getSupportedSuffixs().contains(("*." + fileInfo.suffix().toLower()))) { //歌曲文件后缀使用小写比较
                m_importFailCount++;
                importedCount++;
                continue;
            }
            MediaMeta mediaMeta = m_mediaLibrary->creatMediaMeta(strtp);
            if (mediaMeta.length <= 0) {
                m_importFailCount++;
            } else {
                mediaMeta.updateSearchIndex();
                if (mediaMeta.album.isEmpty()) {
                    mediaMeta.album = tr("Unknown album");
                }
                if (mediaMeta.singer.isEmpty()) {
                    mediaMeta.singer = tr("Unknown artist");
                }
                // 导入数据库
                addMediaMetaToDB(mediaMeta);
                importedCount++;
                // 已导入百分比
                int value = static_cast<int>(importedCount / allCount * 100);
                if (value <= 100) {
                    emit signalImportedPercent(value);
                }
            }
        }
    }

    emit sigImportFinished(m_importFailCount, m_successCount, m_exsitCount);
}

void DBOperate::slotCreatCoverImg(const QList<MediaMeta> &metas)
{
    for (MediaMeta meta : metas) {
        if (m_needStop) {
            break;
        }
        if (m_needSleep) {
            QThread::msleep(SLEEPTIME);
            m_needSleep = false;
        }
        //没有加载过的文件才去解析数据

        QFileInfo coverInfo(Global::cacheDir() + "/images/" + meta.hash + ".jpg");
        if (!coverInfo.exists()) {
            meta.getCoverData(Global::cacheDir());
        }
        emit sigCreatOneCoverImg(meta);
    }
}

void DBOperate::slotRemoveSelectedSongs(const QString &curpage, const QStringList &musichashlist, bool removeFromLocal)
{
    // 从专辑等处删除，统一走删除所有逻辑
    QString removeListHash = curpage;
    if (curpage == "musicResult"
            || curpage == "album"
            || curpage == "artist"
            || curpage == "albumResult"
            || curpage == "artistResult") {
        removeListHash = "all";
    }
    //需要从本地删除
    if (removeFromLocal) {
        //遍历musicNew
        deleteMetaFromAllMusic(musichashlist, removeFromLocal);
    } else {
        if (removeListHash == "all") {
            //遍历musicNew
            deleteMetaFromAllMusic(musichashlist, removeFromLocal);
        } else {
            deleteMetaFromPlaylist(removeListHash, musichashlist);
        }
    }
    emit signalDelFinish();
}

bool DBOperate::deleteMetaFromAllMusic(const QStringList &metaHash, bool removeFromLocal)
{
    QSqlQuery query(m_db);
    QString strsql;
    QList<PlaylistDataThread> playlistMetas = allPlaylistMetaUUid();
    for (QString hash : metaHash) {
        if (m_needStop) {
            break;
        }
        if (m_needSleep) {
            QThread::msleep(SLEEPTIME);
            m_needSleep = false;
        }
        strsql = QString("DELETE FROM musicNew WHERE hash='%1'").arg(hash);
        bool isPrepare = query.prepare(strsql);
        if ((!isPrepare) || (! query.exec())) {
            qCritical() << query.lastError() << strsql;
        } else {
            QThread::msleep(10);
            emit signalRmvSong("all", hash, removeFromLocal);

            //遍历所有歌单,包含我的收藏
            for (PlaylistDataThread playlist : playlistMetas) {
                if (m_needStop) {
                    break;
                }
                if (m_needSleep) {
                    QThread::msleep(SLEEPTIME);
                    m_needSleep = false;
                }
                if (playlist.readonly != 1) {
                    deleteMetaFromPlaylist(playlist.uuid, QStringList() << hash);
                }
            }
            deleteMetaFromPlaylist("fav", QStringList() << hash);
        }
    }

    if (allMusicInfosCount() <= 0) {
        emit signalAllMusicCleared();
    }
    return true;
}

bool DBOperate::deleteMetaFromPlaylist(QString uuid, const QStringList &metaHash)
{
    QSqlQuery query(m_db);
    QString strsql;
    for (QString hash : metaHash) {
        if (m_needStop) {
            break;
        }
        if (m_needSleep) {
            QThread::msleep(SLEEPTIME);
            m_needSleep = false;
        }
        QString sqlIsExists = QString("select music_id from playlist_%1 where music_id = '%2'").arg(uuid).arg(hash);
        if (query.exec(sqlIsExists)) {
            if (query.next()) {
                strsql = QString("DELETE FROM playlist_%1 WHERE music_id='%2'").arg(uuid).arg(hash);
                bool isPrepare = query.prepare(strsql);
                if ((!isPrepare) || (! query.exec())) {
                    qCritical() << query.lastError() << strsql;
                }
                QThread::msleep(10);
                if (uuid == "fav") {
                    emit signalFavSongRemove(hash);
                }
                emit signalRmvSong(uuid, hash, false);
            }
        } else {
            qCritical() << query.lastError() << sqlIsExists;
        }
    }
    return true;
}

int DBOperate::allMusicInfosCount()
{
    int count = 0;
    QString queryString = QString("SELECT count(*) FROM musicNew");
    QSqlQuery queryNew(m_db);
    bool isPrepare = queryNew.prepare(queryString);
    if ((!isPrepare) || (!queryNew.exec())) {
        qCritical() << queryNew.lastError();
        count = 0;
    }
    while (queryNew.next()) {
        count = queryNew.value(0).toInt();
    }
    return count;
}

QList<DBOperate::PlaylistDataThread> DBOperate::allPlaylistMetaUUid()
{
    QList<PlaylistDataThread> playlistMetas;
    playlistMetas.clear();
    QSqlQuery query(m_db);
    bool isPrepare = query.prepare("SELECT uuid, readonly FROM playlist");
    if ((!isPrepare) || (!query.exec())) {
        qWarning() << query.lastError();
        return playlistMetas;
    }

    while (query.next()) {
        PlaylistDataThread playlistMeta;
        playlistMeta.uuid = query.value(0).toString();
        playlistMeta.readonly = query.value(1).toBool();
        playlistMetas << playlistMeta;
    }
    return playlistMetas;
}

void DBOperate::addMediaMetaToDB(const MediaMeta &meta)
{
    if (!this->isMediaMetaExist(meta.hash)) {
        // 导入到所有音乐
        if (addMetaToAll(meta)) {
            emit sigImportMetaFromThread(meta);
            // 添加到自定义歌单,但是当前页面上你所有音乐,则所有音乐要刷新,添加这个信号
            // 直接添加到所有音乐的,通过signalAllMusicAddOne信号刷新
            if (m_importHash != "all"
                    && m_importHash != "album" && m_importHash != "albumResult"
                    && m_importHash != "artist" && m_importHash != "artistResult"
                    && m_importHash != "musicResult" && m_importHash != "play") {
                QList<MediaMeta> metas;
                metas.append(meta);
                addMetaToPlaylist(m_importHash, metas);
            } else {
                // 如果是从播放队列导入需要发送信号通知更新播放队列
                if (m_importHash == "play") {
                    emit signalMusicAddOne("play", meta);
                }
                emit signalMusicAddOne("all", meta);
                m_successCount++;
            }
        } else {
            m_importFailCount++;
        }
    } else {
        if (m_importHash != "all"
                && m_importHash != "album" && m_importHash != "albumResult"
                && m_importHash != "artist" && m_importHash != "artistResult"
                && m_importHash != "musicResult" && m_importHash != "play") {
            QList<MediaMeta> metas;
            metas.append(meta);
            addMetaToPlaylist(m_importHash, metas);
        } else {
            m_exsitCount++;
        }
    }
}

bool DBOperate::isMediaMetaExist(const QString &hash)
{
    QSqlQuery query(m_db);
    bool isPrepare = query.prepare("SELECT COUNT(*) FROM musicNew where hash = :hash");
    query.bindValue(":hash", hash);

    if ((!isPrepare) || (!query.exec())) {
        qWarning() << query.lastError();
        return false;
    }

    if (query.next()) {
        return query.value(0).toInt() > 0;
    }

    return false;
}

int DBOperate::addMetaToPlaylist(QString uuid, const QList<MediaMeta> &metas)
{
    int insert_count = 0;

    for (MediaMeta meta : metas) {
        if (m_needStop) {
            break;
        }
        if (m_needSleep) {
            QThread::msleep(SLEEPTIME);
            m_needSleep = false;
        }
        QSqlQuery query(m_db);
        QString sqlStr = QString("SELECT * FROM playlist_%1 WHERE music_id = :music_id").arg(uuid);
        bool isPrepare = query.prepare(sqlStr);
        query.bindValue(":music_id", meta.hash);

        if (isPrepare && query.exec()) {
            if (!query.next()) {
                // 不存在则添加
                sqlStr = QString("INSERT INTO playlist_%1 "
                                 "(music_id, playlist_id, sort_id) "
                                 "SELECT :music_id, :playlist_id, :sort_id ").arg(uuid);

                isPrepare = query.prepare(sqlStr);
                query.bindValue(":playlist_id", uuid);
                query.bindValue(":music_id", meta.hash);
                query.bindValue(":sort_id", 0);
                if (isPrepare && query.exec()) {
                    insert_count++;
                    m_successCount++;
                    if (uuid == "fav") {
                        emit signalFavSongAdd(meta.hash);
                    }
                    emit signalMusicAddOne(m_importHash, meta);
                } else {
                    m_importFailCount++;
                    qCritical() << query.lastError() << sqlStr;
                }
            } else {
                // 已存在,已存在计数器加1
                m_exsitCount++;
            }
        } else {
            // 查询是否存在失败,导入失败计数器加1
            m_importFailCount++;
            qCritical() << query.lastError() << sqlStr;
        }
    }

    return insert_count;
}

bool DBOperate::addMetaToAll(MediaMeta meta)
{
    QSqlQuery query(m_db);
    bool isPrepare = query.prepare("INSERT INTO musicNew ("
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
    query.bindValue(":hash", meta.hash);
    query.bindValue(":timestamp", meta.timestamp);
    query.bindValue(":title", meta.title);
    query.bindValue(":artist", meta.singer);
    query.bindValue(":album", meta.album);
    query.bindValue(":filetype", meta.filetype);
    query.bindValue(":size", meta.size);
    query.bindValue(":track", meta.track);
    query.bindValue(":offset", meta.offset);
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

    if ((!isPrepare) || (! query.exec())) {
        qCritical() << query.lastError();
        return false;
    }
    return true;
}


