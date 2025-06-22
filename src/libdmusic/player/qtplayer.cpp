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
    m_supportedSuffix << "wav"
                      << "ogg"
                      << "mp3"
                      << "flac";
}

QtPlayer::~QtPlayer()
{
    releasePlayer();
}

void QtPlayer::init()
{
    //防止多次创建
    if (m_mediaPlayer != nullptr) return;

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
        delete m_audioOutput;
        m_audioOutput = nullptr;
    }
    if (m_mediaPlayer) {
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
    if (m_mediaPlayer) {
        int state = m_mediaPlayer->playbackState();
        switch (state) {
        case QMediaPlayer::PlayingState: {
            return DmGlobal::Playing;
        }
        case QMediaPlayer::PausedState: {
            return DmGlobal::Paused;
        }
        default:
            return DmGlobal::Stopped;
        }
    } else {
        return DmGlobal::Stopped;
    }
}

void QtPlayer::play()
{
    init();
    qCDebug(dmMusic) << "Starting playback";
    m_mediaPlayer->play();
}

void QtPlayer::pause()
{
    if (m_mediaPlayer != nullptr && m_mediaPlayer->playbackState() == QMediaPlayer::PlayingState) {
        qCDebug(dmMusic) << "Pausing playback";
        m_mediaPlayer->pause();
    }
}

void QtPlayer::stop()
{
    // 播放和状态状态都可以停止播放
    if (m_mediaPlayer != nullptr && (m_mediaPlayer->playbackState() == QMediaPlayer::PlayingState
                                     || m_mediaPlayer->playbackState() == QMediaPlayer::PausedState)) {
        qCDebug(dmMusic) << "Stopping playback";
        m_mediaPlayer->stop();
    }

}

int QtPlayer::length()
{
    init();
    return static_cast<int>(m_mediaPlayer->duration());
}

void QtPlayer::setTime(qint64 time)
{
    init();
    if (m_mediaPlayer->mediaStatus() == QMediaPlayer::NoMedia)
        return;
    m_mediaPlayer->setPosition(time);
}

qint64 QtPlayer::time()
{
    init();
    if (m_mediaPlayer->mediaStatus() == QMediaPlayer::NoMedia) return -1;
    return m_mediaPlayer->position();
}

void QtPlayer::setMediaMeta(MediaMeta meta)
{
    init();
    if (m_activeMeta.hash == meta.hash) return;

    qCInfo(dmMusic) << "Setting media meta - Title:" << meta.title << "Path:" << meta.localPath;
    m_activeMeta = meta;
    m_mediaPlayer->setSource(QUrl::fromLocalFile(m_activeMeta.localPath));
    emit metaChanged();
}

bool QtPlayer::getMute()
{
    init();
    return m_audioOutput->isMuted();
}

void QtPlayer::setFadeInOutFactor(double fadeInOutFactor)
{
    init();
    m_audioOutput->setVolume(static_cast<int>(10 * fadeInOutFactor));
}

void QtPlayer::setVolume(int volume)
{
    init();
    qCDebug(dmMusic) << "Setting volume to:" << volume;
    m_volume = volume;
    m_audioOutput->setVolume(volume);
}

int QtPlayer::getVolume()
{
    init();
    return m_volume;
}

void QtPlayer::setMute(bool mute)
{
    init();
    qCDebug(dmMusic) << "Setting mute state to:" << mute;
    m_audioOutput->setMuted(mute);
}

void QtPlayer::onMediaStatusChanged(QMediaPlayer::MediaStatus status)
{
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
    init();
    if (m_mediaPlayer->duration() <= 0 || m_mediaPlayer->playbackState() != QMediaPlayer::PlayingState) return;
    m_currPositionChanged = position;
    float value = static_cast<float>(position) / m_mediaPlayer->duration();
    emit timeChanged(position);
    emit positionChanged(value);
    resetPlayInfo();
}

void QtPlayer::resetPlayInfo()
{
    // 防止重复
    if (!m_sinkInputPath.isEmpty()) return;
    readSinkInputPath();

    if (m_sinkInputPath.isEmpty()) {
        qCDebug(dmMusic) << "No sink input path found";
        return;
    }

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
}

void QtPlayer::readSinkInputPath()
{
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
}
