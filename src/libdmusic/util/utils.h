// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UTILS_H
#define UTILS_H

#include <complex>

#include <QVariant>
#include <QDBusConnection>

#include "global.h"

class UtilsPrivate;
class Utils
{
public:
    static bool isChinese(const QChar &c);
    static QStringList simpleChineseSplit(QString &str);
    static void updateChineseMetaInfo(DMusic::MediaMeta &meta);
    static QStringList detectEncodings(const QByteArray &rawData);
    static QString filePathHash(const QString &filepath);
    static void fft(std::complex<float> *Data, int Log2N, int sign);
    static QVariant readDBusProperty(const QString &service, const QString &path, const QString &interface = QString(), const char *property = "", QDBusConnection connection = QDBusConnection::sessionBus());
    static QVariantMap metaToVariantMap(const DMusic::MediaMeta &meta);
    static QVariantMap albumToVariantMap(const DMusic::AlbumInfo &album);
    static QVariantMap artistToVariantMap(const DMusic::ArtistInfo &artist);
    static QVariantMap playlistToVariantMap(const DMusic::PlaylistInfo &playlist);
    static bool containsStr(QString searchText, QString text);
    static int simplifyPlaylistSortType(const int &sortType);

private:
    Utils() {}
};

#endif // DBUSUTILS_H
