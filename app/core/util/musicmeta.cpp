/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "musicmeta.h"

#include <QCryptographicHash>
#include <QTextCodec>
#include <QTime>
#include <QHash>

#include <tag.h>
#include <fileref.h>
#include <taglib.h>
#include <tpropertymap.h>

#include "pinyin.h"

namespace MusicMetaName
{

MusicMeta fromLocalFile(const QFileInfo &fileInfo, const QString &hash)
{
    MusicMeta info;
    info.localpath = fileInfo.absoluteFilePath();
    info.hash = hash;

    if (info.localpath.isEmpty()) {
        return info;
    }

    // TODO: fix me in windows
#ifdef _WIN32
    TagLib::FileRef f(info.localpath.toStdWString().c_str());
#else
    TagLib::FileRef f(info.localpath.toStdString().c_str());
#endif

    // TODO: more encode support
    TagLib::Tag *tag = f.tag();

    if (tag) {
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
    }

    if (f.audioProperties()) {
        // msec
        info.length = f.audioProperties()->length() * 1000;
    }

    if (f.file()) {
        info.size = f.file()->length();
    } else {
        info.size = fileInfo.size();
    }

    auto current = QDateTime::currentDateTime();
    // HACK how to sort by add time
    info.timestamp = current.toMSecsSinceEpoch() * 1000;
    info.filetype = fileInfo.suffix();

    if (info.title.isEmpty()) {
        info.title = fileInfo.baseName();
    }

    pinyinIndex(info);

    return info;
}

QString hash(const QString &hash)
{
    return QString(QCryptographicHash::hash(hash.toUtf8(), QCryptographicHash::Md5).toHex());
}

void pinyinIndex(MusicMeta &info)
{
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
}

};
