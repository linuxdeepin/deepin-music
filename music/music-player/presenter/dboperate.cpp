/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
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

DBOperate::DBOperate(QObject *parent)
{
    Q_UNUSED(parent);
}

DBOperate::~DBOperate()
{

}

void DBOperate::slotGetAllMediaMeta()
{
    QTime t;
    t.start();
    QList<MediaMeta> allMediaMeta;
    QString queryStringNew = QString("SELECT hash, localpath, title, artist, album, "
                                     "filetype, track, offset, length, size, "
                                     "timestamp, invalid, search_id, cuepath, "
                                     "lyricPath, codec "
                                     "FROM musicNew");
    QSqlQuery queryNew(DataBaseService::getInstance()->getDatabase());
    queryNew.prepare(queryStringNew);
    if (! queryNew.exec()) {
        qCritical() << queryNew.lastError();
        emit sigGetAllMediaMetaFromThread(allMediaMeta);
    }

    while (queryNew.next()) {
        MediaMeta meta;
        meta.hash = queryNew.value(0).toString();
        meta.localPath = queryNew.value(1).toString();
        meta.title = queryNew.value(2).toString();
        meta.singer = queryNew.value(3).toString();
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
        allMediaMeta << meta;
    }
    qDebug() << "zy------emit sigGetAllMediaMetaFromThread t = " << t.elapsed() <<
             " " << QTime::currentTime().toString("hh:mm:ss.zzz");
    emit sigGetAllMediaMetaFromThread(allMediaMeta);
}

void DBOperate::setThreadShouldStop()
{
}

void DBOperate::slotImportMedias(const QStringList &urllist)
{
    qDebug() << "------DBOperate::slotImportMedias  currentThread = " << QThread::currentThread();
    if (m_mediaLibrary == nullptr) {
        m_mediaLibrary = MediaLibrary::getInstance();
        m_mediaLibrary->init();
    }
    for (auto &filepath : urllist) {
        QFileInfo fileInfo(filepath);
        if (fileInfo.isDir()) {
            QDirIterator it(filepath, m_mediaLibrary->getSupportedSuffixs().keys(),
                            QDir::Files, QDirIterator::Subdirectories);
            while (it.hasNext()) {
                QString  strtp = it.next();
                MediaMeta mediaMeta = m_mediaLibrary->creatMediaMeta(strtp);
                mediaMeta.updateSearchIndex();
                if (mediaMeta.album.isEmpty()) {
                    mediaMeta.album = tr("Unknown album");
                }
                if (mediaMeta.singer.isEmpty()) {
                    mediaMeta.singer = tr("Unknown artist");
                }
                emit sigImportMetaFromThread(mediaMeta);
            }
        } else {
            QString strtp = filepath;
            MediaMeta mediaMeta = m_mediaLibrary->creatMediaMeta(strtp);
            mediaMeta.updateSearchIndex();
            if (mediaMeta.album.isEmpty()) {
                mediaMeta.album = tr("Unknown album");
            }
            if (mediaMeta.singer.isEmpty()) {
                mediaMeta.singer = tr("Unknown artist");
            }
            emit sigImportMetaFromThread(mediaMeta);
        }
    }

    emit sigImportFinished();
}

void DBOperate::slotCreatCoverImg(const QList<MediaMeta> &metas)
{
    qDebug() << "------DBOperate::slotCreatCoverImg  currentThread = " << QThread::currentThread();
    for (MediaMeta meta : metas) {
        //没有加载过的文件才去解析数据
        if (meta.hasimage) {
            meta.getCoverData(Global::cacheDir());
            QFileInfo coverInfo(Global::cacheDir() + "/images/" + meta.hash + ".jpg");
            if (coverInfo.exists()) {
                meta.hasimage = true;
            } else {
                meta.hasimage = false;
            }
            emit sigCreatOneCoverImg(meta);
        }
    }
}


