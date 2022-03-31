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
    void init() override;
    // 释放资源
    void release() override;
    void initCdaThread();
    void initCddaTrack() override;
    QList<MediaMeta> getCdaMetaInfo() override;

public:
    void play() override;
    void pause() override;
    PlayerBase::PlayState state() override;
    void stop() override;
    int length() override;
    void setTime(qint64 time) override;
    qint64 time() override;
    void setVolume(int volume) override;
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
