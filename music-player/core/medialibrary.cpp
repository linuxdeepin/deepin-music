/*
 * Copyright (C) 2017 ~ 2018 Wuhan Deepin Technology Co., Ltd.
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

#include "medialibrary.h"

#include <QDebug>
#include <QFileInfo>
#include <QDirIterator>


#include <util/cueparser.h>
#include <mediameta.h>
#include <metadetector.h>

#ifdef SUPPORT_INOTIFY
#include "util/inotifyengine.h"
#endif

#include "player.h"
#include "mediadatabase.h"

const static int ScanCacheSize = 50;

class MediaLibraryPrivate
{
public:
    MediaLibraryPrivate(MediaLibrary *parent) : q_ptr(parent)
    {
        losslessSuffixs.insert("flac", true);
        losslessSuffixs.insert("ape", true);
        losslessSuffixs.insert("wav", true);

        auto suffixList = Player::instance()->supportedSuffixList();
        for (auto suffix : suffixList) {
            supportedSuffixs.insert(suffix, true);
        }

//        qDebug() << supportedSuffixs;

#ifdef SUPPORT_INOTIFY
        watcher = new InotifyEngine;
#endif
    }

    MetaPtr createMeta(const QFileInfo &fileInfo);

    MetaPtr importMeta(const QString &filepath,
                       QMap<QString, MetaPtr> &losslessMetaCache,
                       QList<DMusic::CueParserPtr> &cuelist);

    void startMonitor()
    {
        auto metalist = MediaDatabase::instance()->allmetas();
        QMap<QString, QString> dirs;
        for (auto &meta : metalist) {
            QFileInfo metafi(meta.localPath);
            metas.insert(meta.hash, MetaPtr(new MediaMeta(meta)));
            dirs.insert(metafi.absolutePath(), metafi.absolutePath());
        }

#ifdef SUPPORT_INOTIFY
        watcher->addPaths(dirs.keys());
#endif

//        for (auto meta : metas) {
//            qDebug() << meta->title;
//        }
    }

private:
    QHash<QString, bool>    losslessSuffixs;
    QHash<QString, bool>    supportedSuffixs;
    QMap<QString, MetaPtr>  metas;

#ifdef SUPPORT_INOTIFY
    InotifyEngine  *watcher;
#endif
    MediaLibrary *q_ptr;
    Q_DECLARE_PUBLIC(MediaLibrary)
};

MetaPtr MediaLibraryPrivate::createMeta(const QFileInfo &fileinfo)
{
    auto hash = DMusic::filepathHash(fileinfo.absoluteFilePath());
    if (metas.contains(hash)) {
        return metas.value(hash);
    }

    auto meta = MetaPtr(new MediaMeta);
    meta->hash = hash;
    MetaDetector::updateMetaFromLocalfile(meta.data(), fileinfo);
    return meta;
}

MetaPtr MediaLibraryPrivate::importMeta(const QString &filepath,
                                        QMap<QString, MetaPtr> &losslessMetaCache,
                                        QList<DMusic::CueParserPtr> &cuelist)
{
    QFileInfo fileInfo(filepath);
    auto suffix = QString("*.%1").arg(fileInfo.suffix()).toLower();
    if (!supportedSuffixs.contains(suffix)) {
        qWarning() << "skip" << suffix << filepath << supportedSuffixs;
        return MetaPtr();
    }

    if (fileInfo.suffix().toLower() == "cue") {
        cuelist << DMusic::CueParserPtr(new DMusic::CueParser(filepath));
        // TODO: check cue invalid
#ifdef SUPPORT_INOTIFY
        watcher->addPath(fileInfo.absolutePath());
#endif
        qWarning() << "skip" << suffix << filepath;
        return MetaPtr();
    }

    auto hash = DMusic::filepathHash(filepath);
    if (MediaLibrary::instance()->contains(hash)) {
        // FIXME: insertToPlaylist;
        //emit insertToPlaylist(hash, playlist);
        qDebug() << "exit" << hash << MediaLibrary::instance()->meta(hash);
        return MediaLibrary::instance()->meta(hash);
    }

    auto meta = createMeta(fileInfo);

    //check is lossless file
    if (losslessSuffixs.contains(fileInfo.suffix())) {
        losslessMetaCache.insert(meta->localPath, meta);
        return MetaPtr();
    }

    metas.insert(meta->hash, meta);
#ifdef SUPPORT_INOTIFY
    watcher->addPath(fileInfo.absolutePath());
#endif
    return meta;
}

MediaLibrary::MediaLibrary(QObject *parent) :
    QObject(parent), d_ptr(new MediaLibraryPrivate(this))
{
}


MediaLibrary::~MediaLibrary()
{

}

bool MediaLibrary::isEmpty() const
{
    Q_D(const MediaLibrary);
    return d->metas.isEmpty();
}

MetaPtr MediaLibrary::meta(const QString &hash)
{
    Q_D(MediaLibrary);
    return d->metas.value(hash);
}

bool MediaLibrary::contains(const QString &hash) const
{
    Q_D(const MediaLibrary);
    return d->metas.contains(hash);
}

MetaPtrList MediaLibrary::importFile(const QString &filepath)
{
    Q_D(MediaLibrary);
    qDebug() << "import file" << filepath;

    QMap<QString, MetaPtr>          losslessMetaCache;
    QList<DMusic::CueParserPtr>     cuelist;
    MetaPtrList                     metaList;

    auto meta = d->importMeta(filepath, losslessMetaCache, cuelist);
    if (!meta.isNull()) {
        metaList << meta;
    }

    for (auto &cue : cuelist) {
        losslessMetaCache.remove(cue->mediaFilepath());
        metaList += cue->metalist();
        // insert to library
        for (auto meta : cue->metalist()) {
            d->metas.insert(meta->hash, meta);
        }
    }

    for (auto &key : losslessMetaCache.keys()) {
        auto losslessMeta = losslessMetaCache.value(key);
#ifdef SUPPORT_INOTIFY
        d->watcher->addPath(losslessMeta->localPath);
#endif
        metaList << losslessMeta;
        d->metas.insert(losslessMeta->hash, losslessMeta);
    }


    qDebug() << "importFile" << "with media count:" << metaList.length();
    emit MediaDatabase::instance()->addMediaMetaList(metaList);
    return metaList;
}

void MediaLibrary::init()
{
    Q_D(MediaLibrary);
    d->startMonitor();
    MetaDetector::init();
}

void MediaLibrary::removeMediaMetaList(const MetaPtrList metalist)
{
    Q_D(MediaLibrary);
    for (auto meta : metalist) {
        if (meta.isNull()) {
            qCritical() << "invalid meta" << meta;
        }

        d->metas.remove(meta->hash);
    }
}

void MediaLibrary::importMedias(const QString &jobid, const QStringList &urllist)
{
    Q_D(MediaLibrary);
    qDebug() << "import form" << urllist << "to" << jobid;

    int                             mediaCount  = 0;
    QMap<QString, MetaPtr>          losslessMetaCache;
    QList<DMusic::CueParserPtr>     cuelist;
    MetaPtrList                     metaCache;

    for (auto &filepath : urllist) {
        QFileInfo fileInfo(filepath);
        if (fileInfo.isDir()) {
            QDirIterator it(filepath, d->supportedSuffixs.keys(),
                            QDir::Files, QDirIterator::Subdirectories);
            while (it.hasNext()) {
                QString  filepath = it.next();

                auto meta = d->importMeta(filepath, losslessMetaCache, cuelist);
                qDebug() << "process file" << filepath << meta;
                if (meta.isNull()) {
                    qWarning() << "create meta failed:" << filepath << meta;
                    continue;
                }

                metaCache << meta;
                if (metaCache.length() >= ScanCacheSize) {
                    mediaCount += metaCache.length();
                    emit MediaDatabase::instance()->addMediaMetaList(metaCache);
                    emit meidaFileImported(jobid, metaCache);
                    metaCache.clear();
                }
            }
        } else {
            auto meta = d->importMeta(filepath, losslessMetaCache, cuelist);
            if (meta.isNull()) {
                continue;
            }

            metaCache << meta;
            if (metaCache.length() >= ScanCacheSize) {
                mediaCount += metaCache.length();
                emit MediaDatabase::instance()->addMediaMetaList(metaCache);
                emit meidaFileImported(jobid, metaCache);
                metaCache.clear();
            }
        }
    }

    for (auto &cue : cuelist) {
        losslessMetaCache.remove(cue->mediaFilepath());
        metaCache += cue->metalist();
        // insert to library

        for (auto meta : cue->metalist()) {
            d->metas.insert(meta->hash, meta);
        }

        if (metaCache.length() >= ScanCacheSize) {
            mediaCount += metaCache.length();
            emit MediaDatabase::instance()->addMediaMetaList(metaCache);
            emit meidaFileImported(jobid, metaCache);
            metaCache.clear();
        }
    }

    for (auto &key : losslessMetaCache.keys()) {
        auto losslessMeta = losslessMetaCache.value(key);

#ifdef SUPPORT_INOTIFY
        d->watcher->addPath(losslessMeta->localPath);
#endif
        metaCache << losslessMeta;

        d->metas.insert(losslessMeta->hash, losslessMeta);
        if (metaCache.length() >= ScanCacheSize) {
            mediaCount += metaCache.length();
            emit MediaDatabase::instance()->addMediaMetaList(metaCache);
            emit meidaFileImported(jobid, metaCache);
            metaCache.clear();
        }
    }

    if (metaCache.length() > 0) {
        mediaCount += metaCache.length();
        emit MediaDatabase::instance()->addMediaMetaList(metaCache);
        emit meidaFileImported(jobid, metaCache);
        metaCache.clear();
    }

    qDebug() << "scanFinished" << jobid << "with media count:" << mediaCount;
    emit scanFinished(jobid, mediaCount);
}

