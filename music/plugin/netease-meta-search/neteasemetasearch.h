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

#include "neteasemetasearch_global.h"

#include <metasearchengine.h>
#include <mediameta.h>
#include <net/geese.h>

class NETEASEMETASEARCHSHARED_EXPORT NeteaseMetaSearchEngine:
    public DMusic::Plugin::MetaSearchEngine
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.Music.Plugin" FILE "NeteaseMetaSearchEngine.json")
    Q_INTERFACES(DMusic::Plugin::PluginInterface)
public:
    NeteaseMetaSearchEngine(QObject *parent = 0);

    virtual DMusic::Plugin::PluginType pluginType() const Q_DECL_OVERRIDE;
    virtual QString pluginId() const Q_DECL_OVERRIDE;

public slots:
    virtual QObject *getObject() Q_DECL_OVERRIDE;
    virtual void searchMeta(const MetaPtr meta) Q_DECL_OVERRIDE;
    virtual void searchContext(const QString &context);

private:
    DMusic::Net::Geese       *m_geese = nullptr;
};

