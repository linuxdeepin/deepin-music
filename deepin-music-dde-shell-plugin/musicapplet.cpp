// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "musicapplet.h"
#include "musicprober.h"

#include <QProcess>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDBusVariant>
#include <QIcon>
#include <DGuiApplicationHelper>

DGUI_USE_NAMESPACE

MusicApplet::MusicApplet(QObject *parent)
    : QObject(parent)
    , m_prober(new MusicProber(this))
{
    // Initial scan
    refreshMusicState();

    connect(m_prober, &MusicProber::propertiesChanged,
            this, &MusicApplet::markStateDirty);

    m_debounceTimer = new QTimer(this);
    m_debounceTimer->setSingleShot(true);
    m_debounceTimer->setInterval(250);
    connect(m_debounceTimer, &QTimer::timeout,
            this, &MusicApplet::refreshMusicState);
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

void MusicApplet::markStateDirty()
{
    m_stateDirty = true;
    if (m_debounceTimer) {
        m_debounceTimer->start();
    }
}

void MusicApplet::refreshMusicState()
{
    m_stateDirty = false;

    const MusicSnapshot snapshot = m_prober->scanForMusicPlayer();

    // When no MPRIS player is available, show translated defaults.
    const QString effectiveTitle = snapshot.available
        ? snapshot.title : tr("No music detected");
    const QString effectiveSubtitle = snapshot.available
        ? snapshot.subtitle : tr("Open player to start");
    const QString effectiveAppName = snapshot.available
        ? snapshot.appName : tr("Music");

    const bool changed = (
        m_musicAvailable != snapshot.available
        || m_titleText != effectiveTitle
        || m_subtitleText != effectiveSubtitle
        || m_appName != effectiveAppName
        || m_artSource != snapshot.artSource
        || m_musicPlaying != snapshot.playing
        || m_canGoPrevious != snapshot.canGoPrevious
        || m_canGoNext != snapshot.canGoNext
        || m_canTogglePlayback != snapshot.canTogglePlayback
    );

    if (changed) {
        m_musicAvailable = snapshot.available;
        m_titleText = effectiveTitle;
        m_subtitleText = effectiveSubtitle;
        m_appName = effectiveAppName;
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
    // Try to raise via MPRIS first (async to avoid blocking UI)
    const QString service = m_prober->service();
    if (!service.isEmpty()) {
        QDBusInterface rootInterface(service,
                                     QLatin1String("/org/mpris/MediaPlayer2"),
                                     QLatin1String("org.mpris.MediaPlayer2"),
                                     QDBusConnection::sessionBus());
        if (rootInterface.isValid()) {
            rootInterface.asyncCall(QStringLiteral("Raise"));
            return;
        }
    }

    // Fallback: launch deepin-music
    QProcess::startDetached(QStringLiteral("deepin-music"));
}

void MusicApplet::playPreviousTrack()
{
    m_prober->playPrevious();
    // MPRIS PropertiesChanged signal will drive markStateDirty → debounce refresh.
    // Also trigger explicitly in case the signal is delayed.
    markStateDirty();
}

void MusicApplet::toggleMusicPlayback()
{
    m_prober->togglePlayback();
    markStateDirty();
}

void MusicApplet::playNextTrack()
{
    m_prober->playNext();
    markStateDirty();
}
