// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "qtplayer.h"
#include "core/musicsettings.h"
#include "util/dbusutils.h"
#include "util/global.h"

#include <QDBusObjectPath>
#include <QDBusInterface>
#include <QTimer>

QtPlayer::QtPlayer(QObject *parent)
    : PlayerBase(parent)
{

}

QtPlayer::~QtPlayer()
{
    releasePlayer();
}

void QtPlayer::init()
{
    //防止多次创建
    if (m_mediaPlayer == nullptr) {
        qputenv("QT_GSTREAMER_USE_PLAYBIN_VOLUME", "1");
        m_mediaPlayer = new QMediaPlayer(this);
        connect(m_mediaPlayer, &QMediaPlayer::mediaStatusChanged, this, &QtPlayer::onMediaStatusChanged);
        connect(m_mediaPlayer, &QMediaPlayer::positionChanged, this, &QtPlayer::onPositionChanged);
        connect(m_mediaPlayer, &QMediaPlayer::stateChanged, this, [ = ](QMediaPlayer::State newState) {
            PlayerBase::PlayState state = PlayerBase::Stopped;
            switch (newState) {
            case QMediaPlayer::PlayingState: {
                state = PlayerBase::Playing;
            }
            break;
            case QMediaPlayer::PausedState: {
                state = PlayerBase::Paused;
            }
            break;
            default:
                state = PlayerBase::Stopped;
                break;
            }
            emit stateChanged(state);
        });
        connect(m_mediaPlayer, &QMediaPlayer::mutedChanged, this, &QtPlayer::signalMutedChanged);
    }
}


void QtPlayer::releasePlayer()
{
    if (m_mediaPlayer) {
        delete m_mediaPlayer;
        m_mediaPlayer = nullptr;
    }
}


void QtPlayer::release()
{
    releasePlayer();
}


PlayerBase::PlayState QtPlayer::state()
{
    init();
    int state = m_mediaPlayer->state();
    switch (state) {
    case QMediaPlayer::PlayingState: {
        return PlayerBase::Playing;
    }
    case QMediaPlayer::PausedState: {
        return PlayerBase::Paused;
    }
    default:
        return PlayerBase::Stopped;
    }
}

void QtPlayer::play()
{
    init();
    m_mediaPlayer->play();
}

void QtPlayer::pause()
{
    if (m_mediaPlayer != nullptr && m_mediaPlayer->state() == QMediaPlayer::PlayingState)
        m_mediaPlayer->pause();
}

void QtPlayer::pauseNew()
{

}

void QtPlayer::resume()
{

}

void QtPlayer::stop()
{
    // 播放和状态状态都可以停止播放
    if (m_mediaPlayer != nullptr && (m_mediaPlayer->state() == QMediaPlayer::State::PlayingState
                                     || m_mediaPlayer->state() == QMediaPlayer::State::PausedState)) {
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
    bool value = m_activeMeta.mmType == MIMETYPE_CDA ? false : true;
    if (value) {
        m_mediaPlayer->setMedia(QUrl::fromLocalFile(m_activeMeta.localPath));
    } else {
        m_mediaPlayer->setMedia(QUrl::fromUserInput(m_activeMeta.localPath));
    }
    m_mediaPlayer->setVolume(MusicSettings::value("base.play.volume").toInt());
}

bool QtPlayer::getMute()
{
    init();
    return m_mediaPlayer->isMuted();
    //setDbusMute函数会触发“滴”声，影响用户体验
    //return isDbusMuted();
}

void QtPlayer::setFadeInOutFactor(double fadeInOutFactor)
{
    init();
    m_mediaPlayer->setVolume(static_cast<int>(10 * fadeInOutFactor));
}

void QtPlayer::setVolume(int volume)
{
    init();
    m_mediaPlayer->setVolume(volume);
}

int QtPlayer::getVolume()
{
    return m_mediaPlayer->volume();
}

void QtPlayer::setMute(bool value)
{
    init();
    m_mediaPlayer->setMuted(value);
    //setDbusMute函数会触发“滴”声，影响用户体验
    //setDbusMute(value);
}

void QtPlayer::onMediaStatusChanged(QMediaPlayer::MediaStatus status)
{
    // 过滤无效音乐文件
    if (status == QMediaPlayer::MediaStatus::EndOfMedia || status == QMediaPlayer::MediaStatus::InvalidMedia) {
        QTimer::singleShot(300, this, [ = ]() {
            emit end();
        });
    }
}

void QtPlayer::onPositionChanged(qint64 position)
{
    init();
    // 停止播放时，不设置进度
    if (m_mediaPlayer->duration() <= 0 || m_mediaPlayer->state() != QMediaPlayer::State::PlayingState)
        return;
    m_currPositionChanged = position;
    float value = static_cast<float>(position) / m_mediaPlayer->duration();
//    qDebug() << "position" << position << "value" << value;
    emit timeChanged(position);
    emit positionChanged(value);
    resetPlayInfo();
}

bool QtPlayer::setDbusMute(bool value)
{
    readSinkInputPath();
    if (!m_sinkInputPath.isEmpty()) {
        QDBusInterface ainterface("com.deepin.daemon.Audio", m_sinkInputPath,
                                  "com.deepin.daemon.Audio.SinkInput",
                                  QDBusConnection::sessionBus());
        if (!ainterface.isValid()) {
            return false;
        }
        if (isDbusMuted() != value)
            ainterface.call(QLatin1String("SetMute"), value);
        return true;
    }
    return false;
}

bool QtPlayer::isDbusMuted()
{
    readSinkInputPath();
    if (!m_sinkInputPath.isEmpty()) {
        QVariant MuteV = DBusUtils::readDBusProperty("com.deepin.daemon.Audio", m_sinkInputPath,
                                                     "com.deepin.daemon.Audio.SinkInput", "Mute");

        if (!MuteV.isValid()) {
            return false;
        }
        return MuteV.toBool();
    }
    return false;
}

void QtPlayer::resetPlayInfo()
{
    // 防止重复
    if (!m_sinkInputPath.isEmpty()) return;
    readSinkInputPath();

    if (m_sinkInputPath.isEmpty()) return;
    QVariant volumeV = DBusUtils::readDBusProperty("com.deepin.daemon.Audio", m_sinkInputPath,
                                                   "com.deepin.daemon.Audio.SinkInput", "Volume");

    QVariant muteV = DBusUtils::readDBusProperty("com.deepin.daemon.Audio", m_sinkInputPath,
                                                 "com.deepin.daemon.Audio.SinkInput", "Mute");

    if (!volumeV.isValid() || !muteV.isValid()) return;

    QDBusInterface ainterface("com.deepin.daemon.Audio", m_sinkInputPath,
                              "com.deepin.daemon.Audio.SinkInput",
                              QDBusConnection::sessionBus());
    if (!ainterface.isValid()) return ;

    if (!qFuzzyCompare(volumeV.toDouble(), 1.0)) ainterface.call(QLatin1String("SetVolume"), 1.0, false);

    if (muteV.toBool()) ainterface.call(QLatin1String("SetMute"), false);
}

void QtPlayer::readSinkInputPath()
{
    QVariant v = DBusUtils::readDBusProperty("com.deepin.daemon.Audio", "/com/deepin/daemon/Audio",
                                             "com.deepin.daemon.Audio", "SinkInputs");

    if (!v.isValid()) return;

    QList<QDBusObjectPath> allSinkInputsList = v.value<QList<QDBusObjectPath> >();

    for (auto curPath : allSinkInputsList) {
        QVariant nameV = DBusUtils::readDBusProperty("com.deepin.daemon.Audio", curPath.path(),
                                                     "com.deepin.daemon.Audio.SinkInput", "Name");

        if (!nameV.isValid() || nameV.toString() != "Deepin Music")
            continue;

        m_sinkInputPath = curPath.path();
        break;
    }
}
