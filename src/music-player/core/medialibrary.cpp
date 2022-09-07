// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
