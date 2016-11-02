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
}

void Player::playMusic(QSharedPointer<Playlist> playlist, const MusicMeta &info)
{
    qDebug() << "play" << info.localpath;
    MusicMeta nextInfo = info;

    m_playlist = playlist;
    if (info.hash.isEmpty() && this->state() == QMediaPlayer::State::StoppedState) {
        nextInfo = m_playlist->first();
    }

    this->blockSignals(true);
    setMedia(nextInfo);
    this->blockSignals(false);
    emit musicPlayed(playlist, nextInfo);

    this->play();

    this->setPosition(nextInfo.offset);
    qDebug() << nextInfo.offset << nextInfo.offset / 1000
             << this->duration();
}

void Player::playNextMusic(QSharedPointer<Playlist> playlist, const MusicMeta &info)
{
    Q_ASSERT(playlist == m_playlist);

    if (m_mode == RepeatSingle) {
        selectNext(info, RepeatAll);
    } else {
        selectNext(info, m_mode);
    }
}

void Player::playPrevMusic(QSharedPointer<Playlist> playlist, const MusicMeta &info)
{
    Q_ASSERT(playlist == m_playlist);

    if (m_mode == RepeatSingle) {
        selectPrev(info, RepeatAll);
    } else {
        selectPrev(info, m_mode);
    }
}

void Player::setMedia(const MusicMeta &info)
{
    m_info = info;
    if (this->media().canonicalUrl() != QUrl::fromLocalFile(info.localpath)) {
        QMediaPlayer::setMedia(QUrl::fromLocalFile(info.localpath));
    }
}

void Player::changeProgress(qint64 value, qint64 range)
{
    Q_ASSERT(value <= range);
    Q_ASSERT(m_info.offset + m_info.length <= QMediaPlayer::duration());

    auto position = value * m_info.length / range + m_info.offset;
    if (position < 0) {
        qCritical() << "invaild position:" << this->media().canonicalUrl() << position;
        return;
    }
    this->setPosition(position);
}

void Player::selectNext(const MusicMeta &info, PlayMode mode)
{
    if (!m_playlist) {
        return;
    }

    switch (mode) {
    case RepeatAll: {
        playMusic(m_playlist, m_playlist->next(info));
        break;
    }
    case RepeatSingle: {
        playMusic(m_playlist, info);
        break;
    }
    case Shuffle: {
        int randomValue = qrand() % m_playlist->length();
        playMusic(m_playlist, m_playlist->music(randomValue));
        break;
    }
    }
}

void Player::selectPrev(const MusicMeta &info, Player::PlayMode mode)
{
    if (!m_playlist) {
        return;
    }

    switch (mode) {
    case RepeatAll: {
        playMusic(m_playlist, m_playlist->prev(info));
        break;
    }
    case RepeatSingle: {
        playMusic(m_playlist, info);
        break;
    }
    case Shuffle: {
        int randomValue = qrand() % m_playlist->length();
        playMusic(m_playlist, m_playlist->music(randomValue));
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
        if (0 == m_info.length) {
            return;
        }

        if (m_info.offset > position) {
            return;
        }

        if (position > m_info.offset + m_info.length) {
            // TODO: to next
            qDebug() << "auto change next music";
            this->selectNext(m_info, m_mode);
        }
        emit progrossChanged(position - m_info.offset,  m_info.length);
    });
    connect(this, &QMediaPlayer::stateChanged, this, [ = ](QMediaPlayer::State state) {
        switch (state) {
        case QMediaPlayer::StoppedState: {
            qDebug() << "auto change next music";
            this->selectNext(m_info, m_mode);
            break;
        }
        case QMediaPlayer::PlayingState:
        case QMediaPlayer::PausedState:
            break;
        }
    });
}
