/*
 * Copyright (C) 2017 ~ 2018 Wuhan Deepin Technology Co., Ltd.
 *
 * Author:     Iceyer <me@iceyer.net>
 *
 * Maintainer: Iceyer <me@iceyer.net>
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

#include "mediameta.h"

#include <QTime>
#include <QFileInfo>
#include <QCryptographicHash>

#include "util/pinyinsearch.h"
#include "metadetector.h"

using namespace DMusic;

void MediaMeta::updateSearchIndex()
{
    for (auto &str : PinyinSearch::simpleChineseSplit(this->title)) {
        this->pinyinTitle += str;
        this->pinyinTitleShort += str.at(0);
    }
    for (auto &str : PinyinSearch::simpleChineseSplit(this->singer)) {
        this->pinyinArtist += str;
        this->pinyinArtistShort += str.at(0);
    }
    for (auto &str : PinyinSearch::simpleChineseSplit(this->album)) {
        this->pinyinAlbum += str;
        this->pinyinAlbumShort += str.at(0);
    }
}

void MediaMeta::updateCodec(const QByteArray &codec)
{
    QFileInfo cueFi(this->cuePath);
    if (cueFi.exists()) {
        MetaDetector::updateCueFileTagCodec(*this, cueFi, codec);
    } else {
        MetaDetector::updateMediaFileTagCodec(*this, codec, true);
    }
}

void MediaMeta::getCoverData(const QString &tmpPath)
{
    // 直接获取图片，封面图片是否存在在使用前已经判断过
    MetaDetector::getCoverData(localPath, tmpPath, hash);
}

MediaMeta MediaMeta::fromLocalFile(const QFileInfo &fileInfo)
{
    MediaMeta meta;
    meta.hash = filepathHash(fileInfo.absoluteFilePath());
    MetaDetector::updateMetaFromLocalfile(meta, fileInfo);
    return  meta;
}

namespace DMusic {
QString filepathHash(const QString &filepath)
{
    return QString(QCryptographicHash::hash(filepath.toUtf8(), QCryptographicHash::Md5).toHex());
}

QString lengthString(qint64 length)
{
    length = length / 1000;
    int hour = static_cast<int>(length / 3600);

    QString mmStr = QString("%1").arg(length % 3600 / 60, 2, 10, QLatin1Char('0'));
    QString ssStr = QString("%1").arg(length % 60, 2, 10, QLatin1Char('0'));

    if (hour > 0) {
        return QString("%1:%2:%3").arg(hour).arg(mmStr).arg(ssStr);
    } else {
        return QString("%1:%2").arg(mmStr).arg(ssStr);
    }

}

QString sizeString(qint64 sizeByte)
{
    QString text;
    if (sizeByte < 1024) {
        text.sprintf("%.1fB", sizeByte / 1.0);
        return text;
    }
    if (sizeByte < 1024 * 1024) {
        text.sprintf("%.1fK", sizeByte / 1024.0);
        return text;
    }
    if (sizeByte < 1024 * 1024 * 1024) {
        text.sprintf("%.1fM", sizeByte / 1024.0 / 1024.0);
        return text;
    }
    text.sprintf("%.1fG", sizeByte / 1024.0 / 1024.0 / 1024.0);
    return text;
}

}
