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
#include "plugininterface.h"

#include <mediameta.h>
#include <searchmeta.h>

class MediaMeta;

namespace DMusic
{
namespace Plugin
{

class MetaSearchEngine : public QObject, public PluginInterface
{
    Q_OBJECT
public:
    explicit MetaSearchEngine(QObject *parent = 0) : QObject(parent) {}

signals:
    void lyricLoaded(const MetaPtr meta,
                     const DMusic::SearchMeta &search,
                     const QByteArray &lyricData);
    void coverLoaded(const MetaPtr meta,
                     const DMusic::SearchMeta &search,
                     const QByteArray &coverData);

    void metaSearchFinished(const MetaPtr meta);
    void doSearchMeta(const MetaPtr meta);

    void contextSearchFinished(const QString &context, const QList<DMusic::SearchMeta> &metalist);
    void doSearchContext(const QString &context);

public:
    virtual QObject *getObject() = 0 ;
    virtual void searchMeta(const MetaPtr meta) = 0;
};

}
}
