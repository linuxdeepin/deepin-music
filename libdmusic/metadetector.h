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

#pragma once

#include "util/singleton.h"

#include <mediameta.h>

class QFileInfo;
class LIBDMUSICSHARED_EXPORT MetaDetector
{
public:
    static void init();

    static QList<QByteArray> detectEncodings(const MetaPtr meta);
    static QList<QByteArray> detectEncodings(const QByteArray &rawData);

    static void updateCueFileTagCodec(MediaMeta *meta, const QFileInfo &, const QByteArray &codec);
    static void updateMediaFileTagCodec(MediaMeta *meta, const QByteArray &codec, bool forceEncode);
    static void updateMetaFromLocalfile(MediaMeta *meta, const QFileInfo &fileInfo);
};
