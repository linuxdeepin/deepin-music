/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#pragma once

#include <QObject>
#include <QScopedPointer>
#include <QNetworkAccessManager>

#include "libdmusic_global.h"

namespace DMusic
{
namespace Net
{

class Geese;

class LIBDMUSICSHARED_EXPORT Goose : public QObject
{
    Q_OBJECT
public:
    explicit Goose(Geese *geese);

    void get(const QUrl &url);
    void post(const QUrl &url, const QByteArray &data);
signals:
    void arrive(int errCode, const QByteArray &data);

private:
    Geese   *m_geese = nullptr;
};

class GeesePrivate;
class LIBDMUSICSHARED_EXPORT Geese : public QNetworkAccessManager
{
    Q_OBJECT

    friend class Goose;
public:
    explicit Geese(QObject *parent = 0);
    ~Geese();

    Goose *getGoose(QUrl url);
    Goose *postGoose(const QUrl &url, const QByteArray &data);

    void setRawHeader(const QByteArray &headerName, const QByteArray &value);
signals:

public slots:

private:
    void prepare(QNetworkRequest &request);

    Q_DISABLE_COPY(Geese)
    QScopedPointer<GeesePrivate>  d;
};

}
}

