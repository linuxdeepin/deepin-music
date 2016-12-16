/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

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
