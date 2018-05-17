/*
 * Copyright (C) 2016 ~ 2018 Wuhan Deepin Technology Co., Ltd.
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

#include <util/singleton.h>
#include <mediameta.h>
#include <searchmeta.h>

class MetaSearchServicePrivate;
class MetaSearchService : public QObject, public DMusic::DSingleton<MetaSearchService>
{
    Q_OBJECT

public:
    void init();
    ~MetaSearchService();

    static QUrl coverUrl(const MetaPtr meta);
    static QUrl lyricUrl(const MetaPtr meta);
    static QByteArray coverData(const MetaPtr meta);
    static QByteArray lyricData(const MetaPtr meta);

signals:
    void lyricSearchFinished(const MetaPtr meta, const DMusic::SearchMeta & song, const QByteArray &lyricData);
    void coverSearchFinished(const MetaPtr meta, const DMusic::SearchMeta & song,const QByteArray &coverData);
    void contextSearchFinished(const QString &context, const QList<DMusic::SearchMeta> &metalist);

public slots:
    void searchMeta(const MetaPtr meta);
    void searchContext(const QString &context);
    void onChangeMetaCache(const MetaPtr meta, const DMusic::SearchMeta &search);

private:
    explicit MetaSearchService(QObject *parent = 0);

    friend class DMusic::DSingleton<MetaSearchService>;

    QScopedPointer<MetaSearchServicePrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), MetaSearchService)
};

