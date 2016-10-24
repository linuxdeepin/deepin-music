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

#include <QObject>
#include <QSharedPointer>

class MusicInfo;
class Playlist;

class player : public QObject
{
    Q_OBJECT
public:
    explicit player(QObject *parent = 0);

    enum PlayStatus {
        Invaild  = 0,
        Stop,
        Playing,
        Pause
    };
    Q_ENUM(PlayStatus)

    void playMusic(QSharedPointer<Playlist> currentPlaylist, const MusicInfo &info);
    PlayStatus status();

signals:
public slots:
};

#endif // PLAYER_H
