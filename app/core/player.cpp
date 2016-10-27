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
    MusicMeta nextInfo = info;

    m_playlist = playlist;
    if (info.hash.isEmpty() && this->state() == QMediaPlayer::State::StoppedState) {
        nextInfo = m_playlist->first();
    }

    this->blockSignals(true);
    setMedia(nextInfo);
    this->blockSignals(false);
    emit musicPlayed(playlist, nextInfo);

    if (!playlist->history().contains(nextInfo.hash)) {
        // TODO: max
        playlist->history().append(nextInfo.hash);
    }
    this->play();
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

//    if (playlist->history().isEmpty()) {
//        //rebuild history
//        playlist->buildHistory(info.id);
//    }
//    auto nextId = playlist->history().last();
//    MusicInfo nextInfo = playlist->music(nextId);
//    if (nextInfo.id.isEmpty()) {
//        nextInfo = playlist->prev(info);
//    }
//    playMusic(playlist, nextInfo);
}

void Player::setMedia(const MusicMeta &info)
{
    m_info = info;
    QMediaPlayer::setMedia(QUrl::fromLocalFile(info.localpath));
    // TODO:
    if (!m_historyIDs.contains(info.hash)) {
        if (m_historyIDs.length() >= 100) {
            m_historyIDs.pop_front();
        }
        m_historyIDs << info.hash;
    }
}

void Player::changeProgress(qint64 value, qint64 range)
{
    auto position = value * this->duration() / range;
    if (position < 0) {
        qCritical() << "invaild position:" << this->media().canonicalUrl() << position;
    }
    this->setPosition(position);
}

void Player::selectNext(const MusicMeta &info, PlayMode mode)
{
    qDebug() << "next" << m_playlist << m_mode;
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
    qDebug() << "prev" << m_playlist << m_mode;
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
        emit progrossChanged(position,  m_duration);
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
