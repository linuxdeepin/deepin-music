// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "musicapplet.h"
#include "musicprober.h"

#include <QProcess>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QIcon>

MusicApplet::MusicApplet(QObject *parent)
    : QObject(parent)
    , m_prober(new MusicProber(this))
{
    // Seed the icon theme key from the current Qt theme name so the
    // initial image://theme URL is cache-consistent.
    m_iconThemeKey = QIcon::themeName();
    if (m_iconThemeKey.isEmpty()) {
        m_iconThemeKey = QStringLiteral("default");
    }

    // Initial scan
    refreshMusicState();

    connect(m_prober, &MusicProber::propertiesChanged,
            this, &MusicApplet::markStateDirty);

    m_debounceTimer = new QTimer(this);
    m_debounceTimer->setSingleShot(true);
    m_debounceTimer->setInterval(250);
    connect(m_debounceTimer, &QTimer::timeout,
            this, &MusicApplet::refreshMusicState);

    QTimer *timer = new QTimer(this);
    timer->setInterval(10000);
    connect(timer, &QTimer::timeout, this, [this]() {
        if (m_stateDirty) {
            refreshMusicState();
        }
    });
    timer->start();

    QDBusConnection::sessionBus().connect(
        QString(),  // any sender
        QLatin1String("/org/deepin/dde/Appearance1"),
        QLatin1String("org.freedesktop.DBus.Properties"),
        QLatin1String("PropertiesChanged"),
        this,
        SLOT(onAppearancePropertiesChanged(QString,QVariantMap,QStringList)));
}

MusicApplet::~MusicApplet()
{
    if (m_debounceTimer) {
        m_debounceTimer->stop();
    }
}

bool MusicApplet::musicAvailable() const { return m_musicAvailable; }
QString MusicApplet::titleText() const { return m_titleText; }
QString MusicApplet::subtitleText() const { return m_subtitleText; }
QString MusicApplet::appName() const { return m_appName; }
QUrl MusicApplet::artSource() const { return m_artSource; }
bool MusicApplet::musicPlaying() const { return m_musicPlaying; }
bool MusicApplet::canGoPrevious() const { return m_canGoPrevious; }
bool MusicApplet::canGoNext() const { return m_canGoNext; }
bool MusicApplet::canTogglePlayback() const { return m_canTogglePlayback; }
QString MusicApplet::iconThemeKey() const { return m_iconThemeKey; }

void MusicApplet::markStateDirty()
{
    m_stateDirty = true;
    if (m_debounceTimer) {
        m_debounceTimer->start();
    }
}

void MusicApplet::onAppearancePropertiesChanged(const QString &interfaceName,
                                               const QVariantMap &changedProperties,
                                               const QStringList &invalidatedProperties)
{
    Q_UNUSED(interfaceName)
    Q_UNUSED(invalidatedProperties)

    // DDE Appearance service reports icon-theme changes via the IconTheme property.
    if (!changedProperties.contains(QStringLiteral("IconTheme"))) {
        return;
    }

    const QString newTheme = changedProperties.value(QStringLiteral("IconTheme")).toString();
    if (newTheme.isEmpty() || newTheme == m_iconThemeKey) {
        return;
    }

    // Keep Qt's icon theme in sync so QIcon::fromTheme resolves from the new theme.
    QIcon::setThemeName(newTheme);
    m_iconThemeKey = newTheme;
    emit iconThemeKeyChanged();
}

void MusicApplet::refreshMusicState()
{
    m_stateDirty = false;

    const MusicSnapshot snapshot = m_prober->scanForMusicPlayer();

    if (!snapshot.available) {
        if (m_musicAvailable) {
            m_musicAvailable = false;
            m_titleText = QStringLiteral("未检测到音乐");
            m_subtitleText = QStringLiteral("打开播放器开始播放");
            m_appName = QStringLiteral("音乐");
            m_artSource = QUrl();
            m_musicPlaying = false;
            m_canGoPrevious = false;
            m_canGoNext = false;
            m_canTogglePlayback = false;
            emit musicStateChanged();
        }
        return;
    }

    bool changed = false;

    if (m_musicAvailable != snapshot.available
        || m_titleText != snapshot.title
        || m_subtitleText != snapshot.subtitle
        || m_appName != snapshot.appName
        || m_artSource != snapshot.artSource
        || m_musicPlaying != snapshot.playing
        || m_canGoPrevious != snapshot.canGoPrevious
        || m_canGoNext != snapshot.canGoNext
        || m_canTogglePlayback != snapshot.canTogglePlayback) {
        changed = true;
    }

    if (changed) {
        m_musicAvailable = snapshot.available;
        m_titleText = snapshot.title;
        m_subtitleText = snapshot.subtitle;
        m_appName = snapshot.appName;
        m_artSource = snapshot.artSource;
        m_musicPlaying = snapshot.playing;
        m_canGoPrevious = snapshot.canGoPrevious;
        m_canGoNext = snapshot.canGoNext;
        m_canTogglePlayback = snapshot.canTogglePlayback;
        emit musicStateChanged();
    }
}

void MusicApplet::openMusicPlayer()
{
    // Try to raise via MPRIS first
    const QString service = m_prober->service();
    if (!service.isEmpty()) {
        QDBusInterface rootInterface(service,
                                     QLatin1String("/org/mpris/MediaPlayer2"),
                                     QLatin1String("org.mpris.MediaPlayer2"),
                                     QDBusConnection::sessionBus());
        if (rootInterface.isValid()) {
            rootInterface.call(QStringLiteral("Raise"));
            return;
        }
    }

    // Fallback: launch deepin-music
    QProcess::startDetached(QStringLiteral("deepin-music"));
}

void MusicApplet::playPreviousTrack()
{
    m_prober->playPrevious();
    QTimer::singleShot(200, this, &MusicApplet::refreshMusicState);
}

void MusicApplet::toggleMusicPlayback()
{
    m_prober->togglePlayback();
    QTimer::singleShot(200, this, &MusicApplet::refreshMusicState);
}

void MusicApplet::playNextTrack()
{
    m_prober->playNext();
    QTimer::singleShot(200, this, &MusicApplet::refreshMusicState);
}
