// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef QTPLAYER_H
#define QTPLAYER_H
#include "playerbase.h"

#include <QMediaPlayer>
#include <QElapsedTimer>
#include <QTimer>

class QThread;

class QtPlayer: public PlayerBase
{
    Q_OBJECT
public:
    QtPlayer(QObject *parent = nullptr, bool interpolateProgress = false);
    virtual ~QtPlayer();

public:
    // 初始化
    void init() override;
    // 释放资源
    void release() override;
    DmGlobal::PlaybackStatus state() override;
    void play() override;
    void pause() override;
    void stop() override;
    int length() override; //播放总时长
    void setTime(qint64 time) override;
    qint64 time() override;
    void setVolume(int volume) override;
    int getVolume() override;
    void setMute(bool mute) override;
    void setMediaMeta(MediaMeta meta) override;
    bool getMute() override;
    void setFadeInOutFactor(double fadeInOutFactor) override;

protected slots:
    void onMediaStatusChanged(QMediaPlayer::MediaStatus status);
    void onPositionChanged(qint64 position);

private:
    void resetPlayInfo();
    void readSinkInputPath();
    void releasePlayer();
    void updateProgressTimer();
    qint64 estimatedPosition() const;
    void publishPosition(qint64 position);
    void seekTo(qint64 time);
    void prepareApeSeekFile();
    void queuePendingApeSeek();
    void failApeSeek();
    void cancelApeSeek();

private:
    QMediaPlayer *m_mediaPlayer = nullptr;
    QAudioOutput *m_audioOutput = nullptr;
    qint64 m_currPositionChanged = 0;
    const bool m_interpolateProgress;
    QTimer m_progressTimer;
    QElapsedTimer m_positionClock;
    qint64 m_positionAnchor = 0;
    qint64 m_lastPublishedPosition = 0;
    QString m_sinkInputPath;
    QThread *m_apeDecodeThread = nullptr;
    QString m_apeSeekFile;
    qint64 m_pendingApeSeek = -1;
    QMediaPlayer::PlaybackState m_apeRequestedState = QMediaPlayer::StoppedState;
    bool m_apeSeekEndPending = false;
    bool m_apeSeekFileFailed = false;
};

#endif // QTPLAYER_H
