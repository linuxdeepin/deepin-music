// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MUSICPLUGIN_H
#define MUSICPLUGIN_H

#include "pluginsiteminterface_v3.h"
#include "musicapplet.h"

#include <QPointer>
#include <QScopedPointer>

class QQuickView;

class MusicPlugin : public QObject, public PluginsItemInterfaceV3
{
    Q_OBJECT
    Q_INTERFACES(PluginsItemInterfaceV3)
    Q_PLUGIN_METADATA(IID ModuleInterface_iid_V3 FILE "music.json")

public:
    explicit MusicPlugin(QObject *parent = nullptr);
    ~MusicPlugin() override;

    const QString pluginName() const override;
    const QString pluginDisplayName() const override;
    void init(PluginProxyInterface *proxyInter) override;
    QWidget *itemWidget(const QString &itemKey) override;

    Dock::PluginFlags flags() const override;
    bool pluginIsDisable() override;
    void pluginStateSwitched() override;
    int itemSortKey(const QString &itemKey) override;
    void setSortKey(const QString &itemKey, const int order) override;
    const QString itemContextMenu(const QString &itemKey) override;
    void invokedMenuItem(const QString &itemKey, const QString &menuId, const bool checked) override;
    void refreshIcon(const QString &itemKey) override;

    QString cardItemKey() const override;
    QWindow *cardWindow() const override;

private:
    void refreshPluginItemsVisible();

    QScopedPointer<MusicApplet> m_applet;
    mutable QPointer<QQuickView> m_cardView;
};

#endif // MUSICPLUGIN_H
