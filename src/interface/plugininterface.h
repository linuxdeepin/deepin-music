/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     ZouYa <zouya@uniontech.com>
 *
 * Maintainer: WangYu <wangyu@uniontech.com>
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
#include <QtGlobal>

class MediaMeta;

namespace DMusic {

namespace Plugin {

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
