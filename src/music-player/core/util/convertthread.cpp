/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
*
* Author:     huangjie<huangjie@uniontech.com>
* Maintainer: huangjie <huangjie@uniontech.com>
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "convertthread.h"
#include "global.h"
#include <QFile>
#include <QFileInfo>
#include <QDebug>

extern void apeToMp3(QString path, QString hash);

convertThread::convertThread()
    : m_exit(false)
{
}

convertThread::~convertThread()
{
}

void convertThread::run()
{
    int fsize = 0;
    while (!m_exit) {
        mutex.lock();
        musicInfo info = m_infos.takeFirst();
        fsize = m_infos.size();
        mutex.unlock();
        //convet ape to mp3
        apeToMp3(info.mpath, info.mhash);
        QString curPath = Global::cacheDir();
        QString oldPath = QString("%1/images/%2.mp3").arg(curPath).arg(info.mhash + "tmp");
        QString newPath = QString("%1/images/%2.mp3").arg(curPath).arg(info.mhash);

        qDebug() << "convert:" << QFile::rename(oldPath, newPath);
        //        if(QFileInfo(oldPath).exists())
        //            QFile::remove(oldPath);

        if (fsize == 0)
            break;
    }

    if (!m_infos.isEmpty() && !m_exit) { //to forbid add task frequently,but it mostly can't be here
        start();
    } else {
        //do nothing
    }
}

void convertThread::exitToCheckFile()
{
    //close music thread
    m_exit = true;
}

void convertThread::addApeandAmr(const QString &path, const QString &hash)
{
    mutex.lock();
    m_infos.append(musicInfo(path, hash));
    mutex.unlock();
}
