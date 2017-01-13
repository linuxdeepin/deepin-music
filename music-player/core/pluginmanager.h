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
#include <QtPlugin>

#include "util/singleton.h"

namespace DMusic
{
namespace Plugin
{
class PluginInterface;
enum class PluginType;
}
}

class PluginManager : public QObject, public DMusic::DSingleton<PluginManager>
{
    Q_OBJECT
    friend class DMusic::DSingleton<PluginManager>;

public:
    explicit PluginManager(QObject *parent = 0);

    void init() {loadPlugins();}

    QList<DMusic::Plugin::PluginInterface *> getPluginListByType(DMusic::Plugin::PluginType pluginType);

signals:
    void onPluginLoaded(const QString &, DMusic::Plugin::PluginInterface *);

public slots:

private:
    void loadPlugins();

    QList<DMusic::Plugin::PluginInterface *> m_plugins;
};
