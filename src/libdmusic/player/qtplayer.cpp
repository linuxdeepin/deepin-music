// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "qtplayer.h"
#include "apeseekfile.h"
#include "utils.h"
#include "util/log.h"

#include <QAudioOutput>
#include <QDBusObjectPath>
#include <QDBusInterface>
#include <QDir>
#include <QDateTime>
#include <QFileInfo>
#include <QStandardPaths>
#include <QThread>
#include <memory>
#include <utility>

static QString apeSeekCacheFile(const QFileInfo &source)
{
    if (!source.isFile() || !source.lastModified().isValid()) {
        return {};
    }
    QString cachePath = DmGlobal::cachePath();
    if (cachePath.isEmpty()) {
        cachePath = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    }
    if (cachePath.isEmpty()) {
        return {};
    }
    const auto name = QStringLiteral("seekfile/%1-%2-%3.wav")
                          .arg(Utils::filePathHash(source.absoluteFilePath()))
                          .arg(source.size())
                          .arg(source.lastModified().toMSecsSinceEpoch());
    return QDir(cachePath).absoluteFilePath(name);
}

QtPlayer::QtPlayer(QObject *parent, bool interpolateProgress)
    : PlayerBase(parent)
    , m_interpolateProgress(interpolateProgress)
{
    qCDebug(dmMusic) << "QtPlayer constructor";
    m_supportedSuffix << "wav"
                      << "ogg"
                      << "mp3"
                      << "flac"
                      << "opus";
    if (m_interpolateProgress) {
        m_progressTimer.setInterval(100);
        connect(&m_progressTimer, &QTimer::timeout, this, [this]() {
            if (m_pendingApeSeek >= 0) {
                return;
            }
            const qint64 position = estimatedPosition();
            if (position > m_lastPublishedPosition) {
                publishPosition(position);
            }
        });
    }
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
    if (m_interpolateProgress) {
        connect(m_mediaPlayer, &QMediaPlayer::durationChanged, this, &QtPlayer::updateProgressTimer);
        connect(m_mediaPlayer, &QMediaPlayer::seekableChanged, this, &QtPlayer::queuePendingApeSeek);
    }
    connect(m_mediaPlayer, &QMediaPlayer::playbackStateChanged, this, [this](QMediaPlayer::PlaybackState newState) {
        updateProgressTimer();
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
    cancelApeSeek();
    m_apeRequestedState = QMediaPlayer::StoppedState;
    m_progressTimer.stop();
    m_positionClock.invalidate();
    m_positionAnchor = 0;
    m_lastPublishedPosition = 0;
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
    m_apeSeekFile.clear();
    m_apeSeekFileFailed = false;
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
    if (m_interpolateProgress) {
        m_apeRequestedState = QMediaPlayer::PlayingState;
        if (m_pendingApeSeek >= 0 && (!m_apeSeekFile.isEmpty() || m_apeSeekEndPending)) {
            return;
        }
    }
    qCDebug(dmMusic) << "Starting playback";
    m_mediaPlayer->play();
}

void QtPlayer::pause()
{
    qCDebug(dmMusic) << "Pause requested";
    if (m_interpolateProgress) {
        m_apeRequestedState = QMediaPlayer::PausedState;
    }
    if (m_mediaPlayer != nullptr && (m_mediaPlayer->playbackState() == QMediaPlayer::PlayingState
                                     || (m_interpolateProgress && m_mediaPlayer->mediaStatus() == QMediaPlayer::LoadingMedia))) {
        qCDebug(dmMusic) << "Pausing playback";
        m_mediaPlayer->pause();
    }
}

void QtPlayer::stop()
{
    qCDebug(dmMusic) << "Stop requested";
    cancelApeSeek();
    m_apeRequestedState = QMediaPlayer::StoppedState;
    // 播放和状态状态都可以停止播放
    if (m_mediaPlayer != nullptr && (m_mediaPlayer->playbackState() == QMediaPlayer::PlayingState
                                     || m_mediaPlayer->playbackState() == QMediaPlayer::PausedState
                                     || (m_interpolateProgress && m_mediaPlayer->mediaStatus() == QMediaPlayer::LoadingMedia))) {
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
    const bool waitingForCache = !m_apeSeekFile.isEmpty() && m_mediaPlayer->mediaStatus() == QMediaPlayer::LoadingMedia;
    if (m_interpolateProgress && !m_activeMeta.localPath.isEmpty()
        && (m_pendingApeSeek >= 0 || waitingForCache || (m_apeSeekFile.isEmpty() && time > 0))) {
        if (m_apeSeekFileFailed) {
            return;
        }
        const qint64 duration = m_mediaPlayer->duration();
        const qint64 target = duration > 0 ? qBound(qint64(0), time, duration) : qMax(qint64(0), time);
        m_pendingApeSeek = target;
        // The waveform still treats the mouse as pressed inside its release handler.
        QMetaObject::invokeMethod(this, [this, target] {
            if (m_pendingApeSeek == target) {
                publishPosition(target);
            }
        }, Qt::QueuedConnection);
        if (m_apeSeekFile.isEmpty() && !m_apeDecodeThread) {
            prepareApeSeekFile();
        }
        return;
    }
    seekTo(time);
}

void QtPlayer::seekTo(qint64 time)
{
    qCDebug(dmMusic) << "Setting playback position to:" << time;
    m_mediaPlayer->setPosition(time);
    if (m_interpolateProgress && m_mediaPlayer->isSeekable()) {
        m_positionAnchor = m_mediaPlayer->position();
        if (m_positionClock.isValid()) {
            m_positionClock.restart();
        }
        publishPosition(m_positionAnchor);
    }
}

void QtPlayer::prepareApeSeekFile()
{
    // Qt's FFmpeg renderer can wait several seconds for a timestamp after seeking in APE.
    // Decode sequentially on the first seek and reuse the seekable PCM source for this track.
    const QFileInfo source(m_activeMeta.localPath);
    const auto path = apeSeekCacheFile(source);
    if (path.isEmpty() || !QDir().mkpath(QFileInfo(path).absolutePath())) {
        failApeSeek();
        return;
    }
    auto decoded = std::make_shared<bool>(false);
    auto *thread = QThread::create([source, path, decoded] {
        *decoded = isApeSeekFileReady(path) || writeApeSeekFile(source, path);
    });
    thread->setParent(this);
    m_apeDecodeThread = thread;
    connect(thread, &QThread::finished, this, [this, thread, path, decoded] {
        if (m_apeDecodeThread != thread) {
            return;
        }
        m_apeDecodeThread = nullptr;
        if (!*decoded) {
            failApeSeek();
            return;
        }
        m_apeSeekFile = path;
        m_sinkInputPath.clear();
        m_mediaPlayer->setSource(QUrl::fromLocalFile(path));
    });
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    thread->start();
}

void QtPlayer::failApeSeek()
{
    qCWarning(dmMusic) << "Could not prepare APE seek audio for" << m_activeMeta.localPath;
    m_apeSeekFileFailed = true;
    const bool ended = m_apeSeekEndPending;
    cancelApeSeek();
    if (ended) {
        m_apeRequestedState = QMediaPlayer::StoppedState;
        emit end();
    } else {
        // Keep the current source playing and restore its position after a failed seek.
        publishPosition(estimatedPosition());
    }
}

void QtPlayer::cancelApeSeek()
{
    if (m_apeDecodeThread) {
        m_apeDecodeThread->requestInterruption();
        m_apeDecodeThread = nullptr;
    }
    m_pendingApeSeek = -1;
    m_apeSeekEndPending = false;
}

qint64 QtPlayer::time()
{
    qCDebug(dmMusic) << "Get current playback position";
    if (m_pendingApeSeek >= 0) {
        return m_pendingApeSeek;
    }
    init();
    if (m_mediaPlayer->mediaStatus() == QMediaPlayer::NoMedia) {
        qCDebug(dmMusic) << "No media loaded, returning invalid time";
        return -1;
    }
    qint64 pos = m_interpolateProgress ? estimatedPosition() : m_mediaPlayer->position();
    qCDebug(dmMusic) << "Current playback position:" << pos;
    return pos;
}

void QtPlayer::setMediaMeta(MediaMeta meta)
{
    qCDebug(dmMusic) << "Set media meta requested";
    init();
    QString cachedFile;
    if (m_interpolateProgress && !meta.localPath.isEmpty()) {
        const auto path = apeSeekCacheFile(QFileInfo(meta.localPath));
        if (isApeSeekFileReady(path)) {
            cachedFile = path;
        }
    }
    const auto source = QUrl::fromLocalFile(cachedFile.isEmpty() ? meta.localPath : cachedFile);
    if (m_activeMeta.hash == meta.hash
        && (!m_interpolateProgress || (m_activeMeta.localPath == meta.localPath
                                       && m_mediaPlayer->source() == source))) return;

    if (m_interpolateProgress) {
        cancelApeSeek();
        m_apeRequestedState = QMediaPlayer::StoppedState;
        m_progressTimer.stop();
        m_positionClock.invalidate();
        m_positionAnchor = 0;
        m_lastPublishedPosition = 0;
    }
    qCInfo(dmMusic) << "Setting media meta - Title:" << meta.title << "Path:" << meta.localPath;
    m_activeMeta = meta;
    m_apeSeekFile = cachedFile;
    m_apeSeekFileFailed = false;
    m_mediaPlayer->setSource(source);
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
    m_audioOutput->setVolume(qBound(0.0, static_cast<qreal>(m_volume) / 100.0 * fadeInOutFactor, 1.0));
}

void QtPlayer::setVolume(int volume)
{
    qCDebug(dmMusic) << "Set volume requested:" << volume;
    init();
    qCDebug(dmMusic) << "Setting volume to:" << volume;
    m_volume = volume;
    m_audioOutput->setVolume(volume / 100.0);
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
    updateProgressTimer();
    queuePendingApeSeek();
    if (status == QMediaPlayer::MediaStatus::EndOfMedia) {
        if (m_pendingApeSeek >= 0 && m_apeSeekFile.isEmpty()) {
            // The old source can reach EOF while the seek file is still being prepared.
            m_apeSeekEndPending = true;
            return;
        }
        m_apeRequestedState = QMediaPlayer::StoppedState;
        qCInfo(dmMusic) << "Media playback reached end";
        emit end();
    } else if (status == QMediaPlayer::MediaStatus::InvalidMedia) {
        cancelApeSeek();
        m_apeRequestedState = QMediaPlayer::StoppedState;
        qCWarning(dmMusic) << "Invalid media detected for:" << m_activeMeta.localPath;
        emit end();
    }
}

void QtPlayer::queuePendingApeSeek()
{
    if (m_apeSeekFile.isEmpty() || m_pendingApeSeek < 0) {
        return;
    }
    // Qt can emit seekableChanged before it finishes setting the media status.
    QMetaObject::invokeMethod(this, [this, source = m_apeSeekFile] {
        if (m_apeSeekFile != source || m_pendingApeSeek < 0 || !m_mediaPlayer->isSeekable()) {
            return;
        }
        const auto status = m_mediaPlayer->mediaStatus();
        if (status != QMediaPlayer::LoadedMedia && status != QMediaPlayer::BufferingMedia
            && status != QMediaPlayer::BufferedMedia) {
            return;
        }
        const auto position = std::exchange(m_pendingApeSeek, -1);
        m_apeSeekEndPending = false;
        const auto state = m_apeRequestedState;
        seekTo(position);
        if (m_apeSeekFile != source) {
            return;
        }
        if (state == QMediaPlayer::PlayingState) {
            m_mediaPlayer->play();
        } else if (state == QMediaPlayer::PausedState) {
            m_mediaPlayer->pause();
        }
    }, Qt::QueuedConnection);
}

void QtPlayer::onPositionChanged(qint64 position)
{
    qCDebug(dmMusic) << "Position changed to:" << position;
    init();
    if (m_interpolateProgress) {
        m_positionAnchor = position;
        if (m_positionClock.isValid()) {
            m_positionClock.restart();
        } else if (m_mediaPlayer->playbackState() != QMediaPlayer::PlayingState) {
            m_lastPublishedPosition = position;
        }
        if (m_mediaPlayer->playbackState() == QMediaPlayer::PlayingState && m_mediaPlayer->duration() > 0) {
            resetPlayInfo();
        }
        return;
    }
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

void QtPlayer::updateProgressTimer()
{
    if (!m_interpolateProgress || !m_mediaPlayer) return;

    const auto status = m_mediaPlayer->mediaStatus();
    const auto state = m_mediaPlayer->playbackState();
    // The FFmpeg backend can report LoadedMedia while playing after a seek.
    const bool ready = status == QMediaPlayer::LoadedMedia
                       || status == QMediaPlayer::BufferingMedia
                       || status == QMediaPlayer::BufferedMedia;
    const bool canAdvance = state == QMediaPlayer::PlayingState && m_mediaPlayer->duration() > 0 && ready;
    if (canAdvance) {
        // APE frames can span several seconds. Fill in the progress between backend updates.
        if (!m_progressTimer.isActive()) {
            m_positionClock.start();
            m_progressTimer.start();
        }
    } else {
        m_positionAnchor = state == QMediaPlayer::StoppedState
                           ? m_mediaPlayer->position() : estimatedPosition();
        m_positionClock.invalidate();
        m_progressTimer.stop();
        if (state == QMediaPlayer::StoppedState) {
            m_lastPublishedPosition = m_positionAnchor;
        }
    }
}

qint64 QtPlayer::estimatedPosition() const
{
    const qint64 elapsed = m_positionClock.isValid() ? m_positionClock.elapsed() : 0;
    return qBound(qint64(0), m_positionAnchor + elapsed, m_mediaPlayer->duration());
}

void QtPlayer::publishPosition(qint64 position)
{
    const qint64 duration = m_mediaPlayer->duration();
    if (duration <= 0 && m_pendingApeSeek < 0) return;
    m_lastPublishedPosition = position;
    // PlayerEngine already republishes the absolute time. A second event based on
    // metadata duration can overwrite it with a different position.
    emit timeChanged(position);
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
