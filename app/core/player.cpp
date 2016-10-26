/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "player.h"

#include "playlist.h"

#include <QDebug>

void Player::setPlaylist(QSharedPointer<Playlist> playlist)
{

}

void Player::setMode(Player::PlayMode mode)
{
    m_mode = mode;
    if (m_mode == Shuffle) {
//        m_historyIDs.clear();
    }
}

void Player::setMedia(const MusicInfo &info)
{
    qDebug() << info.url;
    m_info = info;
    QMediaPlayer::setMedia(QUrl::fromLocalFile(info.url));
}

void Player::changeProgress(qint64 value, qint64 range)
{
    auto position = value * this->duration() / range;
    if (position < 0) {
        qCritical() << "invaild position:" << this->media().canonicalUrl() << position;
    }
    this->setPosition(position);
}

void Player::selectNext()
{
    if (!m_playlist) {
        return;
    }

    switch (m_mode) {
    case Order: {
        // next of list, stop next
        if (m_playlist->isLast(m_info)) {
            break;
        }
        setMedia(m_playlist->next(m_info));
        this->play();
        break;
    }
    case RepeatAll: {
        setMedia(m_playlist->next(m_info));
        this->play();
        break;
    }
    case RepeatSingle: {
        this->play();
        break;
    }
    case Shuffle: {
        break;
    }
    }
}

Player::Player(QObject *parent) : QMediaPlayer(parent)
{
    connect(this, &QMediaPlayer::durationChanged, this, [ = ](qint64 duration) {
        m_duration = duration;
    });
    connect(this, &QMediaPlayer::positionChanged, this, [ = ](qint64 position) {
        emit progrossChanged(position,  m_duration);
    });
    connect(this, &QMediaPlayer::stateChanged, this, [ = ](QMediaPlayer::State state) {
        qDebug() << state;
        switch (state) {
        case QMediaPlayer::StoppedState: {
            this->selectNext();
            break;
        }
        case QMediaPlayer::PlayingState:
        case QMediaPlayer::PausedState:
            break;
        }
    });
}
