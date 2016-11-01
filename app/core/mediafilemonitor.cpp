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

#include <tag.h>
#include <fileref.h>
#include <taglib.h>
#include <tpropertymap.h>

#include "playlist.h"
#include "mediadatabase.h"

#include "util/pinyin.h"

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

    MusicMetaList metaCache;

    for (auto &url : urllist) {
        auto hash = QString(QCryptographicHash::hash(url.toUtf8(), QCryptographicHash::Md5).toHex());

        if (MediaDatabase::instance()->musicMetaExist(hash)) {
            continue;
        }

        // TODO: fix me in windows
#ifdef _WIN32
        TagLib::FileRef f(url.toStdWString().c_str());
#else
        TagLib::FileRef f(url.toStdString().c_str());
#endif
        QFileInfo fileInfo(url);

        if (f.isNull()) {
            qWarning() << "import music file failed:" << url;
            continue;
        }

        MusicMeta info;
        info.localpath = url;
        info.hash = hash;

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

        auto current = QDateTime::currentDateTime();
        info.timestamp = current.toTime_t()
                         + static_cast<uint>(1000 + current.time().msec());
        info.length = f.audioProperties()->length();
        info.size = f.file()->length();
        info.filetype = fileInfo.suffix();

        if (info.title.isEmpty()) {
            info.title = fileInfo.baseName();
        }

        for (auto &str : Pinyin::simpleChineseSplit(info.title)) {
            info.pinyinTitle += str;
            info.pinyinTitleShort += str.at(0);
        }
        for (auto &str : Pinyin::simpleChineseSplit(info.artist)) {
            info.pinyinArtist += str;
            info.pinyinArtistShort += str.at(0);
        }
        for (auto &str : Pinyin::simpleChineseSplit(info.album)) {
            info.pinyinAlbum += str;
            info.pinyinAlbumShort += str.at(0);
        }

        metaCache << info;

        if (metaCache.length() >= 500) {
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
