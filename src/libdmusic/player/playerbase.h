/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     Zhang Wenchao <zhangwenchao@uniontech.com>
 *
 * Maintainer: WangYu <wangyu@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef PLAYERBASE_H
#define PLAYERBASE_H

#include "global.h"
#include <QObject>

using namespace DMusic;

class PlayerBase : public QObject
{
    Q_OBJECT
public:
    PlayerBase(QObject *parent = nullptr);

    // 初始化
    virtual void init() = 0;
    // 释放资源
    virtual void release() = 0;

public:
    MediaMeta getMediaMeta() {return m_activeMeta;}
    QStringList supportedSuffixList()const {return m_supportedSuffix;}
    virtual void initCddaTrack() {}
    virtual QList<MediaMeta> getCdaMetaInfo() {return QList<MediaMeta>();}

    virtual DmGlobal::PlaybackStatus state() = 0;
    virtual void play() = 0;
    virtual void pause() = 0;
    virtual void stop() = 0;
    virtual int length() = 0;
    virtual void setTime(qint64 time) = 0;
    virtual qint64 time() = 0;
    virtual void setVolume(int volume) = 0;
    virtual int getVolume() = 0;
    virtual void setMute(bool mute) = 0;
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
    void metaChanged();
    void timeChanged(qint64 position);  //播放时长改变
    void positionChanged(float position); // 播放进度改变
    void stateChanged(DmGlobal::PlaybackStatus state); // 状态类型为int 方便统一
    void end();
    void sigSendCdaStatus(int state);
    void signalMutedChanged();

protected:
    MediaMeta            m_activeMeta;
    QStringList          m_supportedSuffix;
    int                  m_volume = 50;
};

#endif // PLAYERBASE_H
