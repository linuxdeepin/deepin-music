/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#pragma once

#include <QObject>
#include <QVariantMap>
#include <QScopedPointer>
#include <QMediaContent>

#include <Mpris>

#include "core/util/singleton.h"
#include "playlist.h"

class PlayerPrivate;
class Player : public QObject, public Singleton<Player>
{
    Q_OBJECT

    Q_PROPERTY(bool canControl READ canControl WRITE setCanControl NOTIFY canControlChanged)
//    Q_PROPERTY(bool canGoNext READ canGoNext WRITE setCanGoNext NOTIFY canGoNextChanged)
//    Q_PROPERTY(bool canGoPrevious READ canGoPrevious WRITE setCanGoPrevious NOTIFY canGoPreviousChanged)
//    Q_PROPERTY(bool canPause READ canPause WRITE setCanPause NOTIFY canPauseChanged)
//    Q_PROPERTY(bool canPlay READ canPlay WRITE setCanPlay NOTIFY canPlayChanged)
//    Q_PROPERTY(bool canSeek READ canSeek WRITE setCanSeek NOTIFY canSeekChanged)
//    Q_PROPERTY(Mpris::LoopStatus loopStatus READ loopStatus WRITE setLoopStatus NOTIFY loopStatusChanged)
//    Q_PROPERTY(double maximumRate READ maximumRate WRITE setMaximumRate NOTIFY maximumRateChanged)
//    Q_PROPERTY(QVariantMap metadata READ metadata WRITE setMetadata NOTIFY metadataChanged)
//    Q_PROPERTY(double minimumRate READ minimumRate WRITE setMinimumRate NOTIFY minimumRateChanged)
//    Q_PROPERTY(PlaybackStatus playbackStatus READ playbackStatus WRITE setPlaybackStatus NOTIFY playbackStatusChanged)
    Q_PROPERTY(qlonglong position READ position WRITE setPosition NOTIFY positionChanged)
//    Q_PROPERTY(double rate READ rate WRITE setRate NOTIFY rateChanged)
//    Q_PROPERTY(bool shuffle READ shuffle WRITE setShuffle NOTIFY shuffleChanged)
    Q_PROPERTY(double volume READ volume WRITE setVolume NOTIFY volumeChanged)
    Q_PROPERTY(qint64 duration READ duration NOTIFY durationChanged)
    Q_PROPERTY(PlaybackMode mode READ mode WRITE setMode NOTIFY modeChanged)
    Q_PROPERTY(bool muted READ muted WRITE setMuted NOTIFY mutedChanged)

    Q_ENUMS(PlaybackStatus)
    Q_ENUMS(PlaybackMode)
    Q_ENUMS(Error)

public:
    enum PlaybackStatus {
        InvalidPlaybackStatus = -1,
        Playing,
        Paused,
        Stopped
    };

    enum PlaybackMode {
        RepeatAll = 0,
        RepeatSingle = 1,
        Shuffle = 2,
    };

    enum Error {
        NoError,
        ResourceError,
        FormatError,
        NetworkError,
        AccessDeniedError,
        ServiceMissingError,
        MediaIsPlaylist
    };
    explicit Player(QObject *parent = 0);
    ~Player();

public:
    void init() {}

    void playMeta(PlaylistPtr playlist, const MusicMeta &meta);
    void resume(PlaylistPtr playlist, const MusicMeta &meta);
    void playNextMeta(PlaylistPtr playlist, const MusicMeta &meta);
    void playPrevMusic(PlaylistPtr playlist, const MusicMeta &meta);
    void pause();
    void stop();

    MusicMeta activeMeta() const;
    PlaylistPtr activePlaylist() const;
    QStringList supportedFilterStringList()const;
    QStringList supportedSuffixList()const;
    QStringList supportedMimeTypes() const;

signals:
    void mediaUpdate(PlaylistPtr playlist, const MusicMeta &meta);
    void mediaPlayed(PlaylistPtr playlist, const MusicMeta &meta);
    void mediaError(PlaylistPtr playlist, const MusicMeta &meta, Player::Error error);

public:
    bool canControl() const;
//    bool canGoNext() const;
//    bool canGoPrevious() const;
//    bool canPause() const;
//    bool canPlay() const;
//    bool canSeek() const;
//    Mpris::LoopStatus loopStatus() const;
//    double maximumRate() const;
//    QVariantMap metadata() const;
//    double minimumRate() const;
//    PlaybackStatus playbackStatus() const;
    qlonglong position() const;
//    double rate() const;
//    bool shuffle() const;
    double volume() const;
    PlaybackMode mode() const;
    bool muted() const;
    qint64 duration() const;

signals:
    void canControlChanged(bool canControl);
//    void canGoNextChanged(bool canGoNext);
//    void canGoPreviousChanged(bool canGoPrevious);
//    void canPauseChanged(bool canPause);
//    void canPlayChanged(bool canPlay);
//    void canSeekChanged(bool canSeek);
//    void loopStatusChanged(Mpris::LoopStatus loopStatus);
//    void maximumRateChanged(double maximumRate);
//    void metadataChanged(QVariantMap metadata);
//    void minimumRateChanged(double minimumRate);
//    void playbackStatusChanged(PlaybackStatus playbackStatus);
    void positionChanged(qlonglong position, qlonglong length);
//    void rateChanged(double rate);
//    void shuffleChanged(bool shuffle);
    void volumeChanged(double volume);
    void modeChanged(PlaybackMode mode);
    void mutedChanged(bool muted);
    void durationChanged(qint64 duration);

public slots:
    void setCanControl(bool canControl);
//    void setCanGoNext(bool canGoNext);
//    void setCanGoPrevious(bool canGoPrevious);
//    void setCanPause(bool canPause);
//    void setCanPlay(bool canPlay);
//    void setCanSeek(bool canSeek);
//    void setLoopStatus(Mpris::LoopStatus loopStatus);
//    void setMaximumRate(double maximumRate);
//    void setMetadata(QVariantMap metadata);
//    void setMinimumRate(double minimumRate);
//    void setPlaybackStatus(PlaybackStatus playbackStatus);
    void setPosition(qlonglong position);
    void setMode(PlaybackMode mode);
//    void setRate(double rate);
//    void setShuffle(bool shuffle);
    void setVolume(double volume);
    void setMuted(bool muted);

private:
    friend class Singleton<Player>;

    QScopedPointer<PlayerPrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), Player)
    qint64 m_duration;
};

Q_DECLARE_METATYPE(Player::Error);

