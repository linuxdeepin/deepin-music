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
#ifndef DBOPERATE_H
#define DBOPERATE_H

#include <QObject>
#include <QDateTime>
#include <QMutex>
#include <QDebug>
#include <QSqlDatabase>

class QSqlDatabase;
class MediaMeta;
class MediaLibrary;
class DBOperate : public QObject
{
    Q_OBJECT
public:
    explicit DBOperate(QObject *parent = nullptr);
    ~DBOperate();
public slots:
    void     setThreadShouldStop();
    void     slotImportMedias(const QStringList &urllist);
    void     slotCreatCoverImg(const QList<MediaMeta> &metas);
private:

signals:
    void     fileIsNotExist(QString imagepath);

    void     sigImportMetaFromThread(MediaMeta meta);
    // 导入成功结束
    void     sigImportFinished();
    // 导入失败，含不支持的文件
    void     sigImportFailed();
    void     sigCreatOneCoverImg(MediaMeta meta);
public:
private:
    MediaLibrary     *m_mediaLibrary = nullptr;
};

#endif // DBOPERATE_H
