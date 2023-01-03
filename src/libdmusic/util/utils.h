/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     ZouYa <zouya@uniontech.com>
 *
 * Maintainer: WangYu <wangyu@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
    static QVariant readDBusProperty(const QString &service, const QString &path, const QString &interface = QString(), const char *propert = "", QDBusConnection connection = QDBusConnection::sessionBus());
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
