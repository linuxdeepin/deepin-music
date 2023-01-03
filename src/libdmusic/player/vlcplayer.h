// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VLCPLAYER_H
#define VLCPLAYER_H
#include "playerbase.h"

#include "vlc/MediaPlayer.h"
#include "vlc/cda.h"
#include <QMediaPlayer>
#include <QTimer>
#include <QPropertyAnimation>

class VlcPlayer : public PlayerBase
{
    Q_OBJECT
public:
    VlcPlayer(QObject *parent = nullptr);
    ~VlcPlayer();

public:
    // 初始化
    void init() override;
    // 释放资源
    void release() override;
    void initCdaThread();
    void initCddaTrack() override;
    QList<MediaMeta> getCdaMetaInfo() override;

public:
    void play() override;
    void pause() override;
    DmGlobal::PlaybackStatus state() override;
    void stop() override;
    int length() override;
    void setTime(qint64 time) override;
    qint64 time() override;
    void setVolume(int volume) override;
    int getVolume() override;
    void setMute(bool value) override;
    bool getMute() override;
    void setMediaMeta(MediaMeta meta) override; //初始化打开vlcMedia
    void setFadeInOutFactor(double fadeInOutFactor) override;

    //Equalizer
    void setEqualizerEnabled(bool enabled) override;
    void loadFromPreset(uint index) override;
    void setPreamplification(float value) override;
    void setAmplificationForBandAt(float amp, uint bandIndex) override;
    float amplificationForBandAt(uint bandIndex) override;
    float preamplification() override;

protected:
    void startCdaThread();
    void setEqualizer(bool enabled, int curIndex, QList<int> indexbaud);

private:
    void releasePlayer();

private:
    VlcInstance *m_qvinstance = nullptr;
    VlcMedia *m_qvmedia = nullptr;
    VlcMediaPlayer *m_qvplayer = nullptr;
    CdaThread *m_pCdaThread = nullptr;
    int m_Vlcstate = -1; //休眠状态缓存(上一次休眠时的状态)
    int m_volume = 50.0;
};

#endif // VLCPLAYER_H
