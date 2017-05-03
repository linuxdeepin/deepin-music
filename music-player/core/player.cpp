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
#include <QMimeDatabase>
#include <QMediaPlayer>
#include <QPropertyAnimation>
#include "metasearchservice.h"

#include <QMimeDatabase>

static QMap<QString, bool>  sSupportedSuffix;
static QStringList          sSupportedSuffixList;
static QStringList          sSupportedFiterList;
static QStringList          sSupportedMimeTypes;

static const int sFadeInOutAnimationDuration = 400; //ms

void initMiniTypes()
{
    //black list
    QHash<QString, bool> suffixBlacklist;
    suffixBlacklist.insert("m3u", true);

    QHash<QString, bool> suffixWhitelist;
    suffixWhitelist.insert("cue", true);

    QStringList  mimeTypeWhiteList;
    mimeTypeWhiteList << "application/vnd.ms-asf";

    QMimeDatabase mdb;
    for (auto &mt : mdb.allMimeTypes()) {
        if (mt.name().startsWith("audio/") || mt.name().startsWith("video/")) {
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
        if (mt.name().startsWith("video/")) {
            sSupportedMimeTypes << mt.name();
        }

        if (mt.name().startsWith("application/octet-stream")) {
            sSupportedMimeTypes << mt.name();
        }
    }

    sSupportedMimeTypes << mimeTypeWhiteList;

    for (auto &suffix : suffixWhitelist.keys()) {
        sSupportedSuffixList << "*." + suffix;
        sSupportedSuffix.insert(suffix, true);
    }
}

QStringList Player::supportedFilterStringList() const
{
    return sSupportedFiterList;
}

QStringList Player::supportedSuffixList() const
{
    return sSupportedSuffixList;
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
    }

    void initConnection();
    void selectPrev(const MetaPtr info, Player::PlaybackMode mode);
    void selectNext(const MetaPtr info, Player::PlaybackMode mode);

    // player property
    bool canControl     = true;
    bool canGoNext      = false;
    bool canGoPrevious  = false;
    bool canPause       = false;
    bool canPlay        = false;
    bool canSeek        = false;
    bool shuffle        = false;
    bool mute           = false; // unused
//    double volume       = 0;

    Player::PlaybackMode    mode    = Player::RepeatAll;
    Player::PlaybackStatus  status  = Player::InvalidPlaybackStatus;


    QMediaPlayer    *qplayer;
    PlaylistPtr     activePlaylist;
    MetaPtr         activeMeta;

    double          volume      = 50.0;
    bool            playOnLoad  = true;
    bool            fadeInOut   = true;
    double          fadeInOutFactor     = 1.0;

    QPropertyAnimation  *fadeInAnimation    = nullptr;
    QPropertyAnimation  *fadeOutAnimation   = nullptr;

    Player *q_ptr;
    Q_DECLARE_PUBLIC(Player)
};

void PlayerPrivate::initConnection()
{
    Q_Q(Player);

    qplayer = new QMediaPlayer(q);
    qplayer->setAudioRole(QAudio::MusicRole);
    initMiniTypes();

    q->connect(qplayer, &QMediaPlayer::positionChanged,
    q, [ = ](qint64 position) {
        if (activeMeta.isNull()) {
            return;
        }

        auto duration = qplayer->duration();
//        qDebug() << DMusic::lengthString(duration)
//                 << DMusic::lengthString(position)
//                 << DMusic::lengthString(activeMeta->offset)
//                 << DMusic::lengthString(activeMeta->length)
//                 << activeMeta->title;

        if (position > 1 && activeMeta->invalid) {
            emit q->mediaError(activePlaylist, activeMeta, Player::NoError);
        }

        // fix len
        if (activeMeta->length == 0 && duration != 0 && duration > 0) {
            activeMeta->length = duration;
            qDebug() << "update" << activeMeta->length;
            emit q->mediaUpdate(activePlaylist, activeMeta);
        }

        if (position >= activeMeta->offset + activeMeta->length + 1800 && qplayer->state() == QMediaPlayer::PlayingState) {
            qDebug() << "WARN!!! change to next by position change";
            selectNext(activeMeta, mode);
            return;
        }

//        qDebug() << position << sFadeInOutAnimationDuration << activeMeta->offset << activeMeta->length;
//        qDebug() << position/1000 << sFadeInOutAnimationDuration << activeMeta->offset/1000 << activeMeta->length/1000;

//        if (position + (sFadeInOutAnimationDuration) >= activeMeta->offset + activeMeta->length) {
//            qDebug() << "start fade out";
//            if (fadeInOut&& !fadeOutAnimation) {
//                fadeOutAnimation = new QPropertyAnimation(q, "fadeInOutFactor");
//                fadeOutAnimation->setStartValue(1.0000);
//                fadeOutAnimation->setKeyValueAt(0.9999, 0.1000);
//                fadeOutAnimation->setEndValue(1.0000);
//                fadeOutAnimation->setDuration(sFadeInOutAnimationDuration);
//                q->connect(fadeOutAnimation, &QPropertyAnimation::finished,
//                q, [ = ]() {
//                    fadeOutAnimation->deleteLater();
//                    fadeOutAnimation = nullptr;
//                });
//                fadeOutAnimation->start();
//            }
//        }

        emit q->positionChanged(position - activeMeta->offset,  activeMeta->length);
    });

    q->connect(qplayer, &QMediaPlayer::stateChanged,
    q, [ = ](QMediaPlayer::State newState) {
        switch (newState) {
        case QMediaPlayer::StoppedState:
            emit q->playbackStatusChanged(Player::Stopped);
            break;
        case QMediaPlayer::PlayingState:
            emit q->playbackStatusChanged(Player::Playing);
            break;
        case QMediaPlayer::PausedState:
            emit q->playbackStatusChanged(Player::Paused);
            break;
        }
    });

    q->connect(qplayer, &QMediaPlayer::volumeChanged,
    q, [ = ](int volume) {
        emit q->volumeChanged(volume / fadeInOutFactor);
    });
    q->connect(qplayer, &QMediaPlayer::mutedChanged,
               q, &Player::mutedChanged);
    q->connect(qplayer, &QMediaPlayer::durationChanged,
               q, &Player::durationChanged);

    q->connect(qplayer, &QMediaPlayer::mediaStatusChanged,
    q, [ = ](QMediaPlayer::MediaStatus status) {
//        qDebug() << "change " << status;
        switch (status) {
        case QMediaPlayer::LoadedMedia: {
            //wtf the QMediaPlayer can play image format, 233333333
            QMimeDatabase db;
            QMimeType type = db.mimeTypeForFile(activeMeta->localPath, QMimeDatabase::MatchContent);
            if (!sSupportedMimeTypes.contains(type.name())) {
                qDebug() << "unsupport mime type" << type << activePlaylist << activeMeta;
                qplayer->pause();
                qplayer->stop();
                emit q->mediaError(activePlaylist, activeMeta, Player::FormatError);
                return;
            }

            if (playOnLoad) {
                qplayer->play();
            }
            break;
        }
        case QMediaPlayer::EndOfMedia: {
            // next
            selectNext(activeMeta, mode);
            break;
        }

        case QMediaPlayer::LoadingMedia: {
            Q_ASSERT(!activeMeta.isNull());

            break;
        }
        case QMediaPlayer::UnknownMediaStatus:
        case QMediaPlayer::NoMedia:
        case QMediaPlayer::StalledMedia:
        case QMediaPlayer::BufferedMedia:
        case QMediaPlayer::BufferingMedia:
        case QMediaPlayer::InvalidMedia:
            break;
        }
    });

    q->connect(qplayer, static_cast<void (QMediaPlayer::*)(QMediaPlayer::Error error)>(&QMediaPlayer::error),
    q, [ = ](QMediaPlayer::Error error) {
        qWarning() << error << activePlaylist << activeMeta;
        emit q->mediaError(activePlaylist, activeMeta, static_cast<Player::Error>(error));
    });

    q->connect(qplayer, &QMediaPlayer::stateChanged,
    q, [ = ](QMediaPlayer::State state) {
//        qDebug() << "change " << state;
        switch (state) {
        case QMediaPlayer::StoppedState:
        case QMediaPlayer::PlayingState:
        case QMediaPlayer::PausedState:
            break;
        }
    });
}

void PlayerPrivate::selectNext(const MetaPtr info, Player::PlaybackMode mode)
{
    Q_Q(Player);
    if (!activePlaylist || activePlaylist->isEmpty()) {
        return;
    }

    switch (mode) {
    case Player::RepeatAll: {
        q->playMeta(activePlaylist, activePlaylist->next(info));
        break;
    }
    case Player::RepeatSingle: {
        q->playMeta(activePlaylist, info);
        break;
    }
    case Player::Shuffle: {
        int randomValue = qrand() % activePlaylist->length();
        q->playMeta(activePlaylist, activePlaylist->music(randomValue));
        break;
    }
    }
}

void PlayerPrivate::selectPrev(const MetaPtr info, Player::PlaybackMode mode)
{
    Q_Q(Player);
    if (!activePlaylist || activePlaylist->isEmpty()) {
        return;
    }

    switch (mode) {
    case Player::RepeatAll: {
        q->playMeta(activePlaylist, activePlaylist->prev(info));
        break;
    }
    case Player::RepeatSingle: {
        q->playMeta(activePlaylist, info);
        break;
    }
    case Player::Shuffle: {
        int randomValue = qrand() % activePlaylist->length();
        q->playMeta(activePlaylist, activePlaylist->music(randomValue));
        break;
    }
    }
}

Player::Player(QObject *parent) : QObject(parent), d_ptr(new PlayerPrivate(this))
{
}

void Player::init()
{
    Q_D(Player);
    qRegisterMetaType<Player::Error>();
    qRegisterMetaType<Player::PlaybackStatus>();

    d->initConnection();
}

Player::~Player()
{
    qDebug() << "destroy Player";
    Q_D(Player);
    d->qplayer->stop();
    d->qplayer->deleteLater();

    delete d->qplayer;
    qDebug() << "Player destroyed";
}


void Player::loadMedia(PlaylistPtr playlist, const MetaPtr meta)
{
    qDebug() << "loadMedia"
             << meta->title
             << DMusic::lengthString(meta->offset) << "/"
             << DMusic::lengthString(meta->length);
    Q_D(Player);
    d->activeMeta = meta;
    d->activePlaylist = playlist;

    d->qplayer->blockSignals(true);
    d->qplayer->setMedia(QMediaContent(QUrl::fromLocalFile(meta->localPath)));
    d->qplayer->blockSignals(false);
    d->activePlaylist->play(meta);
}

void Player::playMeta(PlaylistPtr playlist, const MetaPtr meta)
{
    qDebug() << "playMeta"
             << meta->title
             << DMusic::lengthString(meta->offset) << "/"
             << DMusic::lengthString(meta->length);

    Q_D(Player);
    d->activePlaylist = playlist;

    d->activeMeta = meta;
    d->qplayer->setMedia(QMediaContent(QUrl::fromLocalFile(meta->localPath)));
    d->qplayer->setPosition(meta->offset);
    d->activePlaylist->play(meta);

    emit mediaPlayed(d->activePlaylist, d->activeMeta);

    if (d->qplayer->mediaStatus() == QMediaPlayer::BufferedMedia) {
        QTimer::singleShot(100, this, [ = ]() {
            d->qplayer->play();
        });
    }
}

void Player::resume(PlaylistPtr playlist, const MetaPtr meta)
{
    Q_D(Player);
    qDebug() << "resume top";
    Q_ASSERT(playlist == d->activePlaylist);
    Q_ASSERT(meta->hash == d->activeMeta->hash);

    setPlayOnLoaded(true);
    QTimer::singleShot(50, this, [ = ]() {
        d->qplayer->play();
    });

    if (d->fadeOutAnimation) {
        d->fadeOutAnimation->stop();
        d->fadeOutAnimation->deleteLater();
        d->fadeOutAnimation = nullptr;
    }
    if (d->fadeInOut && !d->fadeInAnimation) {
        qDebug() << "start fade in";
        d->fadeInAnimation = new QPropertyAnimation(this, "fadeInOutFactor");
        d->fadeInAnimation->setStartValue(0.10000);
        d->fadeInAnimation->setEndValue(1.0000);
        d->fadeInAnimation->setDuration(sFadeInOutAnimationDuration);
        connect(d->fadeInAnimation, &QPropertyAnimation::finished,
        this, [ = ]() {
            d->fadeInAnimation->deleteLater();
            d->fadeInAnimation = nullptr;
        });
        d->fadeInAnimation->start();
    }
}

void Player::playNextMeta(PlaylistPtr playlist, const MetaPtr meta)
{
    Q_D(Player);
    Q_ASSERT(playlist == d->activePlaylist);

    setPlayOnLoaded(true);
    if (d->mode == RepeatSingle) {
        d->selectNext(meta, RepeatAll);
    } else {
        d->selectNext(meta, d->mode);
    }
}

void Player::playPrevMusic(PlaylistPtr playlist, const MetaPtr meta)
{
    Q_D(Player);
    Q_ASSERT(playlist == d->activePlaylist);

    setPlayOnLoaded(true);
    if (d->mode == RepeatSingle) {
        d->selectPrev(meta, RepeatAll);
    } else {
        d->selectPrev(meta, d->mode);
    }
}

void Player::pause()
{
    Q_D(Player);

    if (d->fadeInAnimation) {
        d->fadeInAnimation->stop();
        d->fadeInAnimation->deleteLater();
        d->fadeInAnimation = nullptr;
    }

    if (d->fadeInOut && !d->fadeOutAnimation) {
        d->fadeOutAnimation = new QPropertyAnimation(this, "fadeInOutFactor");
        d->fadeOutAnimation->setStartValue(1.0000);
        d->fadeOutAnimation->setKeyValueAt(0.9999, 0.1000);
        d->fadeOutAnimation->setEndValue(1.0000);
        d->fadeOutAnimation->setDuration(sFadeInOutAnimationDuration);
        connect(d->fadeOutAnimation, &QPropertyAnimation::finished,
        this, [ = ]() {
            d->fadeOutAnimation->deleteLater();
            d->fadeOutAnimation = nullptr;
            d->qplayer->pause();
        });
        d->fadeOutAnimation->start();
    } else {
        d->qplayer->pause();
        setFadeInOutFactor(1.0);
    }
}

void Player::stop()
{
    Q_D(Player);
//    d->qplayer->blockSignals(true);
    d->qplayer->pause();
    d->qplayer->setMedia(QMediaContent());
    d->qplayer->stop();
    //    d->qplayer->blockSignals(false);
}

Player::PlaybackStatus Player::status()
{
    Q_D(const Player);
    return static_cast<PlaybackStatus>(d->qplayer->state());
}

bool Player::isActiveMeta(MetaPtr meta) const
{
    Q_D(const Player);
    return  !meta.isNull()
            && ! d->activeMeta.isNull()
            && (meta->hash == d->activeMeta->hash);
}

MetaPtr Player::activeMeta() const
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

int Player::volume() const
{
    Q_D(const Player);
    return static_cast<int>(d->volume);
}

Player::PlaybackMode Player::mode() const
{
    Q_D(const Player);
    return  d->mode;
}

bool Player::muted() const
{
    Q_D(const Player);
    return d->qplayer->isMuted();
}

qint64 Player::duration() const
{
    Q_D(const Player);
    if (d->activeMeta.isNull()) {
        return 0;
    }

    if (d->qplayer->duration() == d->activeMeta->length) {
        return d->qplayer->duration();
    } else {
        return  d->activeMeta->length;
    }
}

double Player::fadeInOutFactor() const
{
    Q_D(const Player);
    return d->fadeInOutFactor;
}

bool Player::fadeInOut() const
{
    Q_D(const Player);
    return d->fadeInOut;
}

bool Player::playOnLoaded() const
{
    Q_D(const Player);
    return d->playOnLoad;
}

void Player::setCanControl(bool canControl)
{
    qCritical() << "Never Changed this" << canControl;
}

void Player::setPosition(qlonglong position)
{
    Q_D(const Player);

    if (d->activeMeta.isNull()) {
        return;
    }

    if (d->qplayer->duration() == d->activeMeta->length) {
        return d->qplayer->setPosition(position);
    } else {
        d->qplayer->setPosition(position + d->activeMeta->offset);
    }
}

void Player::setMode(Player::PlaybackMode mode)
{
    Q_D(Player);
    d->mode = mode;
}

void Player::setVolume(double volume)
{
    Q_D(Player);
    if (volume > 100) {
        volume = 100;
    }

    if (volume < 0) {
        volume = 0;
    }

    d->volume = volume;

    d->qplayer->blockSignals(true);
    d->qplayer->setVolume(d->volume * d->fadeInOutFactor);
    d->qplayer->blockSignals(false);
}

void Player::setMuted(bool mute)
{
    Q_D(Player);
    d->qplayer->setMuted(mute);
}

void Player::setFadeInOutFactor(double fadeInOutFactor)
{
    Q_D(Player);
    d->fadeInOutFactor = fadeInOutFactor;
//    qDebug() << "setFadeInOutFactor" << fadeInOutFactor
//             << d->volume *d->fadeInOutFactor << d->volume;
    d->qplayer->blockSignals(true);
    d->qplayer->setVolume(d->volume * d->fadeInOutFactor);
    d->qplayer->blockSignals(false);

}

void Player::setFadeInOut(bool fadeInOut)
{
    Q_D(Player);
    d->fadeInOut = fadeInOut;
}

void Player::setPlayOnLoaded(bool playOnLoaded)
{
    Q_D(Player);
    d->playOnLoad = playOnLoaded;
}
