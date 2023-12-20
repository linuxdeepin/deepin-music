// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PLAYERBASE_H
#define PLAYERBASE_H

#include <mediameta.h>
#include <QObject>

typedef enum PlayerTypeTag {
    VLC  = 0,
    QtMEDIAPLAYER
} PLAYERTYPETAG;

class PlayerBase : public QObject
{
    Q_OBJECT
public:
    enum PlayState {
        Idle = 0,
        Opening,
        Buffering,
        Playing,
        Paused,
        Stopped,
        Ended,
        Error
    };

    PlayerBase(QObject *parent = nullptr);

    // 初始化
    virtual void init() = 0;
    // 释放资源
    virtual void release() = 0;

public:
    MediaMeta getMediaMeta() {return m_activeMeta;}
    virtual void initCddaTrack() {}
    virtual QList<MediaMeta> getCdaMetaInfo() {return QList<MediaMeta>();}

    virtual PlayState state() = 0;
    virtual void play() = 0;
    virtual void pause() = 0;
    virtual void pauseNew() = 0;
    virtual void resume() = 0;
    virtual void stop() = 0;
    virtual int length() = 0;
    virtual void setTime(qint64 time) = 0;
    virtual qint64 time() = 0;
    virtual void setVolume(int volume) = 0;
    virtual void setMute(bool value) = 0;
    virtual bool getMute() = 0;
    virtual void setMediaMeta(MediaMeta meta) = 0; //初始化打开vlcMedia
    virtual void setFadeInOutFactor(double fadeInOutFactor) = 0;

    //Equalizer
    virtual void setEqualizerEnabled(bool enabled) {Q_UNUSED(enabled);}
    virtual void loadFromPreset(uint index) {Q_UNUSED(index);}
    virtual void setPreamplification(float value) {Q_UNUSED(value);}
    virtual void setAmplificationForBandAt(float amp, uint bandIndex) {Q_UNUSED(amp); Q_UNUSED(bandIndex);}
    virtual float amplificationForBandAt(uint bandIndex) {Q_UNUSED(bandIndex); return 1.0;}
    virtual float preamplification() {return 1.0;}

signals:
    void timeChanged(qint64 position);  //播放时长改变
    void positionChanged(float position); // 播放进度改变
    void stateChanged(PlayState state); // 状态类型为int 方便统一
    void end();
    void sigSendCdaStatus(int state);
    void signalMutedChanged();

protected:
    static int INT_LAST_PROGRESS_FLAG; //1标识当前歌曲有上一次的播放进度，0标识当前歌曲没有上一次的播放记录

    MediaMeta m_activeMeta;
};

#endif // PLAYERBASE_H
