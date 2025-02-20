// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "qtplayer.h"
#include "utils.h"

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
        }
        break;
        case QMediaPlayer::PausedState: {
            state = DmGlobal::Paused;
        }
        break;
        default:
            state = DmGlobal::Stopped;
            break;
        }
        emit stateChanged(state);
    });
    connect(m_audioOutput, &QAudioOutput::mutedChanged, this, &QtPlayer::signalMutedChanged);
}


void QtPlayer::releasePlayer()
{
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
    m_mediaPlayer->play();
}

void QtPlayer::pause()
{
    if (m_mediaPlayer != nullptr && m_mediaPlayer->playbackState() == QMediaPlayer::PlayingState)
        m_mediaPlayer->pause();
}

void QtPlayer::stop()
{
    // 播放和状态状态都可以停止播放
    if (m_mediaPlayer != nullptr && (m_mediaPlayer->playbackState() == QMediaPlayer::PlayingState
                                     || m_mediaPlayer->playbackState() == QMediaPlayer::PausedState)) {
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
    m_audioOutput->setMuted(mute);
}

void QtPlayer::onMediaStatusChanged(QMediaPlayer::MediaStatus status)
{
    // 过滤无效音乐文件
    if (status == QMediaPlayer::MediaStatus::EndOfMedia || status == QMediaPlayer::MediaStatus::InvalidMedia) {
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

    if (m_sinkInputPath.isEmpty()) return;

    QVariant volumeV = Utils::readDBusProperty("org.deepin.dde.Audio1", m_sinkInputPath,
                                               "org.deepin.dde.Audio1.SinkInput", "Volume");

    QVariant muteV = Utils::readDBusProperty("org.deepin.dde.Audio1", m_sinkInputPath,
                                             "org.deepin.dde.Audio1.SinkInput", "Mute");

    if (!volumeV.isValid() || !muteV.isValid()) return;

    QDBusInterface ainterface("org.deepin.dde.Audio1", m_sinkInputPath,
                              "org.deepin.dde.Audio1.SinkInput",
                              QDBusConnection::sessionBus());
    if (!ainterface.isValid()) return ;

    if (!qFuzzyCompare(volumeV.toDouble(), 1.0)) ainterface.call(QLatin1String("SetVolume"), 1.0, false);

    if (muteV.toBool()) ainterface.call(QLatin1String("SetMute"), false);
}

void QtPlayer::readSinkInputPath()
{
    QVariant v = Utils::readDBusProperty("org.deepin.dde.Audio1", "/org/deepin/dde/Audio1",
                                         "org.deepin.dde.Audio1", "SinkInputs");

    if (!v.isValid()) return;

    QList<QDBusObjectPath> allSinkInputsList = v.value<QList<QDBusObjectPath> >();

    for (auto curPath : allSinkInputsList) {
        QVariant nameV = Utils::readDBusProperty("org.deepin.dde.Audio1", curPath.path(),
                                                 "org.deepin.dde.Audio1.SinkInput", "Name");

        if (!nameV.isValid() || (nameV.toString() != "Deepin Music" && nameV.toString() != "deepin-music"))
            continue;

        m_sinkInputPath = curPath.path();
        break;
    }
}
