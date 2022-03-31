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
    void init() Q_DECL_OVERRIDE;
    // 释放资源
    void release() Q_DECL_OVERRIDE;
    void initCdaThread();
    void initCddaTrack() Q_DECL_OVERRIDE;
    QList<MediaMeta> getCdaMetaInfo() Q_DECL_OVERRIDE;

public:
    void play() Q_DECL_OVERRIDE;
    void pause() Q_DECL_OVERRIDE;
    PlayerBase::PlayState state() Q_DECL_OVERRIDE;
    void stop() Q_DECL_OVERRIDE;
    int length() Q_DECL_OVERRIDE;
    void setTime(qint64 time) Q_DECL_OVERRIDE;
    qint64 time() Q_DECL_OVERRIDE;
    void setVolume(int volume) Q_DECL_OVERRIDE;
    void setMute(bool value) Q_DECL_OVERRIDE;
    bool getMute() Q_DECL_OVERRIDE;
    void setMediaMeta(MediaMeta meta) Q_DECL_OVERRIDE; //初始化打开vlcMedia
    void setFadeInOutFactor(double fadeInOutFactor) Q_DECL_OVERRIDE;

    //Equalizer
    void setEqualizerEnabled(bool enabled) Q_DECL_OVERRIDE;
    void loadFromPreset(uint index) Q_DECL_OVERRIDE;
    void setPreamplification(float value) Q_DECL_OVERRIDE;
    void setAmplificationForBandAt(float amp, uint bandIndex) Q_DECL_OVERRIDE;
    float amplificationForBandAt(uint bandIndex) Q_DECL_OVERRIDE;

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
