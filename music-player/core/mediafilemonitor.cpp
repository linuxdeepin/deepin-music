/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "mediafilemonitor.h"

#include <QDebug>

#include <QTime>
#include <QThread>
#include <QMap>
#include <QDir>
#include <QHash>
#include <QCryptographicHash>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QDirIterator>
#include <QTextCodec>
#include <QFileSystemWatcher>

#include <tag.h>
#include <fileref.h>
#include <taglib.h>
#include <tpropertymap.h>

#include "player.h"
#include "playlist.h"
#include "mediadatabase.h"

#include "util/cueparser.h"
#include "util/musicmeta.h"
#include "util/inotifyengine.h"

MediaFileMonitor::MediaFileMonitor(QObject *parent) : QObject(parent)
{
    m_watcher = new InotifyEngine;
    connect(m_watcher, &InotifyEngine::fileRemoved,
            this, &MediaFileMonitor::fileRemoved);
}


void MediaFileMonitor::importPlaylistFiles(PlaylistPtr playlist, const QStringList &filelist)
{
    qDebug() << "import form" << filelist;
    QStringList urllist;

    QHash<QString, bool> losslessSuffix;
    losslessSuffix.insert("flac", true);
    losslessSuffix.insert("ape", true);
    losslessSuffix.insert("wav", true);

    QMap<QString, MusicMeta> losslessMetaCache;
    QList<CueParser>  cuelist;
    MusicMetaList metaCache;

    for (auto &filepath : filelist) {
        QDirIterator it(filepath, Player::instance()->supportedFilterStringList(),
                        QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            QString  url = it.next();

            QFileInfo fileInfo(url);
            if (fileInfo.suffix() == "cue") {
                cuelist << CueParser(url);
                // TODO: check cue invaild
                m_watcher->addPath(fileInfo.absolutePath());
                continue;
            }

            auto hash = QString(QCryptographicHash::hash(url.toUtf8(),
                                QCryptographicHash::Md5).toHex());
            if (MediaDatabase::instance()->musicMetaExist(hash)) {
                continue;
            }

            MusicMeta info = MusicMetaName::fromLocalFile(fileInfo, hash);

            //check is lossless file
            if (losslessSuffix.contains(fileInfo.suffix())) {
                losslessMetaCache.insert(info.localPath, info);
                continue;
            }

            metaCache << info;
            m_watcher->addPath(fileInfo.absolutePath());

            if (metaCache.length() >= ScanCacheSize) {
                emit MediaDatabase::instance()->addMusicMetaList(metaCache);
                emit meidaFileImported(playlist, metaCache);
                metaCache.clear();
            }
        }
    }

    for (auto &cue : cuelist) {
        losslessMetaCache.remove(cue.musicFilePath);
        metaCache += cue.metalist;

        if (metaCache.length() >= ScanCacheSize) {
            emit MediaDatabase::instance()->addMusicMetaList(metaCache);
            emit meidaFileImported(playlist, metaCache);
            metaCache.clear();
        }
    }

    for (auto &key : losslessMetaCache.keys()) {
        metaCache << losslessMetaCache.value(key);
        if (metaCache.length() >= ScanCacheSize) {
            emit MediaDatabase::instance()->addMusicMetaList(metaCache);
            emit meidaFileImported(playlist, metaCache);
            metaCache.clear();
        }
    }

    if (metaCache.length() > 0) {
        emit MediaDatabase::instance()->addMusicMetaList(metaCache);
        emit meidaFileImported(playlist, metaCache);
        metaCache.clear();
    }

//    qDebug() << m_watcher->directories();
    emit scanFinished();
}

void MediaFileMonitor::startMonitor()
{
    auto metalist = MediaDatabase::instance()->searchMusicMeta("", std::numeric_limits<int>::max());
    QMap<QString, QString> dirs;
    for (auto &meta : metalist) {
        QFileInfo metafi(meta.localPath);
        dirs.insert(metafi.absolutePath(), metafi.absolutePath());
    }

    m_watcher->addPaths(dirs.keys());
}
