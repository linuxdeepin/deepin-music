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

#include "inotifyfiles.h"

#include <QDebug>
#include <QMap>
#include <QDir>
#include <QFileInfo>
#include <QTimer>
#include <unistd.h>

class InotifyFilesPrivate
{
public:
    QStringList                 paths;

    QTimer                      timer;
};


InotifyFiles::InotifyFiles(QObject *parent)
    : QObject(parent), d(new InotifyFilesPrivate)
{

    connect(&d->timer, &QTimer::timeout, this, &InotifyFiles::scanFiles);
}

InotifyFiles::~InotifyFiles()
{

}

void InotifyFiles::start()
{
    d->timer.start(1000);
}

void InotifyFiles::clear()
{
    d->paths.clear();
}

void InotifyFiles::addPaths(const QStringList &paths)
{
    for (auto &path : paths) {
        if (!d->paths.contains(path))
            d->paths.append(path);
    }
}

void InotifyFiles::scanFiles()
{
    QStringList allFiles;
    for (int i = d->paths.size() - 1; i >= 0; i--) {
        auto curtFile = d->paths[i];
        if(access(curtFile.toStdString().c_str(),F_OK) != 0){
//            qDebug()<<"d->activeMeta->localPath "<<curtFile;
            d->paths.removeAt(i);
            allFiles.append(curtFile);
        }
//        if (!QFile::exists(curtFile)) {
//            d->paths.removeAt(i);
//            allFiles.append(curtFile);
//        }
    }


    if (!allFiles.isEmpty())
        emit fileChanged(allFiles);


}

void InotifyFiles::addPath(const QString &path)
{
    addPaths(QStringList() << path);
}
