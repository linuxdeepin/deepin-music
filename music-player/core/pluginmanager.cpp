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

#include "pluginmanager.h"

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QPluginLoader>
#include <QApplication>

#include "plugininterface.h"

using namespace DMusic::Plugin;

PluginManager::PluginManager(QObject *parent) : QObject(parent)
{

}

QList<PluginInterface *> PluginManager::getPluginListByType(DMusic::Plugin::PluginType pluginType)
{
//    qDebug() << int(pluginType);
    QList<PluginInterface *> list;
    for (auto plugin : m_plugins) {
//        qDebug() << int(plugin->pluginType()) << int(pluginType);
//        qDebug() << plugin->pluginId();
        if (plugin->pluginType() == pluginType) {
            list << plugin;
        }
    }
    return list;
}

void PluginManager::loadPlugins()
{
    auto relativePath = QString("../lib/%1/plugins/").arg(qApp->applicationName());
    auto appDir = QDir(qApp->applicationDirPath());
    auto pluginsDir = QDir(appDir.absoluteFilePath(relativePath));
    if (!pluginsDir.exists()) {
        pluginsDir = QDir(QString("/usr/lib/%1/plugins").arg(qApp->applicationName()));
    }
    qDebug() << "load plugin from:" << pluginsDir.path();

    for (auto libraryFile : pluginsDir.entryInfoList(QStringList() << "*.so" << "*.dll")) {
        qDebug() << "load plugin so file:" << libraryFile.absoluteFilePath();
        auto loader = new QPluginLoader;
        loader->setFileName(libraryFile.absoluteFilePath());
        auto instance = loader->instance();
        auto pluginInstance =  dynamic_cast<PluginInterface *>(instance);
        qDebug() << "load plugin instance:" << instance << pluginInstance;
        emit onPluginLoaded(pluginInstance->pluginId(), pluginInstance);
        m_plugins.push_back(pluginInstance);
    }
}
