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
#include <QProcess>

convertThread::convertThread()
{
}

convertThread::~convertThread()
{
}

void convertThread::run()
{

    while (true) {

        if (m_infos.isEmpty()) {
            break;
        }

        musicInfo info = m_infos.takeFirst();

        //convet ape to mp3
        apeToMp3(info.mpath, info.mhash);
        QString curPath = Global::cacheDir();
        QString oldPath = QString("%1/images/%2.mp3").arg(curPath).arg(info.mhash + "tmp");
        QString newPath = QString("%1/images/%2.mp3").arg(curPath).arg(info.mhash);
        qDebug() << "convert:" << QFile::rename(oldPath, newPath);
    }
}

void convertThread::addApeandAmr(const QString &path, const QString &hash)
{
    m_infos.append(musicInfo(path, hash));
}

void convertThread::apeToMp3(QString path, QString hash)
{
    QFileInfo fileInfo(path);
    if (fileInfo.suffix().toLower() == "ape") {
        QString curPath = Global::cacheDir();
        QString toPath = QString("%1/images/%2.mp3").arg(curPath).arg(hash + "tmp");

        QString fromPath = QString("%1/.tmp1.ape").arg(curPath);
        QFile::remove(fromPath);
        QFile file(path);
        file.link(fromPath);
        QString program = QString("ffmpeg -i %1  -ac 1 -ab 32 -ar 24000 %2").arg(fromPath).arg(toPath);
        QProcess::execute(program);
    }
}

