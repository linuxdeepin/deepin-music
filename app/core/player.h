/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef PLAYER_H
#define PLAYER_H

#include <QSharedPointer>
#include <QMediaPlayer>

class MusicInfo;
class Playlist;

class Player : public QMediaPlayer
{
    Q_OBJECT
public:

    enum PlayStatus {
        Invaild  = 0,
        Stop,
        Playing,
        Pause
    };
    Q_ENUM(PlayStatus)

    static Player *instance()
    {
        static auto s_app = new Player;
        return s_app;
    }

    void init(){}
signals:
    void progrossChanged(qint64 value, qint64 range);
public slots:
    void setMedia(const QString &mediaUrl);

private:
    explicit Player(QObject *parent = 0);

    qint64 m_duration = -1;
};

#endif // PLAYER_H
