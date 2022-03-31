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
    void stop() override;
    int length() override; //播放总时长
    void setTime(qint64 time) override;
    qint64 time() override;
    void setVolume(int volume) override;
    void setMute(bool value) override;
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

private:
    QMediaPlayer *m_mediaPlayer = nullptr;
    qint64 m_currPositionChanged = 0;
    QString m_sinkInputPath;
};

#endif // QTPLAYER_H
