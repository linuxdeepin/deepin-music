/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "lyricservice.h"

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include <QScopedPointer>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDir>
#include <QFileInfo>

#include "music.h"
#include "../musicapp.h"

static QString cacheLyricPath(const MusicMeta &info)
{
    auto cacheLyricDir = MusicApp::cachePath() + "/lyric";
    return cacheLyricDir + "/" + info.hash + ".lyric";
}

inline QString cacheCoverPath(const MusicMeta &info)
{
    auto cacheLyricDir = MusicApp::cachePath() + "/cover";
    // TODO: key is what?
    return cacheLyricDir + "/" + info.hash + ".jpg";
}

static int doSyncGet(const QString &rootUrl, QByteArray &result)
{
    QNetworkRequest url;
    url.setUrl(rootUrl);

    QScopedPointer<QNetworkAccessManager> connection(new QNetworkAccessManager);
    QScopedPointer<QNetworkReply> reply(connection->get(url));

    QEventLoop waitLoop;
    QObject::connect(reply.data(), SIGNAL(finished()), &waitLoop, SLOT(quit()));
    waitLoop.exec();

    int errorCode = reply->error();
    if (errorCode != 0) {
//        qWarning() << "get" << url.url() << reply->errorString();
        return errorCode;
    }

    result = reply->readAll();

    return errorCode;
}

static int doGecimeAPI(const QString &url, QJsonObject &resultObject)
{
    QByteArray result;
    auto ret = doSyncGet(url, result);
    if (QNetworkReply::NoError != ret) {
        return ret;
    }
    resultObject = QJsonDocument::fromJson(result).object();
    return ret;
}

LyricService::LyricService(QObject *parent) : QObject(parent)
{
    QDir cacheDir(MusicApp::cachePath() + "/lyric");
    if (!cacheDir.exists()) {
        cacheDir.mkpath(".");
    }
    cacheDir = QDir(MusicApp::cachePath() + "/cover");
    if (!cacheDir.exists()) {
        cacheDir.mkpath(".");
    }
}

QString LyricService::coverPath(const MusicMeta &info)
{
    return cacheCoverPath(info);
}


int LyricService::searchCacheLyric(const MusicMeta &info)
{
    QFileInfo lyric(cacheLyricPath(info));
    if (!lyric.exists() || lyric.size() < 1) {
        emit lyricSearchFinished(info, "");
        return -1;
    }
    emit lyricSearchFinished(info, cacheLyricPath(info));
    return 0;
}

int LyricService::searchCacheCover(const MusicMeta &info)
{
    QFileInfo cover(coverPath(info));
    if (!cover.exists() || cover.size() < 1) {
        emit coverSearchFinished(info, "");
        return -1;
    }
    emit coverSearchFinished(info, coverPath(info));
    return 0;
}

//!
//! \brief LyricService::searchLyric
//! \param info
//!
void LyricService::searchLyricCover(const MusicMeta &info)
{
    bool needlyric = false;
    bool needCover = false;

    if (0 != searchCacheLyric(info)) {
        needlyric = true;
    }
    if (0 != searchCacheCover(info)) {
        needCover = true;
    }
    if (!needCover && !needlyric) {
        return;
    }
    if (info.artist != "Unknow Artist") {
        if (0 != doSongArtistRequest(info, needlyric, needCover)) {
            doSongRequest(info, needlyric, needCover);
        }
    } else {
        doSongRequest(info, needlyric, needCover);
    }
}


//!
//! \brief LyricService::doSongRequest
//! \param requestType
//! \param rootUrl
//! \param params
//! \return
/*!
    curl 'http://gecimi.com/api/lyric/海阔天空'
    {
        "count": 15,
        "code": 0,
        "result": [
            { "aid": 2848529, "lrc": "http://s.gecimi.com/lrc/344/34435/3443588.lrc", "sid": 3443588, "artist_id": 2, "song": "\u6d77\u9614\u5929\u7a7a" },
            { "aid": 2607041, "lrc": "http://s.gecimi.com/lrc/311/31116/3111659.lrc", "sid": 3111659, "artist_id": 9208, "song": "\u6d77\u9614\u5929\u7a7a" }
        ]
    }
 */
int LyricService::doSongRequest(const MusicMeta &info, bool lyric, bool cover)
{
    QString rootUrl = "http://gecimi.com/api/lyric/%1";
    rootUrl = rootUrl.arg(info.title);

    QJsonObject jsonObject;
    auto ret = doGecimeAPI(rootUrl, jsonObject);
    if (QNetworkReply::NoError != ret) {
        return ret;
    }

    auto resultArray = jsonObject.value("result").toArray();
    if (0 == resultArray.size()) {
        return -1;
    };
    auto result = resultArray.first().toObject();
    auto aid = result.value("aid").toInt();
    auto sid = result.value("sid").toInt();
    if (lyric) { doLyricRequest(info, sid); }
    if (cover) { doCoverRequest(info, aid); }
    return ret;
}

/*!
 * \brief LyricService::doSongArtistRequest
 * \param info
 * \return
 *
    curl 'http://gecimi.com/api/lyric/海阔天空/Beyond'
    {
        "count": 16,
        "code": 0,
        "result": [
            { "aid": 1563419, "artist_id": 9208, "sid": 1668536, "lrc": "http://s.gecimi.com/lrc/166/16685/1668536.lrc", "song": "\u6d77\u9614\u5929\u7a7a" },S
            { "aid": 3161846, "artist_id": 9208, "sid": 3861244, "lrc": "http://s.gecimi.com/lrc/386/38612/3861244.lrc", "song": "\u6d77\u9614\u5929\u7a7a" }
        ]
    }
 */
int LyricService::doSongArtistRequest(const MusicMeta &info, bool lyric, bool cover)
{
    QString rootUrl = "http://gecimi.com/api/lyric/%1/%2";
    rootUrl = rootUrl.arg(info.title).arg(info.artist);

    QJsonObject jsonObject;
    auto ret = doGecimeAPI(rootUrl, jsonObject);
    if (QNetworkReply::NoError != ret) {
        return ret;
    }

    auto resultArray = jsonObject.value("result").toArray();
    if (0 == resultArray.size()) {
        return -1;
    };
    auto result = resultArray.first().toObject();
    auto aid = result.value("aid").toInt();
    auto sid = result.value("sid").toInt();

    if (lyric) { doLyricRequest(info, sid); }
    if (cover) { doCoverRequest(info, aid); }
    return ret;
}

/*!
 * \brief LyricService::doLyricRequest
 * \param info
 * \param sid
 * \return
 * curl 'http://gecimi.com/api/lrc/3861244'
 * {
    "count": 1,
    "code": 0,
    "result": {
        "lrc": "http://s.gecimi.com/lrc/386/38612/3861244.lrc",
        "sid": "3861244"
    }
}
 */
int LyricService::doLyricRequest(const MusicMeta &info, int sid)
{
    QString rootUrl = "http://gecimi.com/api/lrc/%1";
    rootUrl = rootUrl.arg(sid);

    QJsonObject jsonObject;
    auto ret = doGecimeAPI(rootUrl, jsonObject);
    if (QNetworkReply::NoError != ret) {
        return ret;
    }
    auto count = jsonObject.value("count").toInt();
    if (0 == count) {
        return -1;
    };
    QJsonObject result;
    if (1 == count) {
        result = jsonObject.value("result").toObject();
    }
    if (1 < count) {
        result = jsonObject.value("result").toArray().first().toObject();
    }
    auto lrcurl = result.value("lrc").toString();
    downloadUrl(lrcurl, cacheLyricPath(info));
    emit lyricSearchFinished(info, cacheLyricPath(info));
    return ret;
}

/*!
 * \brief LyricService::doCoverRequest
 * \param info
 * \param aid
 * \return
 *  curl 'http://gecimi.com/api/cover/1573814'
    {
        "count": 1,
        "code": 0,
        "result": {
            "cover": "http://s.gecimi.com/album-cover/157/1573814.jpg",
            "thumb": "http://s.gecimi.com/album-cover/157/1573814-thumb.jpg"
        }
    }
 */
int LyricService::doCoverRequest(const MusicMeta &info, int aid)
{
    QString rootUrl = "http://gecimi.com/api/cover/%1";
    rootUrl = rootUrl.arg(aid);

    QJsonObject jsonObject;
    auto ret = doGecimeAPI(rootUrl, jsonObject);
    if (QNetworkReply::NoError != ret) {
        return ret;
    }
    auto count = jsonObject.value("count").toInt();
    if (0 == count) {
        return -1;
    };
    QJsonObject result;
    if (1 == count) {
        result = jsonObject.value("result").toObject();
    }
    if (1 < count) {
        result = jsonObject.value("result").toArray().first().toObject();
    }
    auto lrcurl = result.value("cover").toString();
    downloadUrl(lrcurl, coverPath(info));
    emit coverSearchFinished(info, coverPath(info));
    return ret;
}

int LyricService::downloadUrl(const QString &rootUrl, const QString &filepath)
{
    QByteArray result;
    auto ret = doSyncGet(rootUrl, result);
    if (QNetworkReply::NoError != ret) {
        return ret;
    }

    QFile file(filepath);
    file.open(QIODevice::WriteOnly);
    file.write(result);
    file.close();

    return ret;
}
