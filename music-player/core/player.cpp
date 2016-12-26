/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "player.h"

#include <QDebug>
#include <QTimer>
#include <QMediaPlayer>

#include <QMimeDatabase>

static QMap<QString, bool>  sSupportedSuffix;
static QStringList          sSupportedSuffixList;
static QStringList          sSupportedFiterList;
static QStringList          sSupportedMimeTypes;

void initMiniTypes()
{
    //black list
    QHash<QString, bool> suffixBlacklist;
    suffixBlacklist.insert("m3u", true);

    QHash<QString, bool> suffixWhitelist;
    suffixWhitelist.insert("cue", true);

    QMimeDatabase mdb;
    for (auto &mt : mdb.allMimeTypes()) {
        if (mt.name().startsWith("audio/")) {
            sSupportedFiterList << mt.filterString();
            for (auto &suffix : mt.suffixes()) {
                if (suffixBlacklist.contains(suffix)) {
                    continue;
                }

                sSupportedSuffixList << "*." + suffix;
                sSupportedSuffix.insert(suffix, true);
            }
            sSupportedMimeTypes << mt.name();
        }
    }

    for (auto &suffix : suffixWhitelist.keys()) {
        sSupportedSuffixList << "*." + suffix;
        sSupportedSuffix.insert(suffix, true);
    }
}

QStringList Player::supportedFilterStringList() const
{
    return sSupportedFiterList;
}

QStringList Player::supportedMimeTypes() const
{
    return sSupportedMimeTypes;
}

class PlayerPrivate
{
public:
    PlayerPrivate(Player *parent) : q_ptr(parent)
    {
        qplayer = new QMediaPlayer;
        initMiniTypes();
    }

    void initConnection();

    // player property
    bool canControl     = true;
    bool canGoNext      = false;
    bool canGoPrevious  = false;
    bool canPause       = false;
    bool canPlay        = false;
    bool canSeek        = false;
    bool shuffle        = false;
//    bool mute           = false;
//    double volume       = 0;

    Player::PlaybackMode    mode    = Player::RepeatAll;
    Player::PlaybackStatus  status  = Player::InvalidPlaybackStatus;

    // media property
    QVariantMap metadata;
    double rate;
    double maximumRate;
    double minimumRate;
    qlonglong position  = 0;

    QMediaPlayer    *qplayer;
    PlaylistPtr     activePlaylist;
    MusicMeta       activeMeta;

    Player *q_ptr;
    Q_DECLARE_PUBLIC(Player);
};

Player::Player(QObject *parent) : QObject(parent), d_ptr(new PlayerPrivate(this))
{
    Q_D(Player);
    d->initConnection();
}

Player::~Player()
{

}

void Player::playMeta(PlaylistPtr playlist, const MusicMeta &meta)
{
    Q_D(Player);
    qDebug() << "--------" << meta.localPath;

    d->qplayer->blockSignals(true);
    d->qplayer->setMedia(QMediaContent(QUrl::fromLocalFile(meta.localPath)));
    d->qplayer->blockSignals(false);

    this->setPosition(meta.offset);

    QTimer::singleShot(100, this, [ = ]() {
        d->qplayer->play();
    });
    d->activePlaylist = playlist;
    d->activeMeta = meta;
    d->activePlaylist->play(meta);
    emit mediaPlayed(d->activePlaylist, d->activeMeta);
}

void Player::resume(PlaylistPtr playlist, const MusicMeta &meta)
{
    Q_D(Player);
    Q_ASSERT(playlist == d->activePlaylist);
    Q_ASSERT(meta.hash == d->activeMeta.hash);
    QTimer::singleShot(50, this, [ = ]() {
        d->qplayer->play();
    });
}

void Player::playNextMusic(PlaylistPtr playlist, const MusicMeta &meta)
{

}

void Player::playPrevMusic(PlaylistPtr playlist, const MusicMeta &meta)
{

}

void Player::pause()
{

}

void Player::stop()
{

}

MusicMeta Player::activeMeta() const
{
    Q_D(const Player);
    return d->activeMeta;
}

PlaylistPtr Player::activePlaylist() const
{
    Q_D(const Player);
    return d->activePlaylist;
}

//!
//! \brief canControl
//! Always be true
bool Player::canControl() const
{
    Q_D(const Player);
    return d->canControl;
}

qlonglong Player::position() const
{
    Q_D(const Player);
    return d->qplayer->position();
}

double Player::volume() const
{
    Q_D(const Player);
    return d->qplayer->volume();
}

Player::PlaybackMode Player::mode() const
{

}

bool Player::muted() const
{
    Q_D(const Player);
    return d->qplayer->isMuted();
}

qint64 Player::duration() const
{
    Q_D(const Player);
    return d->qplayer->duration();
}

void Player::setCanControl(bool canControl)
{
    qCritical() << "Never Changed this" << canControl;
}

void Player::setPosition(qlonglong position)
{
    Q_D(const Player);
    return d->qplayer->setPosition(position);
}

void Player::setMode(Player::PlaybackMode mode)
{
    Q_D(Player);
    d->mode = mode;
}

void Player::setVolume(double volume)
{
    Q_D(Player);
    d->qplayer->setVolume(volume);
}

void Player::setMuted(bool mute)
{
    Q_D(Player);
    d->qplayer->setMuted(mute);
}

void PlayerPrivate::initConnection()
{
    Q_Q(Player);
    q->connect(qplayer, &QMediaPlayer::positionChanged,
    q, [ = ](qint64 position) {
        emit q->positionChanged(position, qplayer->duration());
    });
    q->connect(qplayer, &QMediaPlayer::mutedChanged,
               q, &Player::mutedChanged);
    q->connect(qplayer, &QMediaPlayer::durationChanged,
               q, &Player::durationChanged);

    q->connect(qplayer, &QMediaPlayer::mediaStatusChanged,
    q, [ = ](QMediaPlayer::MediaStatus status) {
        qDebug() << status;
//        switch (state) {
//        case QMediaPlayer::StoppedState: {
////            qDebug() << "auto change next music";
////            this->selectNext(m_info, m_mode);
////            break;
//        }
//        case QMediaPlayer::PlayingState:
//        case QMediaPlayer::PausedState:
//            break;
//        }
    });

    q->connect(qplayer, static_cast<void (QMediaPlayer::*)(QMediaPlayer::Error error)>(&QMediaPlayer::error),
    q, [ = ](QMediaPlayer::Error error) {
        qDebug() << error;
//        switch (state) {
//        case QMediaPlayer::StoppedState: {
////            qDebug() << "auto change next music";
////            this->selectNext(m_info, m_mode);
////            break;
//        }
//        case QMediaPlayer::PlayingState:
//        case QMediaPlayer::PausedState:
//            break;
//        }
    });

    q->connect(qplayer, &QMediaPlayer::stateChanged,
    q, [ = ](QMediaPlayer::State state) {
        qDebug() << state;
        switch (state) {
        case QMediaPlayer::StoppedState: {
//            qDebug() << "auto change next music";
//            this->selectNext(m_info, m_mode);
//            break;
        }
        case QMediaPlayer::PlayingState:
        case QMediaPlayer::PausedState:
            break;
        }
    });
}
