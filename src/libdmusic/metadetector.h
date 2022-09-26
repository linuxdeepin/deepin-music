// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QImage>

#include "util/singleton.h"

#include <mediameta.h>

class QFileInfo;
class LIBDMUSICSHARED_EXPORT MetaDetector: public QObject, public DMusic::DSingleton<MetaDetector>
{
public:
    void init();

    static QList<QByteArray> detectEncodings(const MediaMeta &meta);
    static QList<QByteArray> detectEncodings(const QByteArray &rawData);

    void updateCueFileTagCodec(MediaMeta &meta, const QFileInfo &, const QByteArray &codec);

    MediaMeta updateMetaFromLocalfile(MediaMeta meta, const QFileInfo &fileInfo, int engineType);
    MediaMeta updateMediaFileTagCodec(MediaMeta &meta, const QByteArray &codecName, bool forceEncode);

    static void getCoverData(const QString &path, const QString &tmpPath, const QString &hash, int engineType);
    static QPixmap getCoverDataPixmap(MediaMeta meta, int engineType);
private:
    explicit MetaDetector();
    friend class DMusic::DSingleton<MetaDetector>;
};
