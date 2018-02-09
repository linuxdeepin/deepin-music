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

    MetaPtr meta(const QString &hash);
    bool contains(const QString &hash) const;
    bool isEmpty() const;

    MetaPtrList importFile(const QString &filepath);
signals:
    void mediaClean();
    void scanFinished(const QString &jobid, int mediaCount);
    void meidaFileImported(const QString &jobid, MetaPtrList metalist);

public slots:
    void init();
    void removeMediaMetaList(const MetaPtrList metalist);
    void importMedias(const QString &jobid, const QStringList &urllist);

private:
    explicit MediaLibrary(QObject *parent = 0);
    friend class DMusic::DSingleton<MediaLibrary>;
    QScopedPointer<MediaLibraryPrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), MediaLibrary)
};

