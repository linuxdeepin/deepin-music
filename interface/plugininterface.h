/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#pragma once

#include <QObject>
#include <QtGlobal>

class MediaMeta;

namespace DMusic
{

namespace Plugin
{

enum class PluginType {
    TypeMetaSearchEngine = 0,
};

class PluginInterface
{
public:
    virtual ~PluginInterface() {}

    virtual PluginType pluginType() const = 0;
    virtual QString pluginId() const = 0;
};

}
}

QT_BEGIN_NAMESPACE
Q_DECLARE_INTERFACE(DMusic::Plugin::PluginInterface, "com.deepin.dde.Music.Plugin");
QT_END_NAMESPACE
