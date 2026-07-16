// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QString>
#include <QStringList>
#include <QUrl>

struct MusicSnapshot
{
    QString service;
    QString desktopEntry;
    QString appName;
    QString title = QStringLiteral("未检测到音乐");
    QString subtitle = QStringLiteral("打开播放器开始播放");
    QUrl artSource;
    bool available = false;
    bool playing = false;
    bool canGoPrevious = false;
    bool canGoNext = false;
    bool canTogglePlayback = false;
    int score = std::numeric_limits<int>::min();
};

class MusicProber : public QObject
{
    Q_OBJECT

public:
    explicit MusicProber(QObject *parent = nullptr);
    ~MusicProber() override;

    MusicSnapshot scanForMusicPlayer();
    QString service() const { return m_service; }

    Q_INVOKABLE void playPrevious();
    Q_INVOKABLE void togglePlayback();
    Q_INVOKABLE void playNext();

signals:
    void propertiesChanged();
    void serviceAppeared(const QString &service);
    void serviceVanished(const QString &service);

private slots:
    void onPlayerPropertiesChanged(const QString &interfaceName,
                                    const QVariantMap &changedProperties,
                                    const QStringList &invalidatedProperties);
    void onNameOwnerChanged(const QString &serviceName,
                            const QString &oldOwner,
                            const QString &newOwner);

private:
    // DBus helpers
    static QVariantMap dbusProperties(const QString &service, const QString &path, const QString &interfaceName);
    static QVariantMap dbusProperties(const QString &service, const QString &interfaceName);
    static QString stringFromDBusValue(const QVariant &value);
    static QStringList stringListFromDBusValue(const QVariant &value);
    static bool boolFromDBusValue(const QVariant &value);
    static QVariantMap mapFromDBusValue(const QVariant &value);
    static QVariant unwrapDBusValue(const QVariant &value);
    static QVariantMap variantMapFromDBusArgument(const QDBusArgument &argument);
    static uint serviceProcessId(const QString &serviceName);

    // Music-specific helpers
    static QString desktopEntryFromMprisService(const QString &serviceName);
    static QString joinMusicSubtitleParts(const QStringList &parts);
    static QUrl resolveArtSource(const QString &artUrl);

    // Snapshot building (extracted to deduplicate scanForMusicPlayer)
    static MusicSnapshot buildMusicSnapshot(const QString &serviceName,
                                             const QVariantMap &playerProperties,
                                             const QVariantMap &rootProperties);

    // Signal subscription management
    void subscribeToPlayer(const QString &service);
    void unsubscribeFromPlayer(const QString &service);
    void rescanMprisServices();

    QString m_service;
    QStringList m_watchedServices;
};
