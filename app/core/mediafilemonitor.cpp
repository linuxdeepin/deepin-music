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
#include <QMap>
#include <QDir>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QCryptographicHash>
#include <QDirIterator>
#include <QThread>
#include <QTextCodec>
#include <QTime>
#include <QHash>

#include <tag.h>
#include <fileref.h>
#include <taglib.h>
#include <tpropertymap.h>

#include "playlist.h"
#include "mediadatabase.h"

#include "util/cueparser.h"
#include "util/musicmeta.h"

static QMap<QString, bool>  sSupportedSuffix;
static QStringList          sSupportedSuffixList;
static QStringList          sSupportedFiterList;

QStringList MediaFileMonitor::supportedFilterStringList()
{
    return sSupportedFiterList;
}

MediaFileMonitor::MediaFileMonitor(QObject *parent) : QObject(parent)
{
    //black list
    QHash<QString, bool> suffixBlacklist;
    suffixBlacklist.insert("m3u", true);

    QHash<QString, bool> suffixWhitelist;
    suffixWhitelist.insert("cue", true);

    QMimeDatabase mdb;
    for (auto &mt : mdb.allMimeTypes()) {
        if (mt.name().startsWith("audio/")) {
            sSupportedFiterList <<  mt.filterString();
            for (auto &suffix : mt.suffixes()) {
                if (suffixBlacklist.contains(suffix)) {
                    continue;
                }
                sSupportedSuffixList << "*." + suffix;
                sSupportedSuffix.insert(suffix, true);
            }
        }
    }

    for (auto &suffix : suffixWhitelist.keys()) {
        sSupportedSuffixList << "*." + suffix;
        sSupportedSuffix.insert(suffix, true);
    }
}

void MediaFileMonitor::importPlaylistFiles(QSharedPointer<Playlist> playlist, const QStringList &filelist)
{
    QStringList urllist;

    QHash<QString, bool> losslessSuffix;
    losslessSuffix.insert("flac", true);
    losslessSuffix.insert("ape", true);
    losslessSuffix.insert("wav", true);

    QMap<QString, MusicMeta> losslessMetaCache;
    QList<CueParser>  cuelist;
    MusicMetaList metaCache;

    for (auto &filepath : filelist) {
        QDirIterator it(filepath, sSupportedSuffixList,
                        QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            QString  url = it.next();

            QFileInfo fileInfo(url);
            if (fileInfo.suffix() == "cue") {
                cuelist << CueParser(url);
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
                losslessMetaCache.insert(info.localpath, info);
                continue;
            }

            metaCache << info;

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
}
