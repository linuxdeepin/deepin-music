/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "geese.h"

#include <QDebug>

#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QMimeDatabase>

namespace DMusic
{
namespace Net
{

Goose::Goose(Geese *geese)
{
    m_geese = geese;
}

void Goose::get(const QUrl &url)
{
    QNetworkRequest request(url);

    m_geese->prepare(request);
    auto reply = m_geese->get(request);

    connect(reply, &QNetworkReply::finished,
    this, [ = ]() {
        emit arrive(QNetworkReply::NoError, reply->readAll());
    });

    connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error),
    this, [ = ](QNetworkReply::NetworkError error) {
        qWarning() << "Goose: get" << reply->errorString();
        emit arrive(error, reply->readAll());
    });

}

void Goose::post(const QUrl &url, const QByteArray &data)
{
    QNetworkRequest request(url);

    m_geese->prepare(request);
    auto reply = m_geese->post(request, data);

    connect(reply, &QNetworkReply::finished,
    this, [ = ]() {
//        QMimeDatabase mdb;
//        auto contentType = reply->header(QNetworkRequest::ContentTypeHeader).toString();
//        qDebug() << mdb.mimeTypeForName(contentType);
        emit arrive(QNetworkReply::NoError, reply->readAll());
    });

    connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error),
    this, [ = ](QNetworkReply::NetworkError error) {
        qWarning() << "Goose: get" << reply->errorString();
        emit arrive(error, reply->readAll());
    });
}

class GeesePrivate
{
public:
    QMap<QByteArray, QByteArray> rawHeader;
};

Geese::Geese(QObject *parent): QNetworkAccessManager(parent), d(new GeesePrivate)
{

}

Geese::~Geese()
{

}

Goose *Geese::getGoose(QUrl url)
{
    auto goose = new Goose(this);
    goose->get(url);
    return goose;
}

Goose *Geese::postGoose(const QUrl &url, const QByteArray &data)
{
    auto goose = new Goose(this);
    goose->post(url, data);
    return goose;
}

void Geese::setRawHeader(const QByteArray &headerName, const QByteArray &value)
{
    d->rawHeader.insert(headerName, value);
}

void Geese::prepare(QNetworkRequest &request)
{
    for (auto &headerName : d->rawHeader.keys()) {
        request.setRawHeader(headerName, d->rawHeader.value(headerName));
    }
}

}
}
