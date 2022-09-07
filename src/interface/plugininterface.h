// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
