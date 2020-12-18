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
        if (filepath.isEmpty()) {
            continue;
        }
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


