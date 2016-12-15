/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#pragma once

#include "netease-meta-search_global.h"

#include <metasearchengine.h>

class NETEASEMETASEARCHSHARED_EXPORT NeteaseMetaSearchEngine:
    public DMusic::Plugin::MetaSearchEngine
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.Music.Plugin" FILE "NeteaseMetaSearchEngine.json")
    Q_INTERFACES(DMusic::Plugin::PluginInterface)
public:

    NeteaseMetaSearchEngine(QObject *parent = 0);

    virtual DMusic::Plugin::PluginType pluginType() const;
    virtual QString pluginId() const;
    virtual void searchMeta(const MusicMeta &meta);
};

