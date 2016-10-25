/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "mediafilemonitor.h"

#include <QMap>
#include <QDir>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QCryptographicHash>
#include <QDirIterator>
#include <QDebug>
#include <QApplication>

#include <tag.h>
#include <fileref.h>

#include "playlist.h"

static QMap<QString, bool>  sSupportedSuffix;
static QStringList          sSupportedSuffixList;
static QStringList          sSupportedFiterList;

QStringList MediaFileMonitor::supportedFilterStringList()
{
    return sSupportedFiterList;
}

MediaFileMonitor::MediaFileMonitor(QObject *parent) : QObject(parent)
{
    QMimeDatabase mdb;
    for (auto &mt : mdb.allMimeTypes()) {
        if (mt.name().startsWith("audio/")) {
            sSupportedFiterList <<  mt.filterString();
            for (auto &suffix : mt.suffixes()) {
                sSupportedSuffixList << "*." + suffix;
                sSupportedSuffix.insert(suffix, true);
            }
        }
    }
}
#include <QThread>
void MediaFileMonitor::importPlaylistFiles(QSharedPointer<Playlist> playlist, const QStringList &filelist)
{
    QStringList urllist;

    for (auto &filepath : filelist) {
        QDirIterator it(filepath, sSupportedSuffixList, QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            urllist << it.next();
        }
    }

    if (urllist.empty()) {
        qCritical() << "can not find meida file";
        return;
    }

    for (auto &url : urllist) {
        TagLib::FileRef f(url.toStdString().c_str());

        // TODO: fix me in windows
        if (f.isNull()) {
            qDebug() << url;
            continue;
        }

        MusicInfo info;
        info.url = url;
        info.id = QString(QCryptographicHash::hash(info.url.toUtf8(), QCryptographicHash::Md5).toHex());
        info.title = QString::fromUtf8(f.tag()->title().toCString(true));
        info.artist = QString::fromUtf8(f.tag()->artist().toCString(true));
        info.album = QString::fromUtf8(f.tag()->album().toCString(true));
        info.length = f.audioProperties()->length();
        info.size = f.file()->length();
        info.filetype =  QFileInfo(url).suffix();

        if (info.title.isEmpty()) {
            info.title = QFileInfo(url).baseName();
        }

        if (info.artist.isEmpty()) {
            info.artist = tr("Unknow Artist");
        }

        if (info.album.isEmpty()) {
            info.album = tr("Unknow Album");
        }

        qDebug() << QThread::currentThread() << qApp->thread();
        QThread::msleep(200);
        emit meidaFileImported(playlist, info);
    }
}
