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

#ifndef INOTIFYENGINE_H
#define INOTIFYENGINE_H

#include <QObject>
#include <QMutex>
#include <QScopedPointer>

class QDir;
class InotifyFilesPrivate;
class InotifyFiles : public QObject
{
    Q_OBJECT
public:
    explicit InotifyFiles(QObject *parent = 0);
    ~InotifyFiles();

    void start();

signals:
    void fileChanged(const QStringList &files);

public slots:
    void clear();
    void addPath(const QString &path);
    void addPaths(const QStringList &paths);
    void scanFiles();

private:
    QScopedPointer<InotifyFilesPrivate> d;
    QMutex  m_mutex;
};

#endif // INOTIFYENGINE_H
