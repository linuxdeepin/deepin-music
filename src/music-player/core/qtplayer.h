// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef QTPLAYER_H
#define QTPLAYER_H
#include "playerbase.h"
#include <QMediaPlayer>
#include <QTime>

class QtPlayer: public PlayerBase
{
    Q_OBJECT
public:
    QtPlayer(QObject *parent = nullptr);
    virtual ~QtPlayer();

public:
    // 初始化
    void init() override;
    // 释放资源
    void release() override;
    PlayerBase::PlayState state() override;
    void play() override;
    void pause() override;
    void pauseNew() override;
    void resume() override;
    void stop() override;
    int length() override; //播放总时长
    void setTime(qint64 time) override;
    qint64 time() override;
    void setVolume(int volume) override;
    int getVolume();
    void setMute(bool value) override;
    void setMediaMeta(MediaMeta meta) override;
    bool getMute() override;
    void setFadeInOutFactor(double fadeInOutFactor) override;

protected slots:
    void onMediaStatusChanged(QMediaPlayer::MediaStatus status);
    void onPositionChanged(qint64 position);

private:
    bool setDbusMute(bool value);
    bool isDbusMuted();
    void resetPlayInfo();
    void setSinkInputAlmostMute();
    void readSinkInputPath();
    void releasePlayer();

private:
    QMediaPlayer *m_mediaPlayer = nullptr;
    qint64 m_currPositionChanged = 0;
    QString m_sinkInputPath;
    int m_correctVolume = 0; // 记录正确的音量值
    bool m_isTempMuted = false;
};

#endif // QTPLAYER_H
