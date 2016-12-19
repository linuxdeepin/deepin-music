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

#include <metasearchengine.h>

#include "music.h"
#include "../musicapp.h"

#include "playlistmanager.h"
#include "pluginmanager.h"

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

    qDebug() << "load meta search plugins";
    auto plugins = PluginManager::instance()->getPluginListByType(DMusic::Plugin::PluginType::TypeMetaSearchEngine);
    for (auto plugin : plugins) {
        auto engine = dynamic_cast<DMusic::Plugin::MetaSearchEngine *>(plugin);
        qDebug() << "load meta search engine" << engine;
        connect(engine, &DMusic::Plugin::MetaSearchEngine::coverLoaded,
        this, [ = ](const MusicMeta & meta, const QByteArray & coverData) {
            emit coverSearchFinished(meta, coverData);
        });
        connect(engine, &DMusic::Plugin::MetaSearchEngine::lyricLoaded,
        this, [ = ](const MusicMeta & meta, const QByteArray & coverData) {
            emit lyricSearchFinished(meta, coverData);
        });
//        connect(engine, &DMusic::Plugin::MetaSearchEngine::doSearchMeta,
//                engine, &DMusic::Plugin::MetaSearchEngine::searchMeta);
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
//    emit lyricSearchFinished(info, cacheLyricPath(info));
    return 0;
}

int LyricService::searchCacheCover(const MusicMeta &info)
{
    QFileInfo cover(coverPath(info));
    if (!cover.exists() || cover.size() < 1) {
        emit coverSearchFinished(info, "");
        return -1;
    }
//    emit coverSearchFinished(info, coverPath(info));
    return 0;
}

//!
//! \brief LyricService::searchMeta
//! \param info
//!
void LyricService::searchMeta(const MusicMeta &info)
{
    qDebug() << "requestLyricCoverSearch" << info.title;
    auto plugins = PluginManager::instance()->getPluginListByType(DMusic::Plugin::PluginType::TypeMetaSearchEngine);
    for (auto plugin : plugins) {
        auto engine = dynamic_cast<DMusic::Plugin::MetaSearchEngine *>(plugin);
        emit engine->doSearchMeta(info);
    }
//    bool needlyric = false;
//    bool needCover = false;

//    if (0 != searchCacheLyric(info)) {
//        needlyric = true;
//    }
//    if (0 != searchCacheCover(info)) {
//        needCover = true;
//    }
//    if (!needCover && !needlyric) {
//        return;
//    }
//    if (info.artist != "Unknow Artist") {
//        if (0 != doSongArtistRequest(info, needlyric, needCover)) {
//            doSongRequest(info, needlyric, needCover);
//        }
//    } else {
//        doSongRequest(info, needlyric, needCover);
//    }
}

