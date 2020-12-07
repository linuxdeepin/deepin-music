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
#include <QPropertyAnimation>
#include <QPixmap>
#include <QIcon>

#include <MprisPlayer>
#include <mediameta.h>

#include <util/singleton.h>
#include "vlc/MediaPlayer.h"

class QAudioBuffer;
class MprisPlayer;
class Player : public QObject, public DMusic::DSingleton<Player>
{
    Q_OBJECT
    Q_PROPERTY(double fadeInOutFactor READ fadeInOutFactor WRITE setFadeInOutFactor NOTIFY fadeInOutFactorChanged)
public:
    enum PlaybackStatus {
        InvalidPlaybackStatus = -1,
        Stopped = 0,
        Playing,
        Paused,
    };

    enum PlaybackMode {
        RepeatNull = -1,
        RepeatAll,
        RepeatSingle,
        Shuffle,
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
    //zy---begin
public:
    void playMeta(MediaMeta meta);
    void pause();
    void resume();
    void pauseNow();
    void playPreMeta();
    void playNextMeta(bool isAuto);
    // 清除播放列表
    void clearPlayList();
    // 移出播放列表中的歌曲
    void playRmvMeta(const QStringList &metalist);
    // 添加播放歌曲
    void playListAppendMeta(MediaMeta meta);
    // 设置播放列表
    void setPlayList(const QList<MediaMeta> &list);
    // 获取播放列表
    QList<MediaMeta> *getPlayList();
    //获取dbus实例
    MprisPlayer *getMpris() const;
    //当前播放playlist的hash
    void setCurrentPlayListHash(QString hash, bool reloadMetaList); //reloadMetaList为true表示需要更新播放列表
    QString getCurrentPlayListHash();
    PlaybackStatus status();
    MediaMeta activeMeta();
    QIcon playingIcon();
    //启动加载歌曲进度
    void loadMediaProgress(const QString &path);
    //设置当前歌曲
    void setActiveMeta(const MediaMeta &meta);
    //播放列表中第一首歌，当音乐处于停止状态时
    void forcePlayMeta();
signals:
    // 播放状态改变
    void signalPlaybackStatusChanged(Player::PlaybackStatus playbackStatus);
    // 播放音乐改变
    void signalMediaMetaChanged();
    // 播放状态动态图改变
    void signalUpdatePlayingIcon();
    // 播放列表改变
    void signalPlayListChanged();
    // 当前歌曲删除或无效停止播放
    void signalMediaStop(const QString &activeHah);

public slots:
    void changePicture();
    void setVolume(int volume);
    void setMuted(bool muted);
private:
    QTimer         *m_timer = nullptr;
    QIcon           m_playingIcon = QIcon::fromTheme("music_play1");
    int             m_playingCount = 0;
    QString         m_currentPlayListHash;//当前正在播放的playlist的hash
    PlaybackMode    m_mode    = Player::RepeatAll;//循环播放模式
    //zy--end
public:
    void init();
    void stop();

    VlcMediaPlayer *core();
    QStringList supportedSuffixList()const;
    QStringList supportedMimeTypes() const;

signals:
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
    void initConnection();
    void initMpris();//dbus interface
    friend class DMusic::DSingleton<Player>;
    //begin
    MediaMeta m_ActiveMeta;
    QList<MediaMeta> m_MetaList;
    //end

    // player property
    bool m_canControl     = true;
    bool m_canGoNext      = false;
    bool m_canGoPrevious  = false;
    bool m_canPause       = false;
    bool m_canPlay        = false;
    bool m_canSeek        = false;
    bool m_shuffle        = false;
    bool m_mute           = false; // unused
    QString m_sinkInputPath;

    VlcInstance             *m_qvinstance;
    VlcMedia                *m_qvmedia;
    VlcMediaPlayer          *m_qvplayer;
//    MetaPtr                 m_activeMeta;

    int             m_volume      = 50.0;
    bool            m_playOnLoad  = true;
    bool            m_firstPlayOnLoad  = true; //外部双击打开处理一次
    bool            m_fadeInOut   = false;
    double          m_fadeInOutFactor     = 1.0;
    qlonglong       m_m_position          = 0;//只能用于判断音乐是否正常结束

    MprisPlayer     *m_pMpris = nullptr;    //音乐dbus接口

    QPropertyAnimation  *m_fadeInAnimation    = nullptr;
    QPropertyAnimation  *m_fadeOutAnimation   = nullptr;
};

Q_DECLARE_METATYPE(Player::Error)
Q_DECLARE_METATYPE(Player::PlaybackStatus)
