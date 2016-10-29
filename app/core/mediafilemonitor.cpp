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
#include <QTextCodec>
#include <QTime>

#include <tag.h>
#include <fileref.h>
#include <taglib.h>
#include <tpropertymap.h>

#include "../../vendor/src/chinese2pinyin/chinese2pinyin.h"

inline bool isAlphabeta(const QChar &c)
{
    QRegExp re("[A-Za-z]*");
    return re.exactMatch(c);
}

inline bool isNumber(const QChar &c)
{
    QRegExp re("[0-9]*");
    return re.exactMatch(c);
}

inline bool isChinese(const QChar &c)
{
    return c.unicode() < 0x9FBF && c.unicode() > 0x4E00;
}

inline QString toChinese(const QString &c)
{
    QString pinyin = Pinyin::Chinese2Pinyin(c);
    if (pinyin.length() >= 2
            && isNumber(pinyin.at(pinyin.length() - 1))) {
        return pinyin.left(pinyin.length() - 1);
    }
    return pinyin;
}

QStringList simpleSplit(QString &pinyin)
{
    QStringList wordList;
    bool isLastAlphabeta = false;
    for (auto &c : pinyin) {
        bool isCurAlphabeta = isAlphabeta(c);
        if (isCurAlphabeta) {
            if (!isLastAlphabeta) {
                wordList << c;
            } else {
                wordList.last().append(c);
            }
            continue;
        }
        isLastAlphabeta = isCurAlphabeta;
        if (isNumber(c)) {
            wordList << c;
            continue;
        }
        if (isChinese(c)) {
            wordList << toChinese(c);
            continue;
        }
    }
    return wordList;
}

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

        // TODO: fix me in windows
#ifdef _WIN32
        TagLib::FileRef f(url.toStdWString().c_str());
#else
        TagLib::FileRef f(url.toStdString().c_str());
#endif

        if (f.isNull()) {
            qWarning() << "import music file failed:" << url;
            continue;
        }

        MusicMeta info;
        info.localpath = url;
        info.hash = id;

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
        info.timestamp = current.toTime_t()  * 1000 + current.time().msec();
        info.length = f.audioProperties()->length();
        info.size = f.file()->length();
        info.filetype =  QFileInfo(url).suffix();

//        qDebug() << info.title << info.artist << info.album
//                 << f.tag()->properties().toString().toCString(true);

        if (info.title.isEmpty()) {
            info.title = QFileInfo(url).baseName();
        }

        for (auto &str : simpleSplit(info.title)) {
            info.pinyinTitle += toChinese(str);
            info.pinyinTitleShort += str.at(0);
        }
        qDebug() << info.pinyinTitle
                 << info.pinyinTitleShort
                 << simpleSplit(info.title);

        if (info.artist.isEmpty()) {
//            info.artist = tr("Unknow Artist");
        }

        if (info.album.isEmpty()) {
//            info.album = tr("Unknow Album");
        }

//        QThread::msleep(400);
        emit meidaFileImported(playlist, info);
    }
}
