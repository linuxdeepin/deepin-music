/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

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
    qDebug() << int(pluginType);
    QList<PluginInterface *> list;
    for (auto plugin : m_plugins) {
        qDebug() << int(plugin->pluginType()) << int(pluginType);
        qDebug() << plugin->pluginId();
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
    qDebug() << "load plugins from" << pluginsDir.path();
    if (!pluginsDir.exists()) {
        pluginsDir = QDir(QString("/usr/lib/%1/plugins").arg(qApp->applicationName()));
    }
    qDebug() << "load plugins from" << pluginsDir.path();

    for (auto libraryFile : pluginsDir.entryInfoList(QStringList() << "*.so")) {
        qDebug() << libraryFile.absoluteFilePath();
        auto loader = new QPluginLoader;
        loader->setFileName(libraryFile.absoluteFilePath());
        auto instance = loader->instance();
        qDebug() << "load" << libraryFile.absoluteFilePath() << instance;
        auto pluginInstance =  dynamic_cast<PluginInterface *>(instance);
        qDebug() << "load" << libraryFile.absoluteFilePath() << pluginInstance;
        emit onPluginLoaded(loader->objectName(), pluginInstance);
        m_plugins.push_back(pluginInstance);
    }
}
