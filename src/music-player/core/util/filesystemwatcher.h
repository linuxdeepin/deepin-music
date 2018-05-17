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

#ifndef FILESYSTEMWATCHER_H
#define FILESYSTEMWATCHER_H

#include <QtCore>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#define RichDirPrefix ".deepin_rich_dir_"
#define EVENT_NUM 16
#define MAX_BUF_SIZE 1024

class FileSystemWatcher : public QObject
{
    Q_OBJECT
public:
    explicit FileSystemWatcher(QObject *parent = 0);
    ~FileSystemWatcher();

    bool addPath(const QString &path);
    bool removePath(const QString &path);
//    QStringList addPaths(const QStringList &paths);
//    QStringList removePaths(const QStringList &paths);

    void addWatchFolder(const QString &path);
signals:
//    void directoryChanged(QString path);
//    void folderChanged(const QString& path);
//    void fileChanged(QString path);

//    void fileCreated(int cookie, QString path);
//    void fileModify(int cookie, QString path);
//    void fileMovedFrom(int cookie, QString path);
//    void fileMovedTo(int cookie, QString out);
//    void fileDeleted(int cookie, QString path);
//    void fileAttribChanged(int cookie, QString path);


private:
};

#endif // FILESYSTEMWATCHER_H
