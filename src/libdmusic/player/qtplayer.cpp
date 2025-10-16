// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "qtplayer.h"
#include "utils.h"
#include "util/log.h"

#include <QAudioOutput>
#include <QDBusObjectPath>
#include <QDBusInterface>

QtPlayer::QtPlayer(QObject *parent)
    : PlayerBase(parent)
{
    qCDebug(dmMusic) << "QtPlayer constructor";
    m_supportedSuffix << "wav"
                      << "ogg"
                      << "mp3"
                      << "flac"
                      << "opus";
}

QtPlayer::~QtPlayer()
{
    qCDebug(dmMusic) << "QtPlayer destructor";
    releasePlayer();
}

void QtPlayer::init()
{
    qCDebug(dmMusic) << "Initializing QtPlayer";
    //防止多次创建
    if (m_mediaPlayer != nullptr) {
        qCDebug(dmMusic) << "QtPlayer already initialized, returning";
        return;
    }

    qCDebug(dmMusic) << "Creating QMediaPlayer instance";
    m_mediaPlayer = new QMediaPlayer(this);
    m_audioOutput = new QAudioOutput(this);
    m_mediaPlayer->setAudioOutput(m_audioOutput);

    connect(m_mediaPlayer, &QMediaPlayer::mediaStatusChanged, this, &QtPlayer::onMediaStatusChanged);
    connect(m_mediaPlayer, &QMediaPlayer::positionChanged, this, &QtPlayer::onPositionChanged);
    connect(m_mediaPlayer, &QMediaPlayer::playbackStateChanged, this, [ = ](QMediaPlayer::PlaybackState newState) {
        DmGlobal::PlaybackStatus state = DmGlobal::Stopped;
        switch (newState) {
        case QMediaPlayer::PlayingState: {
            state = DmGlobal::Playing;
            qCInfo(dmMusic) << "QMediaPlayer state changed to Playing";
        }
        break;
        case QMediaPlayer::PausedState: {
            state = DmGlobal::Paused;
            qCInfo(dmMusic) << "QMediaPlayer state changed to Paused";
        }
        break;
        default:
            state = DmGlobal::Stopped;
            qCInfo(dmMusic) << "QMediaPlayer state changed to Stopped";
            break;
        }
        emit stateChanged(state);
    });
    connect(m_audioOutput, &QAudioOutput::mutedChanged, this, &QtPlayer::signalMutedChanged);
}


void QtPlayer::releasePlayer()
{
    qCDebug(dmMusic) << "Releasing QMediaPlayer resources";
    if (m_audioOutput) {
        qCDebug(dmMusic) << "Deleting QAudioOutput instance";
        delete m_audioOutput;
        m_audioOutput = nullptr;
    }
    if (m_mediaPlayer) {
        qCDebug(dmMusic) << "Deleting QMediaPlayer instance";
        delete m_mediaPlayer;
        m_mediaPlayer = nullptr;
    }
}


void QtPlayer::release()
{
    qCDebug(dmMusic) << "Release requested";
    releasePlayer();
}


DmGlobal::PlaybackStatus QtPlayer::state()
{
    qCDebug(dmMusic) << "Getting current playback state";
    if (m_mediaPlayer) {
        qCDebug(dmMusic) << "Media player instance exists, checking state";
        int state = m_mediaPlayer->playbackState();
        switch (state) {
        case QMediaPlayer::PlayingState: {
            qCDebug(dmMusic) << "Current state: Playing";
            return DmGlobal::Playing;
        }
        case QMediaPlayer::PausedState: {
            qCDebug(dmMusic) << "Current state: Paused";
            return DmGlobal::Paused;
        }
        default:
            qCDebug(dmMusic) << "Current state: Stopped";
            return DmGlobal::Stopped;
        }
    } else {
        qCDebug(dmMusic) << "No media player instance, state: Stopped";
        return DmGlobal::Stopped;
    }
}

void QtPlayer::play()
{
    qCDebug(dmMusic) << "Play requested";
    init();
    qCDebug(dmMusic) << "Starting playback";
    m_mediaPlayer->play();
}

void QtPlayer::pause()
{
    qCDebug(dmMusic) << "Pause requested";
    if (m_mediaPlayer != nullptr && m_mediaPlayer->playbackState() == QMediaPlayer::PlayingState) {
        qCDebug(dmMusic) << "Pausing playback";
        m_mediaPlayer->pause();
    }
}

void QtPlayer::stop()
{
    qCDebug(dmMusic) << "Stop requested";
    // 播放和状态状态都可以停止播放
    if (m_mediaPlayer != nullptr && (m_mediaPlayer->playbackState() == QMediaPlayer::PlayingState
                                     || m_mediaPlayer->playbackState() == QMediaPlayer::PausedState)) {
        qCDebug(dmMusic) << "Stopping playback";
        m_mediaPlayer->stop();
    }

}

int QtPlayer::length()
{
    qCDebug(dmMusic) << "Getting media duration";
    init();
    return static_cast<int>(m_mediaPlayer->duration());
}

void QtPlayer::setTime(qint64 time)
{
    qCDebug(dmMusic) << "Set time requested:" << time;
    init();
    if (m_mediaPlayer->mediaStatus() == QMediaPlayer::NoMedia) {
        qCDebug(dmMusic) << "Cannot set time - no media loaded";
        return;
    }
    qCDebug(dmMusic) << "Setting playback position to:" << time;
    m_mediaPlayer->setPosition(time);
}

qint64 QtPlayer::time()
{
    qCDebug(dmMusic) << "Get current playback position";
    init();
    if (m_mediaPlayer->mediaStatus() == QMediaPlayer::NoMedia) {
        qCDebug(dmMusic) << "No media loaded, returning invalid time";
        return -1;
    }
    qint64 pos = m_mediaPlayer->position();
    qCDebug(dmMusic) << "Current playback position:" << pos;
    return pos;
}

void QtPlayer::setMediaMeta(MediaMeta meta)
{
    qCDebug(dmMusic) << "Set media meta requested";
    init();
    if (m_activeMeta.hash == meta.hash) return;

    qCInfo(dmMusic) << "Setting media meta - Title:" << meta.title << "Path:" << meta.localPath;
    m_activeMeta = meta;
    m_mediaPlayer->setSource(QUrl::fromLocalFile(m_activeMeta.localPath));
    emit metaChanged();
}

bool QtPlayer::getMute()
{
    qCDebug(dmMusic) << "Get mute state";
    init();
    return m_audioOutput->isMuted();
}

void QtPlayer::setFadeInOutFactor(double fadeInOutFactor)
{
    qCDebug(dmMusic) << "Set fade in/out factor";
    init();
    m_audioOutput->setVolume(static_cast<int>(10 * fadeInOutFactor));
}

void QtPlayer::setVolume(int volume)
{
    qCDebug(dmMusic) << "Set volume requested:" << volume;
    init();
    qCDebug(dmMusic) << "Setting volume to:" << volume;
    m_volume = volume;
    m_audioOutput->setVolume(volume);
}

int QtPlayer::getVolume()
{
    qCDebug(dmMusic) << "Get current volume";
    init();
    return m_volume;
}

void QtPlayer::setMute(bool mute)
{
    qCDebug(dmMusic) << "Set mute state requested:" << mute;
    init();
    qCDebug(dmMusic) << "Setting mute state to:" << mute;
    m_audioOutput->setMuted(mute);
}

void QtPlayer::onMediaStatusChanged(QMediaPlayer::MediaStatus status)
{
    qCDebug(dmMusic) << "Media status changed to:" << status;
    if (status == QMediaPlayer::MediaStatus::EndOfMedia) {
        qCInfo(dmMusic) << "Media playback reached end";
        emit end();
    } else if (status == QMediaPlayer::MediaStatus::InvalidMedia) {
        qCWarning(dmMusic) << "Invalid media detected for:" << m_activeMeta.localPath;
        emit end();
    }
}

void QtPlayer::onPositionChanged(qint64 position)
{
    qCDebug(dmMusic) << "Position changed to:" << position;
    init();
    if (m_mediaPlayer->duration() <= 0 || m_mediaPlayer->playbackState() != QMediaPlayer::PlayingState) {
        qCDebug(dmMusic) << "Ignoring position change - invalid duration or not playing";
        return;
    }
    m_currPositionChanged = position;
    float value = static_cast<float>(position) / m_mediaPlayer->duration();
    qCDebug(dmMusic) << "Position changed:" << position << "(" << value * 100 << "%)";
    emit timeChanged(position);
    emit positionChanged(value);
    resetPlayInfo();
    qCDebug(dmMusic) << "Emitted positionChanged signal";
}

void QtPlayer::resetPlayInfo()
{
    qCDebug(dmMusic) << "Resetting play info";
    // 防止重复
    if (!m_sinkInputPath.isEmpty()) {
        qCDebug(dmMusic) << "Sink input path already set, skipping reset";
        return;
    }
    readSinkInputPath();

    if (m_sinkInputPath.isEmpty()) {
        qCDebug(dmMusic) << "No sink input path found";
        return;
    }
    qCDebug(dmMusic) << "Resetting play info for sink:" << m_sinkInputPath;

    QVariant volumeV = Utils::readDBusProperty("org.deepin.dde.Audio1", m_sinkInputPath,
                                               "org.deepin.dde.Audio1.SinkInput", "Volume");

    QVariant muteV = Utils::readDBusProperty("org.deepin.dde.Audio1", m_sinkInputPath,
                                             "org.deepin.dde.Audio1.SinkInput", "Mute");

    if (!volumeV.isValid() || !muteV.isValid()) {
        qCWarning(dmMusic) << "Failed to read audio properties from DBus";
        return;
    }

    QDBusInterface ainterface("org.deepin.dde.Audio1", m_sinkInputPath,
                              "org.deepin.dde.Audio1.SinkInput",
                              QDBusConnection::sessionBus());
    if (!ainterface.isValid()) {
        qCWarning(dmMusic) << "Failed to create DBus interface for audio";
        return;
    }

    if (!qFuzzyCompare(volumeV.toDouble(), 1.0)) {
        qCDebug(dmMusic) << "Normalizing audio volume to 1.0";
        ainterface.call(QLatin1String("SetVolume"), 1.0, false);
    }

    if (muteV.toBool()) {
        qCDebug(dmMusic) << "Unmuting audio";
        ainterface.call(QLatin1String("SetMute"), false);
    }
    qCDebug(dmMusic) << "Sink input path reset complete";
}

void QtPlayer::readSinkInputPath()
{
    qCDebug(dmMusic) << "Reading sink input path";
    QVariant v = Utils::readDBusProperty("org.deepin.dde.Audio1", "/org/deepin/dde/Audio1",
                                         "org.deepin.dde.Audio1", "SinkInputs");

    if (!v.isValid()) {
        qCWarning(dmMusic) << "Failed to read sink inputs from DBus";
        return;
    }

    QList<QDBusObjectPath> allSinkInputsList = v.value<QList<QDBusObjectPath> >();
    qCDebug(dmMusic) << "Found" << allSinkInputsList.size() << "sink inputs";

    for (auto curPath : allSinkInputsList) {
        QVariant nameV = Utils::readDBusProperty("org.deepin.dde.Audio1", curPath.path(),
                                                 "org.deepin.dde.Audio1.SinkInput", "Name");

        if (!nameV.isValid() || (nameV.toString() != "Deepin Music" && nameV.toString() != "deepin-music"))
            continue;

        m_sinkInputPath = curPath.path();
        qCDebug(dmMusic) << "Found Deepin Music sink input at:" << m_sinkInputPath;
        break;
    }
    qCDebug(dmMusic) << "Sink input path read complete";
}
