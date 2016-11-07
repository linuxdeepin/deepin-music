/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef INOTIFYENGINE_H
#define INOTIFYENGINE_H

#include <QObject>
#include <QScopedPointer>

struct inotify_event;
class QDir;

class InotifyEnginePrivate;
class InotifyEngine : public QObject
{
    Q_OBJECT
public:
    explicit InotifyEngine(QObject *parent = 0);
    ~InotifyEngine();

signals:
    void fileRemoved(const QString &filepath);

public slots:
    void addPath(const QString &path);
    void addPaths(const QStringList &paths);

private slots:
    void readFromInotify(int);
    void handleInotifyEvent(inotify_event *);

private:
    bool watchDirectory(QDir& dir);
    void watchAllDirectory(const QString &path);
    QScopedPointer<InotifyEnginePrivate> d;
};

#endif // INOTIFYENGINE_H
