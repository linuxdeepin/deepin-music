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

LyricService::LyricService(QObject *parent) : QObject(parent)
{
    m_geese  =  new DMusic::Net::Geese(this);
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
            if (coverData.length() > 0) {
                QFile coverFile(cacheCoverPath(meta));
                coverFile.open(QIODevice::WriteOnly);
                coverFile.write(coverData);
                coverFile.close();
            }

            emit coverSearchFinished(meta, coverData);
        });
        connect(engine, &DMusic::Plugin::MetaSearchEngine::lyricLoaded,
        this, [ = ](const MusicMeta & meta, const QByteArray & lyricData) {
            if (lyricData.length() > 0) {
                QFile lyricFile(cacheLyricPath(meta));
                lyricFile.open(QIODevice::WriteOnly);
                lyricFile.write(lyricData);
                lyricFile.close();
            }

            emit lyricSearchFinished(meta, lyricData);
        });

        void ();
        connect(engine, &DMusic::Plugin::MetaSearchEngine::contextSearchFinished,
        this, [ = ](const QString & context, const MusicMetaList &metalist) {
            emit  contextSearchFinished(context, metalist);
        });
    }
}

QUrl LyricService::coverUrl(const MusicMeta &info)
{
    return QUrl::fromLocalFile(cacheCoverPath(info));
}

QByteArray LyricService::coverData(const MusicMeta &info)
{
    QFile coverFile(cacheCoverPath(info));
    qDebug() << cacheCoverPath(info);
    if (coverFile.open(QIODevice::ReadOnly)) {
        return coverFile.readAll();
    }
    return QByteArray();
}

QByteArray LyricService::lyricData(const MusicMeta &info)
{
    QFile lyricFile(cacheLyricPath(info));
    if (lyricFile.open(QIODevice::ReadOnly)) {
        return lyricFile.readAll();
    }
    return QByteArray();
}

int LyricService::searchCacheLyric(const MusicMeta &info)
{
    QFileInfo lyric(cacheLyricPath(info));
    if (!lyric.exists() || lyric.size() < 1) {
//        emit lyricSearchFinished(info, QByteArray());
        return -1;
    }
    emit lyricSearchFinished(info, lyricData(info));
    return 0;
}

int LyricService::searchCacheCover(const MusicMeta &info)
{
    QFileInfo cover(cacheCoverPath(info));
    if (!cover.exists() || cover.size() < 1) {
//        emit coverSearchFinished(info, QByteArray());
        return -1;
    }
    emit coverSearchFinished(info, coverData(info));
    return 0;
}

//!
//! \brief LyricService::searchMeta
//! \param info
//!
void LyricService::searchMeta(const MusicMeta &info)
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

    auto plugins = PluginManager::instance()->getPluginListByType(DMusic::Plugin::PluginType::TypeMetaSearchEngine);
    for (auto plugin : plugins) {
        qDebug() << "search by " << plugin->pluginId() << info.title;
        auto engine = dynamic_cast<DMusic::Plugin::MetaSearchEngine *>(plugin);
        emit engine->doSearchMeta(info);
    }

}

void LyricService::searchContext(const QString &context)
{
    auto plugins = PluginManager::instance()->getPluginListByType(DMusic::Plugin::PluginType::TypeMetaSearchEngine);
    for (auto plugin : plugins) {
        qDebug() << "search by " << plugin->pluginId() << context;
        auto engine = dynamic_cast<DMusic::Plugin::MetaSearchEngine *>(plugin);
        emit engine->doSearchContext(context);
    }
}

void LyricService::onChangeMetaCache(const MusicMeta &meta)
{
    qDebug() << "change" << meta.searchCoverUrl << meta.searchLyricUrl;
    connect(m_geese->getGoose(meta.searchCoverUrl), &DMusic::Net::Goose::arrive,
    this, [ = ](int errCode, const QByteArray & coverData) {
        qDebug() << "onChangeMetaCache recive: " << errCode << coverData.length();
        if (coverData.length() > 0) {
            QFile coverFile(cacheCoverPath(meta));
            coverFile.open(QIODevice::WriteOnly);
            coverFile.write(coverData);
            coverFile.close();
        }
        emit coverSearchFinished(meta, coverData);
    });

    connect(m_geese->getGoose(meta.searchLyricUrl), &DMusic::Net::Goose::arrive,
    this, [ = ](int errCode, const QByteArray & data) {
        qDebug() << "onChangeMetaCache recive: " << errCode << data.length();
        auto document = QJsonDocument::fromJson(data);
        auto lrc = document.object().value("lrc").toObject();
        auto lrcData =  lrc.value("lyric").toString().toUtf8();

        if (lrcData.length() <= 0) {
            lrcData = "                           ";
        }

        QFile lyricFile(cacheLyricPath(meta));
        lyricFile.open(QIODevice::WriteOnly);
        lyricFile.write(lrcData);
        lyricFile.close();

        emit lyricSearchFinished(meta, lrcData);
    });

}

