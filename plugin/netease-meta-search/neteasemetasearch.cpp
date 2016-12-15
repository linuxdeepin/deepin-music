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

#include "../../app/core/music.h"

using namespace DMusic::Plugin;

struct NeteaseAlbum {
    int     id;
    QString name;
    QString coverUrl;
};

struct NeteaseArtist {
    int     id;
    QString name;
    QString avatarUrl;
};

struct NeteaseSong {
    int                     id;
    QString                 name;
    NeteaseAlbum            album;
    QList<NeteaseArtist>    artists;
};

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

int downloadUrl(const QString &rootUrl, const QString &filepath)
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

static QList<NeteaseSong> syncNeteaseSearch(const QString &title)
{
    QList<NeteaseSong> neteaseSongs;
    QByteArray result;
    QString queryUrl = QLatin1String("http://music.163.com/api/search/pc");
    QString queryTemplate = QLatin1String("s=%1&offset=0&limit=1&type=1");
    QUrl params = QUrl(queryTemplate.arg(title));

    doNeteaseSyncPost(queryUrl, params.toEncoded(), result);

    // parse
    auto document = QJsonDocument::fromJson(result);
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
        qDebug() << "++++++ Find song:";
        qDebug() << neteaseSong.id << neteaseSong.name
                 << neteaseSong.album.name << neteaseSong.album.coverUrl;
        for (auto artist : neteaseSong.artists) {
            qDebug() << artist.name;
        }
        qDebug() << "++++++";
        neteaseSongs << neteaseSong;
    }
    qDebug() << "-------------------------------------------------------";
    return neteaseSongs;
}

NeteaseMetaSearchEngine::NeteaseMetaSearchEngine(QObject *parent): MetaSearchEngine(parent)
{
}

QString NeteaseMetaSearchEngine::pluginId() const
{
    return "Netease-Meta-Search";
}

DMusic::Plugin::PluginType NeteaseMetaSearchEngine::pluginType() const
{
    qDebug() << int(DMusic::Plugin::PluginType::TypeMetaSearchEngine);
    return DMusic::Plugin::PluginType::TypeMetaSearchEngine;
}

void NeteaseMetaSearchEngine::searchMeta(const MusicMeta &meta)
{
    auto list = syncNeteaseSearch(meta.title);
    if (!list.isEmpty()) {
        auto song = list.first();
        qDebug() << "call downloadUrl " << song.album.coverUrl;
        downloadUrl(song.album.coverUrl,  QString("/tmp/%1.png").arg(meta.hash));
        emit coverLoaded(meta, QString("/tmp/%1.png").arg(meta.hash).toUtf8());
    }
}


