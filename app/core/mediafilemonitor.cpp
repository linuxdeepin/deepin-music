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
#include <QThread>
#include <QApplication>

#include <tag.h>
#include <fileref.h>
#include <taglib.h>
#include <tpropertymap.h>

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
#include <QTextCodec>
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
        // TODO: do not import exist file;
        auto id = QString(QCryptographicHash::hash(url.toUtf8(), QCryptographicHash::Md5).toHex());

        TagLib::FileRef f(url.toStdString().c_str());

        // TODO: fix me in windows
        if (f.isNull()) {
            qDebug() << url;
            continue;
        }

        MusicInfo info;
        info.url = url;
        info.id = id;

        // TODO: more encode support
        TagLib::Tag *tag = f.tag();
        bool encode = true;
        encode &= tag->title().isNull() ? true : tag->title().isLatin1();
        encode &= tag->artist().isNull() ? true : tag->artist().isLatin1();
        encode &= tag->album().isNull() ? true : tag->album().isLatin1();
        if (encode) {
            // Localized encode, current only GB18030 is used.
            QTextCodec *codec = QTextCodec::codecForName("GB18030");
            info.album = codec->toUnicode(tag->album().toCString());
            info.artist = codec->toUnicode(tag->artist().toCString());
            info.title = codec->toUnicode(tag->title().toCString());
        } else {
            // UTF8 encoded.
            info.album = TStringToQString(tag->album());
            info.artist = TStringToQString(tag->artist());
            info.title = TStringToQString(tag->title());
        }

        info.length = f.audioProperties()->length();
        info.size = f.file()->length();
        info.filetype =  QFileInfo(url).suffix();

        qDebug() << info.title << info.artist << info.album
                 << f.tag()->properties().toString().toCString(true);

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
        QThread::msleep(400);
        emit meidaFileImported(playlist, info);
    }
}
