// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QScopedPointer>

#include <mediameta.h>

#include "util/singleton.h"

class MediaLibraryPrivate;
class MediaLibrary : public QObject, public DMusic::DSingleton<MediaLibrary>
{
    Q_OBJECT
public:
    ~MediaLibrary();

    MediaMeta creatMediaMeta(QString path);
    QStringList   getSupportedSuffixs();
signals:
    void mediaClean();
    void scanFinished(const QString &jobid, int mediaCount);

public slots:
    void init();
private:
    explicit MediaLibrary(QObject *parent = nullptr);
    friend class DMusic::DSingleton<MediaLibrary>;
};

