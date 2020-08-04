/*
 * Copyright (C) 2016 ~ 2018 Wuhan Deepin Technology Co., Ltd.
 *
 * Author:     Iceyer <me@iceyer.net>
 *
 * Maintainer: Iceyer <me@iceyer.net>
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

#pragma once

#include <QObject>
#include <QVariantMap>
#include <QScopedPointer>
#include <QMediaContent>

#include <Mpris>

#include <util/singleton.h>
#include "playlist.h"
#include "vlc/MediaPlayer.h"

class QAudioBuffer;
class PlayerPrivate;
class Player : public QObject, public DMusic::DSingleton<Player>
{
    Q_OBJECT

public:
    enum PlaybackStatus {
        InvalidPlaybackStatus = -1,
        Stopped = 0,
        Playing,
        Paused,
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

    static const int VolumeStep = 10;

    explicit Player(QObject *parent = nullptr);
    ~Player();

public:
    void init();

    void setActivePlaylist(PlaylistPtr playlist);
    void setCurPlaylist(PlaylistPtr curPlaylist);
    void loadMedia(PlaylistPtr playlist, const MetaPtr meta);
    void playMeta(PlaylistPtr playlist, const MetaPtr meta);
    void resume(PlaylistPtr playlist, const MetaPtr meta);
    void playNextMeta(PlaylistPtr playlist, const MetaPtr meta);
    void playNextMeta();
    void playPrevMusic(PlaylistPtr playlist, const MetaPtr meta);
    void pause();
    void pauseNow();
    void stop();

    VlcMediaPlayer *core();
    PlaybackStatus status();

    bool isActiveMeta(MetaPtr meta) const;
    MetaPtr activeMeta() const;
    PlaylistPtr curPlaylist() const;
    PlaylistPtr activePlaylist() const;
    QStringList supportedFilterStringList()const;
    QStringList supportedSuffixList()const;
    QStringList supportedMimeTypes() const;

signals:
    void mediaUpdate(PlaylistPtr playlist, const MetaPtr meta);
    void mediaPlayed(PlaylistPtr playlist, const MetaPtr meta);
    void mediaError(PlaylistPtr playlist, const MetaPtr meta, Player::Error error);

    void readyToResume();

public:
    bool canControl() const;
    qlonglong position() const;
    int volume() const;
    PlaybackMode mode() const;
    bool muted();
    qint64 duration() const;
    double fadeInOutFactor() const;
    bool fadeInOut() const;
    bool playOnLoaded() const;

signals:
    void canControlChanged(bool canControl);
    void playbackStatusChanged(Player::PlaybackStatus playbackStatus);
    void positionChanged(qlonglong position, qlonglong length, qint64 coefficient);
    void sliderReleased(qint64 value);

    void volumeChanged(int volume);
    void modeChanged(PlaybackMode mode);
    void mutedChanged(bool muted);
    /************************************************
     * local mute operation
     * *********************************************/
    void localMutedChanged();
    void durationChanged(qint64 duration);
    void fadeInOutFactorChanged(double fadeInOutFactor);
    void fadeInOutChanged(bool fadeInOut);
    void playOnLoadedChanged(bool playOnLoaded);
    void audioBufferProbed(const QAudioBuffer &buffer);

public slots:
    void setCanControl(bool canControl);
    void setPosition(qlonglong position);
    void setMode(PlaybackMode mode);
    void setVolume(int volume);
    void setMuted(bool muted);
    /*********************
     * local mute
     * *********************/
    void setLocalMuted(bool muted);
    /*********************
     * to dbus mute
     * *********************/
    void setDbusMuted(bool muted = false);
    void setFadeInOutFactor(double fadeInOutFactor);
    void setFadeInOut(bool fadeInOut);
    void setPlayOnLoaded(bool playOnLoaded);
    void musicFileMiss();
    void setEqualizer(bool enabled, int curIndex, QList<int> indexbaud);
    void setEqualizerEnable(bool enable);
    void setEqualizerpre(int val);
    void setEqualizerbauds(int index, int val);
    void setEqualizerCurMode(int curIndex);
    /***********************************************
     * if player stat is stop state or
     * device does not start
     * **********************************/
    bool isValidDbusMute();
private:
    void readSinkInputPath();
    bool setMusicVolume(double volume);
    bool setMusicMuted(bool muted);
    bool isMusicMuted();
    bool isDevValid();
private:
    friend class DMusic::DSingleton<Player>;
    QScopedPointer<PlayerPrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), Player)
};

Q_DECLARE_METATYPE(Player::Error)
Q_DECLARE_METATYPE(Player::PlaybackStatus)

