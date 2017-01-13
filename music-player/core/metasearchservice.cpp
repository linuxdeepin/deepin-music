/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "metasearchservice.h"

#include <QUrl>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>

#include <net/geese.h>
#include <metasearchengine.h>

#include "util/global.h"
#include "pluginmanager.h"

class MetaSearchServicePrivate
{
public:
    MetaSearchServicePrivate(MetaSearchService *parent) : q_ptr(parent){}

    int searchCacheLyric(const MetaPtr meta);
    int searchCacheCover(const MetaPtr meta);

    DMusic::Net::Geese       *m_geese = nullptr;

    MetaSearchService *q_ptr;
    Q_DECLARE_PUBLIC(MetaSearchService)
};

MetaSearchService::~MetaSearchService()
{

}

static QString cacheLyricPath(const MetaPtr meta)
{
    auto cacheLyricDir = Global::cacheDir() + "/lyric";
    return cacheLyricDir + "/" + meta->hash + ".lyric";
}

inline QString cacheCoverPath(const MetaPtr meta)
{
    auto cacheLyricDir =  Global::cacheDir() + "/cover";
    // TODO: key is what?
    return cacheLyricDir + "/" + meta->hash + ".jpg";
}

MetaSearchService::MetaSearchService(QObject *parent) :
    QObject(parent), d_ptr(new MetaSearchServicePrivate(this))
{
    Q_D(MetaSearchService);
    d->m_geese  =  new DMusic::Net::Geese(this);
    QDir cacheDir(Global::cacheDir() + "/lyric");
    if (!cacheDir.exists()) {
        cacheDir.mkpath(".");
    }
    cacheDir = QDir(Global::cacheDir() + "/cover");
    if (!cacheDir.exists()) {
        cacheDir.mkpath(".");
    }

    qDebug() << "load meta search plugins";
    auto plugins = PluginManager::instance()->getPluginListByType(DMusic::Plugin::PluginType::TypeMetaSearchEngine);
    for (auto plugin : plugins) {
        auto engine = dynamic_cast<DMusic::Plugin::MetaSearchEngine *>(plugin);
        qDebug() << "load meta search engine" << engine;
        connect(engine, &DMusic::Plugin::MetaSearchEngine::coverLoaded,
        this, [ = ](const MetaPtr  meta, const QByteArray & coverData) {
            if (coverData.length() > 0) {
                QFile coverFile(cacheCoverPath(meta));
                coverFile.open(QIODevice::WriteOnly);
                coverFile.write(coverData);
                coverFile.close();
            }

            emit coverSearchFinished(meta, coverData);
        });
        connect(engine, &DMusic::Plugin::MetaSearchEngine::lyricLoaded,
        this, [ = ](const MetaPtr  meta, const QByteArray & lyricData) {
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
        this, [ = ](const QString & context, const QList<MediaMeta> &metalist) {
            emit  contextSearchFinished(context, metalist);
        });
    }
}

QUrl MetaSearchService::coverUrl(const MetaPtr meta)
{
    return QUrl::fromLocalFile(cacheCoverPath(meta));
}

QUrl MetaSearchService::lyricUrl(const MetaPtr meta)
{
    return QUrl::fromLocalFile(cacheLyricPath(meta));
}

QByteArray MetaSearchService::coverData(const MetaPtr meta)
{
    Q_ASSERT(!meta.isNull());

    QFile coverFile(cacheCoverPath(meta));
    qDebug() << cacheCoverPath(meta);
    if (coverFile.open(QIODevice::ReadOnly)) {
        return coverFile.readAll();
    }
    return QByteArray();
}

QByteArray MetaSearchService::lyricData(const MetaPtr meta)
{
    QFile lyricFile(cacheLyricPath(meta));
    if (lyricFile.open(QIODevice::ReadOnly)) {
        return lyricFile.readAll();
    }
    return QByteArray();
}

int MetaSearchServicePrivate::searchCacheLyric(const MetaPtr meta)
{
    Q_Q(MetaSearchService);
    QFileInfo lyric(cacheLyricPath(meta));
    if (!lyric.exists() || lyric.size() < 1) {
//        emit lyricSearchFinished(meta, QByteArray());
        return -1;
    }
    emit q->lyricSearchFinished(meta, MetaSearchService::lyricData(meta));
    return 0;
}

int MetaSearchServicePrivate::searchCacheCover(const MetaPtr meta)
{
    Q_Q(MetaSearchService);
    QFileInfo cover(cacheCoverPath(meta));
    if (!cover.exists() || cover.size() < 1) {
//        emit coverSearchFinished(meta, QByteArray());
        return -1;
    }
    emit q->coverSearchFinished(meta, MetaSearchService::coverData(meta));
    return 0;
}

//!
//! \brief LyricService::searchMeta
//! \param meta
//!
void MetaSearchService::searchMeta(const MetaPtr meta)
{
    Q_D(MetaSearchService);
    bool needlyric = false;
    bool needCover = false;

    if (0 != d->searchCacheLyric(meta)) {
        needlyric = true;
    }
    if (0 != d->searchCacheCover(meta)) {
        needCover = true;
    }
    if (!needCover && !needlyric) {
        return;
    }

    auto plugins = PluginManager::instance()->getPluginListByType(DMusic::Plugin::PluginType::TypeMetaSearchEngine);
    for (auto plugin : plugins) {
        qDebug() << "search by " << plugin->pluginId() << meta->title;
        auto engine = dynamic_cast<DMusic::Plugin::MetaSearchEngine *>(plugin);
        emit engine->doSearchMeta(meta);
    }

}

void MetaSearchService::searchContext(const QString &context)
{
    auto plugins = PluginManager::instance()->getPluginListByType(DMusic::Plugin::PluginType::TypeMetaSearchEngine);
    for (auto plugin : plugins) {
        qDebug() << "search by " << plugin->pluginId() << context;
        auto engine = dynamic_cast<DMusic::Plugin::MetaSearchEngine *>(plugin);
        emit engine->doSearchContext(context);
    }
}

void MetaSearchService::onChangeMetaCache(const MetaPtr meta)
{
    Q_D(MetaSearchService);
    qDebug() << "change" << meta->searchCoverUrl << meta->searchLyricUrl;
    connect(d->m_geese->getGoose(meta->searchCoverUrl), &DMusic::Net::Goose::arrive,
    this, [ = ](int errCode, const QByteArray & coverData) {
        qDebug() << "onChangeMetaCache received: " << errCode << coverData.length();
        if (coverData.length() > 0) {
            QFile coverFile(cacheCoverPath(meta));
            coverFile.open(QIODevice::WriteOnly);
            coverFile.write(coverData);
            coverFile.close();
        }
        emit coverSearchFinished(meta, coverData);
    });

    connect(d->m_geese->getGoose(meta->searchLyricUrl), &DMusic::Net::Goose::arrive,
    this, [ = ](int errCode, const QByteArray & data) {
        qDebug() << "onChangeMetaCache received: " << errCode << data.length();
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

