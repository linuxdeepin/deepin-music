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
#include <QFile>
#include <QHash>
#include <QDebug>
#include <QAudioBuffer>
#include <QMediaContent>

#include <tag.h>
#include <fileref.h>
#include <taglib.h>
#include <tpropertymap.h>

#ifdef __cplusplus
extern "C" {
#endif
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#ifdef __cplusplus
}
#endif

#include "pinyin.h"
#include "icu.h"
#include "cueparser.h"

namespace MusicMetaName
{

QList<QByteArray> detectCodec(const MusicMeta &info)
{
    if (info.cuePath.isEmpty()) {
#ifdef _WIN32
        // TODO: fix me in windows
        TagLib::FileRef f(info.localPath.toStdWString().c_str());
#else
        TagLib::FileRef f(info.localPath.toStdString().c_str());
#endif
        TagLib::Tag *tag = f.tag();
        if (tag) {
            bool encode = true;
            encode &= tag->title().isNull() ? true : tag->title().isLatin1();
            encode &= tag->artist().isNull() ? true : tag->artist().isLatin1();
            encode &= tag->album().isNull() ? true : tag->album().isLatin1();
            auto detectString = tag->album() + tag->artist() + tag->title();
            auto detectByte = QByteArray(detectString.toCString());
            return ICU::codeName(detectByte);
        } else {
            return QList<QByteArray>();
        }
    } else {
        QFile cueFile(info.cuePath);
        if (!cueFile.open(QIODevice::ReadOnly)) { return QList<QByteArray>(); }

        QByteArray cueByte = cueFile.readAll();
        cueFile.close();
        return ICU::codeName(cueByte);
    }
    return QList<QByteArray>();
}

void updateCodec(MusicMeta &meta, QByteArray codecName)
{
    if (meta.cuePath.isEmpty()) {
#ifdef _WIN32
        // TODO: fix me in windows
        TagLib::FileRef f(info.localPath.toStdWString().c_str());
#else
        TagLib::FileRef f(meta.localPath.toStdString().c_str());
#endif
        TagLib::Tag *tag = f.tag();
        if (tag) {
            bool encode = true;
            encode &= tag->title().isNull() ? true : tag->title().isLatin1();
            encode &= tag->artist().isNull() ? true : tag->artist().isLatin1();
            encode &= tag->album().isNull() ? true : tag->album().isLatin1();

            QTextCodec *codec = QTextCodec::codecForName(codecName);
            if (encode && codec) {
                meta.album = codec->toUnicode(tag->album().toCString());
                meta.artist = codec->toUnicode(tag->artist().toCString());
                meta.title = codec->toUnicode(tag->title().toCString());
                qDebug() << meta.album << meta.artist  << meta.title;
            }

            qDebug() << f.file()->name();
            if (meta.title.isEmpty() && f.file()) {
                meta.title = f.file()->name();
            }
        }
    } else {
//        qDebug() << "check" << meta.cuePath;
        CueParser parser(meta.cuePath, codecName);
        for (auto cueMeta : parser.metalist) {
//            qDebug() << "check" << cueMeta.hash << meta.hash;
            if (cueMeta.hash == meta.hash) {
                meta.title = cueMeta.title;
                meta.artist = cueMeta.artist;
                meta.album = cueMeta.album;
            }
        }
    }
}


MusicMeta fromLocalFile(const QFileInfo &fileInfo, const QString &hash)
{
    MusicMeta info;
    info.localPath = fileInfo.absoluteFilePath();
    info.hash = hash;

    if (info.localPath.isEmpty()) {
        return info;
    }

#ifdef _WIN32
    // TODO: fix me in windows
    TagLib::FileRef f(info.localPath.toStdWString().c_str());
#else
    TagLib::FileRef f(info.localPath.toStdString().c_str());
#endif

    // TODO: more encode support
    TagLib::Tag *tag = f.tag();

    if (tag) {
        bool encode = true;
        encode &= tag->title().isNull() ? true : tag->title().isLatin1();
        encode &= tag->artist().isNull() ? true : tag->artist().isLatin1();
        encode &= tag->album().isNull() ? true : tag->album().isLatin1();

//         WARING: icu detect is useless because the sample is to small !!!!!

        auto detectString = tag->album() + tag->artist() + tag->title();
        auto detectByte = QByteArray(detectString.toCString());
        QByteArray codeName = ICU::codeName(detectByte).value(0);
        QTextCodec *codec = QTextCodec::codecForName(codeName);

//                    Localized encode, current only GB18030 is used.
//        QTextCodec *codec = QTextCodec::codecForName("GB18030");

        if (encode && codec) {
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

    {
        av_register_all();

        AVFormatContext *pFormatCtx = avformat_alloc_context();
        avformat_open_input(&pFormatCtx, info.localPath.toStdString().c_str(), NULL, NULL);
        if (pFormatCtx) {
//            AVDictionaryEntry *tag = NULL;
            avformat_find_stream_info(pFormatCtx, NULL);
//            while ((tag = av_dict_get(pFormatCtx->metadata, "", tag, AV_DICT_IGNORE_SUFFIX))) {
//                qDebug() << tag->key << "=" << tag->value;
//            }
            int64_t duration = pFormatCtx->duration / 1000;
            info.length = duration;
        }

        avformat_close_input(&pFormatCtx);
        avformat_free_context(pFormatCtx);
    }

    if (f.audioProperties() && 0 == info.length) {
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
