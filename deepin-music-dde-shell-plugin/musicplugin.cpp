// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "musicplugin.h"

#include <QQuickView>
#include <QQmlContext>
#include <QQmlEngine>
#include <QJsonDocument>
#include <QJsonObject>
#include <QCoreApplication>
#include <DGuiApplicationHelper>
#include <DStandardPaths>

DGUI_USE_NAMESPACE
DCORE_USE_NAMESPACE

#define MUSIC_KEY "music"
#define STATE_KEY "enable"

// ── MusicPlugin ────────────────────────────────────────────────────────

MusicPlugin::MusicPlugin(QObject *parent)
    : QObject(parent)
{
}

MusicPlugin::~MusicPlugin()
{
    if (m_cardView) {
        // Close the scene graph first to stop render thread activity,
        // then delete. QPointer auto-nulls after deletion.
        m_cardView->close();
        delete m_cardView;
    }
}

const QString MusicPlugin::pluginName() const
{
    return QStringLiteral("music");
}

const QString MusicPlugin::pluginDisplayName() const
{
    return QCoreApplication::translate("Plugin.DisplayName", "Music");
}

void MusicPlugin::init(PluginProxyInterface *proxyInter)
{
    m_proxyInter = proxyInter;

    const QString trName = QStringLiteral("deepin-music-dde-shell-plugin");
    QStringList trDirs;
    const auto dataDirs = DStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
    for (const auto &dir : dataDirs) {
        trDirs << QDir(dir).filePath(QStringLiteral("deepin-music-dde-shell-plugin/translations"));
    }
    DGuiApplicationHelper::loadTranslator(trName, trDirs, { QLocale::system() });

    m_applet.reset(new MusicApplet(this));

    refreshPluginItemsVisible();
}

QWidget *MusicPlugin::itemWidget(const QString &itemKey)
{
    Q_UNUSED(itemKey)
    return nullptr;
}

Dock::PluginFlags MusicPlugin::flags() const
{
    return Dock::Attribute_HasCard;
}

bool MusicPlugin::pluginIsDisable()
{
    return !m_proxyInter->getValue(this, STATE_KEY, true).toBool();
}

void MusicPlugin::pluginStateSwitched()
{
    m_proxyInter->saveValue(this, STATE_KEY, pluginIsDisable());
    refreshPluginItemsVisible();
}

int MusicPlugin::itemSortKey(const QString &itemKey)
{
    const QString key = QString("pos_%1_%2").arg(itemKey).arg(Dock::Efficient);
    return m_proxyInter->getValue(this, key, -1).toInt();
}

void MusicPlugin::setSortKey(const QString &itemKey, const int order)
{
    const QString key = QString("pos_%1_%2").arg(itemKey).arg(Dock::Efficient);
    m_proxyInter->saveValue(this, key, order);
}

const QString MusicPlugin::itemContextMenu(const QString &itemKey)
{
    Q_UNUSED(itemKey)

    QJsonObject menuObj;
    QJsonArray items;

    QJsonObject openItem;
    openItem["itemId"] = "open";
    openItem["itemText"] = QCoreApplication::translate("Plugin.OpenPlayer", "Open");
    openItem["isActive"] = true;
    items.append(openItem);

    menuObj["items"] = items;
    return QJsonDocument(menuObj).toJson();
}

void MusicPlugin::invokedMenuItem(const QString &itemKey, const QString &menuId, const bool checked)
{
    Q_UNUSED(itemKey)
    Q_UNUSED(checked)

    if (menuId == QLatin1String("open")) {
        m_applet->openMusicPlayer();
    }
}

void MusicPlugin::refreshIcon(const QString &itemKey)
{
    Q_UNUSED(itemKey)
}

QString MusicPlugin::cardItemKey() const
{
    return QStringLiteral("music-card");
}

QWindow *MusicPlugin::cardWindow() const
{
    if (m_cardView) {
        return m_cardView;
    }

    auto view = new QQuickView;
    view->setColor(Qt::transparent);
    view->setResizeMode(QQuickView::SizeRootObjectToView);
    view->rootContext()->setContextProperty(QStringLiteral("musicApplet"), m_applet.data());
    view->setSource(QUrl(QStringLiteral("qrc:/music/MusicCard.qml")));
    if (view->status() == QQuickView::Error) {
        delete view;
        return nullptr;
    }

    QQmlEngine::setObjectOwnership(view, QQmlEngine::CppOwnership);
    // No destroyed signal connection — cleanup is handled in the destructor
    // via m_cardView (QPointer). Connecting to destroyed would cause a
    // double-free since the destructor already deletes the view.
    m_cardView = view;
    return m_cardView;
}

void MusicPlugin::refreshPluginItemsVisible()
{
    if (!m_proxyInter) {
        return;
    }

    if (pluginIsDisable()) {
        m_proxyInter->itemRemoved(this, MUSIC_KEY);
        return;
    }

    m_proxyInter->itemAdded(this, MUSIC_KEY);
}

