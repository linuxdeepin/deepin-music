/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "neteasemetasearch.h"

#include <QDebug>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QEventLoop>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QMimeDatabase>

#include "../../music-player/core/music.h"

#include <searchmeta.h>

#include "metaanalyzer.h"

using namespace DMusic;
using namespace DMusic::Plugin;

static int doSyncGet(const QString &rootUrl, QByteArray &result)
{
    QNetworkRequest url;
    url.setUrl(rootUrl);

    QScopedPointer<QNetworkAccessManager> connection(new QNetworkAccessManager);
    QScopedPointer<QNetworkReply> reply(connection->get(url));

    qDebug() << "doSyncGet" << rootUrl;
    QEventLoop waitLoop;
    QObject::connect(reply.data(), SIGNAL(finished()), &waitLoop, SLOT(quit()));
    waitLoop.exec();

    int errorCode = reply->error();
    if (errorCode != 0) {
        qWarning() << "get" << url.url() << reply->errorString();
        return errorCode;
    }
    result = reply->readAll();

    return errorCode;
}

int downloadFile(const QString &rootUrl, const QString &filepath)
{
    QByteArray result;
    auto ret = doSyncGet(rootUrl, result);
    if (QNetworkReply::NoError != ret) {
        return ret;
    }

    qDebug() << "save" << filepath;
    QFile file(filepath);
    file.open(QIODevice::WriteOnly);
    file.write(result);
    file.close();

    return ret;
}

NeteaseMetaSearchEngine::NeteaseMetaSearchEngine(QObject *parent): MetaSearchEngine(parent)
{
    qRegisterMetaType<QList<MediaMeta> >();
    qRegisterMetaType<QList<DMusic::SearchMeta> >();
    qRegisterMetaType<DMusic::SearchMeta>();
    qRegisterMetaType<QList<SearchMeta>>();

    m_geese = new DMusic::Net::Geese(this);
    m_geese->setRawHeader("Content-Type", "application/x-www-form-urlencoded");
    m_geese->setRawHeader("Cookie", "appver=1.5.0.75771;");
    m_geese->setRawHeader("Referer", "http://music.163.com/");

//    qDebug() << "-------------------------------------------------------";
//    connect(this, &MetaSearchEngine::doSearchMeta,
//            this, &NeteaseMetaSearchEngine::searchMeta);
    connect(getObject(), SIGNAL(doSearchMeta(const MetaPtr)),
            this, SLOT(searchMeta(const MetaPtr)));
    connect(getObject(), SIGNAL(doSearchContext(const QString &)),
            this, SLOT(searchContext(const QString &)));
//    qDebug() << "-------------------------------------------------------";
}

QString NeteaseMetaSearchEngine::pluginId() const
{
    return "Netease-Meta-Search";
}

QObject *NeteaseMetaSearchEngine::getObject()
{
    return this;
}

DMusic::Plugin::PluginType NeteaseMetaSearchEngine::pluginType() const
{
    return DMusic::Plugin::PluginType::TypeMetaSearchEngine;
}

static QList<SearchMeta> toSongList(const QByteArray &data)
{
    QList<SearchMeta> SearchMetas;

    auto document = QJsonDocument::fromJson(data);
    auto searchResult = document.object().value("result").toObject();
    auto songs = searchResult.value("songs").toArray();
//    auto songCount = searchResult.value("songCount").toInt();
//    qDebug() << "--------\n Find " << songCount << "result";
    for (auto songJson : songs) {
        SearchMeta searchMeta;
        auto song = songJson.toObject();
        auto length = song.value("bMusic").toObject().value("playTime").toInt();
        searchMeta.id = QString("netease_%1").arg(song.value("id").toInt());
        searchMeta.name = song.value("name").toString();
        searchMeta.length = length;
        auto album = song.value("album").toObject();
        searchMeta.album.name = album.value("name").toString();
        searchMeta.album.coverUrl = album.value("blurPicUrl").toString();
        auto artists = song.value("artists").toArray();

        for (auto artistJson : artists) {
            auto artist = artistJson.toObject();
            SearchArtist neArtist;
            neArtist.name = artist.value("name").toString();
            searchMeta.artists << neArtist;
        }
//        qDebug() << ">>>> Find song:";
//        qDebug() << SearchMeta.id << SearchMeta.name
//                 << SearchMeta.artists.first().name
//                 << SearchMeta.album.name;
//        for (auto artist : SearchMeta.artists) {
//            qDebug() << artist.name;
//        }
//        qDebug() << "<<<<";
        SearchMetas << searchMeta;
    }
//    qDebug() << "--------";
    return SearchMetas;
}

static QByteArray toLyric(const QByteArray &data)
{
    auto document = QJsonDocument::fromJson(data);
    auto lrc = document.object().value("lrc").toObject();
    return lrc.value("lyric").toString().toUtf8();
}

void NeteaseMetaSearchEngine::searchMeta(const MetaPtr meta)
{
    if (meta.isNull()) {
        return;
    }

    QString queryUrl = QLatin1String("http://music.163.com/api/search/pc");
    QString queryTemplate = QLatin1String("s=%1&offset=0&limit=5&type=1");
    QUrl params = QUrl(queryTemplate.arg(meta->title));

    auto anlyzer = QSharedPointer<MetaAnalyzer>(new MetaAnalyzer(meta, m_geese));
    connect(anlyzer.data(), &MetaAnalyzer::searchFinished,
    this, [ = ](const MetaPtr meta, SearchMeta song) {
        qDebug() << "GTE" << song.id << "cover:" << song.album.coverUrl ;
        connect(m_geese->getGoose(song.album.coverUrl), &DMusic::Net::Goose::arrive,
        this, [ = ](int errCode, const QByteArray & data) {
            qDebug() << "netease cover respone:" << errCode << "with body size:" << data.length();
            emit this->coverLoaded(meta, song, data);
        });

        QString lyricUrl = QLatin1String("http://music.163.com/api/song/lyric?os=pc&id=%1&lv=-1&kv=-1&tv=-1");
        lyricUrl = lyricUrl.arg(QString(song.id).remove("netease_"));
        qDebug() << "GTE" << song.id << "lyric:" << lyricUrl;
        connect(m_geese->getGoose(lyricUrl), &DMusic::Net::Goose::arrive,
        this, [ = ](int errCode, const QByteArray & data) {
            qDebug() << "netease lyric respone:" << errCode << "with body size:" << data.length();
            emit this->lyricLoaded(meta, song, toLyric(data));
        });
    });

    auto goose = m_geese->postGoose(queryUrl, params.toEncoded());
    connect(goose, &DMusic::Net::Goose::arrive,
    this, [ = ](int errCode, const QByteArray & data) {
        if (errCode != QNetworkReply::NoError || anlyzer.isNull()) {
            return;
        }
        auto searchMetas = toSongList(data);
        anlyzer->onGetAblumResult(searchMetas);
        goose->deleteLater();
    });

    queryTemplate = QLatin1String("s=%1&offset=0&limit=5&type=1");
    params = QUrl(queryTemplate.arg(meta->title + meta->album));
    goose = m_geese->postGoose(queryUrl, params.toEncoded());
    connect(goose, &DMusic::Net::Goose::arrive,
    this, [ = ](int errCode, const QByteArray & data) {
        if (errCode != QNetworkReply::NoError || anlyzer.isNull()) {
            return;
        }
        auto searchMetas = toSongList(data);
        anlyzer->onGetTitleResult(searchMetas);
        goose->deleteLater();
    });
}

void NeteaseMetaSearchEngine::searchContext(const QString &context)
{
    QString queryUrl = QLatin1String("http://music.163.com/api/search/pc");
    QString queryTemplate = QLatin1String("s=%1&offset=0&limit=10&type=1");
    QUrl params = QUrl(queryTemplate.arg(context));
    auto goose = m_geese->postGoose(queryUrl, params.toEncoded());
    connect(goose, &DMusic::Net::Goose::arrive,
    this, [ = ](int errCode, const QByteArray & data) {
        qDebug() << "NeteaseMetaSearchEngine errcode: " << errCode;
        auto searchMetas = toSongList(data);
        qDebug() << "contextSearchFinished with " << searchMetas.length() << "result";
        emit this->contextSearchFinished(context, searchMetas);
        goose->deleteLater();
    });
}


