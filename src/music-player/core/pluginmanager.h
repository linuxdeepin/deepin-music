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
