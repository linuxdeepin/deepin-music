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
    void init() Q_DECL_OVERRIDE;
    // 释放资源
    void release() Q_DECL_OVERRIDE;
    PlayerBase::PlayState state() Q_DECL_OVERRIDE;
    void play() Q_DECL_OVERRIDE;
    void pause() Q_DECL_OVERRIDE;
    void stop() Q_DECL_OVERRIDE;
    int length() Q_DECL_OVERRIDE; //播放总时长
    void setTime(qint64 time) Q_DECL_OVERRIDE;
    qint64 time() Q_DECL_OVERRIDE;
    void setVolume(int volume) Q_DECL_OVERRIDE;
    void setMute(bool value) Q_DECL_OVERRIDE;
    void setMediaMeta(MediaMeta meta) Q_DECL_OVERRIDE;
    bool getMute() Q_DECL_OVERRIDE;
    void setFadeInOutFactor(double fadeInOutFactor) Q_DECL_OVERRIDE;

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
