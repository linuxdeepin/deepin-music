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

#include "metainfo.h"
#include "metaanalyzer.h"

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

/*!
 * \brief doSyncPost
 * \param rootUrl
 * \param result
 * \return
 *  Cookie: appver=1.5.0.75771;
    Referer: http://music.163.com/
 */
static int doNeteaseSyncPost(const QString &rootUrl, const QByteArray &encodeData, QByteArray &result)
{
    QNetworkRequest request(rootUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setRawHeader("Cookie", "appver=1.5.0.75771;");
    request.setRawHeader("Referer", "http://music.163.com/");

    QScopedPointer<QNetworkAccessManager> connection(new QNetworkAccessManager);
    QScopedPointer<QNetworkReply> reply(connection->post(request, encodeData));

    QEventLoop waitLoop;
    QObject::connect(reply.data(), SIGNAL(finished()), &waitLoop, SLOT(quit()));
    waitLoop.exec();

    int errorCode = reply->error();
    if (errorCode != 0) {
        qWarning() << "get" << request.url() << reply->errorString() << errorCode;
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
    m_geese = new DMusic::Net::Geese(this);
    m_geese->setRawHeader("Content-Type", "application/x-www-form-urlencoded");
    m_geese->setRawHeader("Cookie", "appver=1.5.0.75771;");
    m_geese->setRawHeader("Referer", "http://music.163.com/");

//    qDebug() << "-------------------------------------------------------";
//    connect(this, &MetaSearchEngine::doSearchMeta,
//            this, &NeteaseMetaSearchEngine::searchMeta);
    connect(getObject(), SIGNAL(doSearchMeta(const MusicMeta &)),
            this, SLOT(searchMeta(const MusicMeta &)));
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

static QList<NeteaseSong> toSongList(const QByteArray &data)
{
    QList<NeteaseSong> neteaseSongs;

    auto document = QJsonDocument::fromJson(data);
    auto searchResult = document.object().value("result").toObject();
    auto songs = searchResult.value("songs").toArray();
    auto songCount = searchResult.value("songCount").toInt();

    qDebug() << "-------------------------------------------------------\n Find " << songCount << "result";
    for (auto songJson : songs) {
        NeteaseSong neteaseSong;
        auto song = songJson.toObject();
        neteaseSong.id = song.value("id").toInt();
        neteaseSong.name = song.value("name").toString();
        auto album = song.value("album").toObject();
        neteaseSong.album.name = album.value("name").toString();
        neteaseSong.album.coverUrl = album.value("blurPicUrl").toString();
        auto artists = song.value("artists").toArray();

        for (auto artistJson : artists) {
            auto artist = artistJson.toObject();
            NeteaseArtist neArtist;
            neArtist.name = artist.value("name").toString();
            neteaseSong.artists << neArtist;
        }
//        qDebug() << ">>>> Find song:";
//        qDebug() << neteaseSong.id << neteaseSong.name
//                 << neteaseSong.artists.first().name
//                 << neteaseSong.album.name;
//        for (auto artist : neteaseSong.artists) {
//            qDebug() << artist.name;
//        }
//        qDebug() << "<<<<";
        neteaseSongs << neteaseSong;
    }
    qDebug() << "-------------------------------------------------------";
    return neteaseSongs;
}

static QByteArray toLyric(const QByteArray &data)
{
    auto document = QJsonDocument::fromJson(data);
    auto lrc = document.object().value("lrc").toObject();
    return lrc.value("lyric").toString().toUtf8();

}

void NeteaseMetaSearchEngine::searchMeta(const MusicMeta &meta)
{
    qDebug() << "searchMeta";
    QString queryUrl = QLatin1String("http://music.163.com/api/search/pc");
    QString queryTemplate = QLatin1String("s=%1&offset=0&limit=5&type=1");
    QUrl params = QUrl(queryTemplate.arg(meta.title));

    auto anlyzer = new MetaAnalyzer(meta, m_geese);

    connect(anlyzer, &MetaAnalyzer::searchFinished,
    this, [ = ](const MusicMeta & meta, NeteaseSong song) {

        qDebug() << "get " << "=====" << song.album.coverUrl;
        connect(m_geese->getGoose(song.album.coverUrl), &DMusic::Net::Goose::arrive,
        this, [ = ](int errCode, const QByteArray & data) {
            qDebug() << "NeteaseMetaSearchEngine recive: " << errCode << data.length();
            emit this->coverLoaded(meta, data);
        });

        QString lyricUrl = QLatin1String("http://music.163.com/api/song/lyric?os=pc&id=%1&lv=-1&kv=-1&tv=-1");
        lyricUrl = lyricUrl.arg(song.id);
        qDebug() << "get " << "=====" << lyricUrl;
        connect(m_geese->getGoose(lyricUrl), &DMusic::Net::Goose::arrive,
        this, [ = ](int errCode, const QByteArray & data) {
            qDebug() << "NeteaseMetaSearchEngine recive: " << errCode << data.length();
            emit this->lyricLoaded(meta, toLyric(data));
        });

        anlyzer->deleteLater();
    });
    auto goose = m_geese->postGoose(queryUrl, params.toEncoded());

    connect(goose, &DMusic::Net::Goose::arrive,
    this, [ = ](int errCode, const QByteArray & data) {
        qDebug() << "NeteaseMetaSearchEngine Resut: " << errCode;
        auto neteaseSongs = toSongList(data);
        emit anlyzer->onGetAblumResult(neteaseSongs);
    });

    queryTemplate = QLatin1String("s=%1&offset=0&limit=5&type=1");
    params = QUrl(queryTemplate.arg(meta.title + meta.album));
    goose = m_geese->postGoose(queryUrl, params.toEncoded());
    connect(goose, &DMusic::Net::Goose::arrive,
    this, [ = ](int errCode, const QByteArray & data) {
        qDebug() << "NeteaseMetaSearchEngine Resut: " << errCode;
        auto neteaseSongs = toSongList(data);
        emit anlyzer->onGetTitleResult(neteaseSongs);
    });
}


