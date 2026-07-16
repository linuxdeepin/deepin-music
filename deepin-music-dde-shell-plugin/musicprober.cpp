// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "musicprober.h"

#include <QDBusArgument>
#include <QDBusReply>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QFileInfo>
#include <QVariantMap>
#include <QDebug>

// MPRIS DBus constants
static constexpr auto MprisPath = "/org/mpris/MediaPlayer2";
static constexpr auto MprisRootInterface = "org.mpris.MediaPlayer2";
static constexpr auto MprisPlayerInterface = "org.mpris.MediaPlayer2.Player";
static constexpr auto DBusPropertiesInterface = "org.freedesktop.DBus.Properties";
static constexpr auto MprisServicePrefix = "org.mpris.MediaPlayer2.";

// Scoring weights
static constexpr int ScoreKnownService = 300;
static constexpr int ScorePlaying = 300;
static constexpr int ScorePaused = 200;
static constexpr int ScoreStopped = 100;
static constexpr int ScoreHasTitle = 30;
static constexpr int ScoreHasArt = 15;
static constexpr int ScoreCanControl = 10;
static constexpr int ScoreHasNavigation = 80;

QVariantMap MusicProber::dbusProperties(const QString &service, const QString &path, const QString &interfaceName)
{
    QDBusInterface propertiesInterface(service,
                                       path,
                                       DBusPropertiesInterface,
                                       QDBusConnection::sessionBus());
    const QDBusReply<QVariantMap> reply = propertiesInterface.call(QStringLiteral("GetAll"), interfaceName);
    return reply.isValid() ? reply.value() : QVariantMap();
}

QVariantMap MusicProber::dbusProperties(const QString &service, const QString &interfaceName)
{
    return dbusProperties(service, QLatin1String(MprisPath), interfaceName);
}

QVariantMap MusicProber::variantMapFromDBusArgument(const QDBusArgument &argument)
{
    QVariantMap result;
    if (argument.currentType() != QDBusArgument::MapType) {
        return result;
    }

    argument.beginMap();
    while (!argument.atEnd()) {
        QString key;
        QDBusVariant value;
        argument.beginMapEntry();
        argument >> key >> value;
        argument.endMapEntry();
        result.insert(key, value.variant());
    }
    argument.endMap();
    return result;
}

QVariant MusicProber::unwrapDBusValue(const QVariant &value)
{
    if (value.canConvert<QDBusVariant>()) {
        return qvariant_cast<QDBusVariant>(value).variant();
    }

    if (value.userType() == qMetaTypeId<QDBusArgument>()) {
        const QDBusArgument argument = qvariant_cast<QDBusArgument>(value);
        if (argument.currentType() == QDBusArgument::VariantType) {
            QDBusVariant unwrappedValue;
            argument >> unwrappedValue;
            return unwrapDBusValue(unwrappedValue.variant());
        }
    }

    return value;
}

QString MusicProber::stringFromDBusValue(const QVariant &value)
{
    return unwrapDBusValue(value).toString().trimmed();
}

QStringList MusicProber::stringListFromDBusValue(const QVariant &value)
{
    const QVariant unwrappedValue = unwrapDBusValue(value);
    if (unwrappedValue.canConvert<QStringList>()) {
        return unwrappedValue.toStringList();
    }

    QStringList strings;
    const QVariantList values = unwrappedValue.toList();
    for (const QVariant &entry : values) {
        const QString text = stringFromDBusValue(entry);
        if (!text.isEmpty()) {
            strings << text;
        }
    }

    return strings;
}

bool MusicProber::boolFromDBusValue(const QVariant &value)
{
    return unwrapDBusValue(value).toBool();
}

QVariantMap MusicProber::mapFromDBusValue(const QVariant &value)
{
    const QVariant unwrappedValue = unwrapDBusValue(value);
    if (unwrappedValue.userType() == qMetaTypeId<QDBusArgument>()) {
        return variantMapFromDBusArgument(qvariant_cast<QDBusArgument>(unwrappedValue));
    }

    if (unwrappedValue.canConvert<QVariantMap>()) {
        return unwrappedValue.toMap();
    }

    return {};
}

uint MusicProber::serviceProcessId(const QString &serviceName)
{
    if (serviceName.isEmpty()) {
        return 0;
    }

    QDBusConnectionInterface *connectionInterface = QDBusConnection::sessionBus().interface();
    if (!connectionInterface) {
        return 0;
    }

    const QDBusReply<uint> pidReply = connectionInterface->servicePid(serviceName);
    return pidReply.isValid() ? pidReply.value() : 0;
}

QString MusicProber::desktopEntryFromMprisService(const QString &serviceName)
{
    constexpr auto prefix = "org.mpris.MediaPlayer2.";
    if (!serviceName.startsWith(QLatin1String(prefix))) {
        return {};
    }

    // Strip the prefix to get the player name
    QString playerPart = serviceName.mid(QLatin1String(prefix).size());
    if (playerPart.startsWith(QLatin1String("chrome.")) || playerPart.startsWith(QLatin1String("firefox."))) {
        return {};
    }

    return playerPart.toLower();
}

QString MusicProber::joinMusicSubtitleParts(const QStringList &parts)
{
    QStringList filteredParts;
    for (const QString &part : parts) {
        const QString trimmedPart = part.trimmed();
        if (!trimmedPart.isEmpty() && !filteredParts.contains(trimmedPart)) {
            filteredParts << trimmedPart;
        }
    }

    return filteredParts.join(QStringLiteral(" - "));
}

QUrl MusicProber::resolveArtSource(const QString &artUrl)
{
    const QUrl sourceUrl(artUrl);
    if (!sourceUrl.isValid() || sourceUrl.isEmpty()) {
        return {};
    }

    // Non-local URLs (http(s), data, etc.) — pass through as-is; QML can load them.
    if (!sourceUrl.isLocalFile()) {
        return sourceUrl;
    }

    // Local file: validate existence to avoid broken images in the UI.
    const QString sourcePath = sourceUrl.toLocalFile();
    if (!QFileInfo::exists(sourcePath)) {
        return {};
    }

    // Return the original local path directly — deepin-music already caches
    // cover art at ~/.cache/deepin-music/images/{hash}.jpg and reports it
    // via MPRIS mpris:artUrl, so no additional copy is needed. For other
    // MPRIS players the path is also typically already stable.
    return sourceUrl;
}

MusicProber::MusicProber(QObject *parent)
    : QObject(parent)
{
    // Subscribe to NameOwnerChanged to detect MPRIS services appearing/disappearing
    QDBusConnection::sessionBus().connect(
        QString(),
        QString(),
        QLatin1String("org.freedesktop.DBus"),
        QLatin1String("NameOwnerChanged"),
        this,
        SLOT(onNameOwnerChanged(QString,QString,QString)));

    // Initial scan for already-running services
    rescanMprisServices();
}

MusicProber::~MusicProber()
{
    // Unsubscribe from all watched services
    for (const QString &service : m_watchedServices) {
        unsubscribeFromPlayer(service);
    }
}

void MusicProber::subscribeToPlayer(const QString &service)
{
    if (m_watchedServices.contains(service)) {
        return;
    }

    // Subscribe to PropertiesChanged on the player interface
    const bool ok = QDBusConnection::sessionBus().connect(
        service,
        QLatin1String(MprisPath),
        QLatin1String(DBusPropertiesInterface),
        QLatin1String("PropertiesChanged"),
        this,
        SLOT(onPlayerPropertiesChanged(QString,QVariantMap,QStringList)));

    if (ok) {
        m_watchedServices << service;
    } else {
        qWarning() << "Failed to subscribe to PropertiesChanged for" << service;
    }
}

void MusicProber::unsubscribeFromPlayer(const QString &service)
{
    QDBusConnection::sessionBus().disconnect(
        service,
        QLatin1String(MprisPath),
        QLatin1String(DBusPropertiesInterface),
        QLatin1String("PropertiesChanged"),
        this,
        SLOT(onPlayerPropertiesChanged(QString,QVariantMap,QStringList)));

    m_watchedServices.removeAll(service);
}

void MusicProber::rescanMprisServices()
{
    QDBusConnectionInterface *connectionInterface = QDBusConnection::sessionBus().interface();
    if (!connectionInterface) {
        return;
    }

    const QDBusReply<QStringList> namesReply = connectionInterface->registeredServiceNames();
    if (!namesReply.isValid()) {
        return;
    }

    for (const QString &serviceName : namesReply.value()) {
        if (serviceName.startsWith(QLatin1String(MprisServicePrefix))) {
            subscribeToPlayer(serviceName);
        }
    }
}

void MusicProber::onNameOwnerChanged(const QString &serviceName,
                                      const QString &oldOwner,
                                      const QString &newOwner)
{
    if (!serviceName.startsWith(QLatin1String(MprisServicePrefix))) {
        return;
    }

    if (!oldOwner.isEmpty() && newOwner.isEmpty()) {
        // Service vanished
        unsubscribeFromPlayer(serviceName);
        emit serviceVanished(serviceName);

        // Clear stale service reference
        if (serviceName == m_service) {
            m_service.clear();
        }

        emit propertiesChanged();
    } else if (oldOwner.isEmpty() && !newOwner.isEmpty()) {
        // Service appeared
        subscribeToPlayer(serviceName);
        emit serviceAppeared(serviceName);
        emit propertiesChanged();
    }
}

void MusicProber::onPlayerPropertiesChanged(const QString &interfaceName,
                                             const QVariantMap &changedProperties,
                                             const QStringList &invalidatedProperties)
{
    Q_UNUSED(invalidatedProperties)

    // Only react to Player or Root interface changes
    if (interfaceName != QLatin1String(MprisPlayerInterface)
        && interfaceName != QLatin1String(MprisRootInterface)) {
        return;
    }

    emit propertiesChanged();
}

MusicSnapshot MusicProber::buildMusicSnapshot(const QString &serviceName,
                                              const QVariantMap &playerProperties,
                                              const QVariantMap &rootProperties)
{
    MusicSnapshot snapshot;

    const QString identity = stringFromDBusValue(rootProperties.value(QStringLiteral("Identity")));
    const QString playbackStatus = stringFromDBusValue(playerProperties.value(QStringLiteral("PlaybackStatus")));
    const QVariantMap metadata = mapFromDBusValue(playerProperties.value(QStringLiteral("Metadata")));

    snapshot.service = serviceName;
    snapshot.desktopEntry = stringFromDBusValue(rootProperties.value(QStringLiteral("DesktopEntry")));
    if (snapshot.desktopEntry.isEmpty()) {
        snapshot.desktopEntry = desktopEntryFromMprisService(serviceName);
    }
    snapshot.appName = identity;
    snapshot.available = true;
    snapshot.playing = playbackStatus == QStringLiteral("Playing");
    snapshot.canGoPrevious = boolFromDBusValue(playerProperties.value(QStringLiteral("CanGoPrevious")));
    snapshot.canGoNext = boolFromDBusValue(playerProperties.value(QStringLiteral("CanGoNext")));
    snapshot.canTogglePlayback = boolFromDBusValue(playerProperties.value(QStringLiteral("CanPause")))
        || boolFromDBusValue(playerProperties.value(QStringLiteral("CanPlay")));

    snapshot.title = stringFromDBusValue(metadata.value(QStringLiteral("xesam:title")));
    snapshot.subtitle = joinMusicSubtitleParts(
        stringListFromDBusValue(metadata.value(QStringLiteral("xesam:artist"))));
    const QString artUrl = stringFromDBusValue(metadata.value(QStringLiteral("mpris:artUrl")));
    if (!artUrl.isEmpty()) {
        snapshot.artSource = resolveArtSource(artUrl);
    }

    // Scoring
    snapshot.score = 0;
    if (playbackStatus == QStringLiteral("Playing")) {
        snapshot.score += ScorePlaying;
    } else if (playbackStatus == QStringLiteral("Paused")) {
        snapshot.score += ScorePaused;
    } else {
        snapshot.score += ScoreStopped;
    }

    if (!snapshot.title.isEmpty()) {
        snapshot.score += ScoreHasTitle;
    }
    if (snapshot.artSource.isValid()) {
        snapshot.score += ScoreHasArt;
    }
    if (boolFromDBusValue(playerProperties.value(QStringLiteral("CanControl")))) {
        snapshot.score += ScoreCanControl;
    }

    if (snapshot.canGoPrevious || snapshot.canGoNext) {
        snapshot.score += ScoreHasNavigation;
    }

    return snapshot;
}

MusicSnapshot MusicProber::scanForMusicPlayer()
{
    MusicSnapshot bestSnapshot;

    // If we have a known service, try it first
    if (!m_service.isEmpty()) {
        const QVariantMap playerProperties = dbusProperties(m_service, QLatin1String(MprisPlayerInterface));
        if (!playerProperties.isEmpty()) {
            const QVariantMap rootProperties = dbusProperties(m_service, QLatin1String(MprisRootInterface));
            bestSnapshot = buildMusicSnapshot(m_service, playerProperties, rootProperties);
            bestSnapshot.score += ScoreKnownService;
        } else {
            // Known service is gone or no longer has player interface — clear it
            m_service.clear();
        }
    }

    // Scan all MPRIS services
    QDBusConnectionInterface *connectionInterface = QDBusConnection::sessionBus().interface();
    if (!connectionInterface) {
        return bestSnapshot;
    }

    const QDBusReply<QStringList> namesReply = connectionInterface->registeredServiceNames();
    if (!namesReply.isValid()) {
        return bestSnapshot;
    }

    const QStringList serviceNames = namesReply.value();
    for (const QString &serviceName : serviceNames) {
        if (!serviceName.startsWith(QLatin1String(MprisServicePrefix))) {
            continue;
        }

        // Skip if we already have this service
        if (serviceName == m_service && bestSnapshot.available) {
            continue;
        }

        const QVariantMap playerProperties = dbusProperties(serviceName, QLatin1String(MprisPlayerInterface));
        if (playerProperties.isEmpty()) {
            continue;
        }

        const uint servicePid = serviceProcessId(serviceName);
        if (servicePid == 0) {
            continue;
        }

        const QVariantMap rootProperties = dbusProperties(serviceName, QLatin1String(MprisRootInterface));
        MusicSnapshot snapshot = buildMusicSnapshot(serviceName, playerProperties, rootProperties);

        // Auto-update known service to the best scoring one
        if (!bestSnapshot.available || snapshot.score > bestSnapshot.score) {
            bestSnapshot = snapshot;
        }
    }

    // Persist the best service for subsequent control calls
    if (bestSnapshot.available && !bestSnapshot.service.isEmpty()) {
        m_service = bestSnapshot.service;
    }

    return bestSnapshot;
}

void MusicProber::playPrevious()
{
    if (m_service.isEmpty()) {
        qDebug() << "playPrevious: no MPRIS service available";
        return;
    }

    QDBusInterface playerInterface(m_service,
                                   QLatin1String(MprisPath),
                                   QLatin1String(MprisPlayerInterface),
                                   QDBusConnection::sessionBus());
    QDBusReply<void> reply = playerInterface.call(QStringLiteral("Previous"));
    if (!reply.isValid()) {
        qDebug() << "playPrevious failed:" << reply.error().message();
    }
}

void MusicProber::togglePlayback()
{
    if (m_service.isEmpty()) {
        qDebug() << "togglePlayback: no MPRIS service available";
        return;
    }

    QDBusInterface playerInterface(m_service,
                                   QLatin1String(MprisPath),
                                   QLatin1String(MprisPlayerInterface),
                                   QDBusConnection::sessionBus());
    QDBusReply<void> reply = playerInterface.call(QStringLiteral("PlayPause"));
    if (!reply.isValid()) {
        qDebug() << "togglePlayback failed:" << reply.error().message();
    }
}

void MusicProber::playNext()
{
    if (m_service.isEmpty()) {
        qDebug() << "playNext: no MPRIS service available";
        return;
    }

    QDBusInterface playerInterface(m_service,
                                   QLatin1String(MprisPath),
                                   QLatin1String(MprisPlayerInterface),
                                   QDBusConnection::sessionBus());
    QDBusReply<void> reply = playerInterface.call(QStringLiteral("Next"));
    if (!reply.isValid()) {
        qDebug() << "playNext failed:" << reply.error().message();
    }
}
