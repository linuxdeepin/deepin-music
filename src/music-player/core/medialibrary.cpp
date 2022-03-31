/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     ZouYa <zouya@uniontech.com>
 *
 * Maintainer: WangYu <wangyu@uniontech.com>
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

#include "medialibrary.h"

#include <QDebug>
#include <QFileInfo>
#include <QDirIterator>

#include <mediameta.h>
#include <metadetector.h>
#include "util/global.h"

#ifdef SUPPORT_INOTIFY
#include "util/inotifyengine.h"
#endif

#include "player.h"

MediaLibrary::MediaLibrary(QObject *parent) : QObject(parent)
{
}


MediaLibrary::~MediaLibrary()
{

}

MediaMeta MediaLibrary::creatMediaMeta(QString path)
{
    MediaMeta mediaMeta;
    QFileInfo fileinfo(path);
    while (fileinfo.isSymLink()) {  //to find final target
        fileinfo.setFile(fileinfo.symLinkTarget());
    }
    auto hash = DMusic::filepathHash(fileinfo.absoluteFilePath());
    mediaMeta.hash = hash;
    mediaMeta = MetaDetector::getInstance()->updateMetaFromLocalfile(mediaMeta, fileinfo, Global::playbackEngineType());

    return mediaMeta;
}

QStringList MediaLibrary::getSupportedSuffixs()
{
    return  Player::getInstance()->supportedSuffixList();
}

void MediaLibrary::init()
{
    MetaDetector::getInstance();
}
