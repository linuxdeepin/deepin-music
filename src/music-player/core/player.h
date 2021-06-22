/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     ZouYa <zouya@uniontech.com>
 *
 * Maintainer: WangYu <wangyu@uniontech.com>
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
#include <QRandomGenerator>
#include <QDBusInterface>

#include <MprisPlayer>
#include <mediameta.h>

#include "util/singleton.h"
#include "vlc/MediaPlayer.h"

class QAudioBuffer;
class MprisPlayer;
class CdaThread;
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
    // zy---begin
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
    void playRmvMeta(const QStringList &metalistToDel);
    // 直接删除，不考虑逻辑
    void removeMeta(const QStringList &metalistToDel);
    // 添加播放歌曲
    void playListAppendMeta(const MediaMeta &meta);
    // 设置播放列表
    void setPlayList(const QList<MediaMeta> &list);
    // 获取播放列表
    QList<MediaMeta> *getPlayList();
    // 获取CD播放列表
    QList<MediaMeta> getCdaPlayList();
    // 获取dbus实例
    MprisPlayer *getMpris() const;
    // 当前播放playlist的hash, reloadMetaList为true表示需要更新播放列表
    void setCurrentPlayListHash(QString hash, bool reloadMetaList);
    QString getCurrentPlayListHash();
    PlaybackStatus status();
    MediaMeta getActiveMeta();
    QIcon playingIcon();
    // 启动加载歌曲进度
    //void loadMediaProgress(const QString &path);
    // 设置当前歌曲
    void setActiveMeta(const MediaMeta &meta);
    // 当音乐处于停止状态或者播放队列为空时，播放列表中第一首歌
    void forcePlayMeta();
    // 初始化均衡器配置
    void initEqualizerCfg();
    // 获取音量状态
    bool getMuted();
    // 获取音量大小
    int getVolume() const;

signals:
    // 播放状态改变
    void signalPlaybackStatusChanged(Player::PlaybackStatus playbackStatus);
    // 播放音乐改变
    void signalMediaMetaChanged(MediaMeta meta);
    // 播放状态动态图改变
    void signalUpdatePlayingIcon();
    // 播放列表改变
    void signalPlayListChanged();
    // 当前歌曲删除或无效停止播放
    void signalMediaStop(const QString &activeHah);
    // 播放列表中歌曲被删除
    void signalPlayQueueMetaRemove(const QString &metaHash);
    // 静音状态改变
    void signalMutedChanged();
    // 音量数值改变
    void signalVolumeChanged();
public slots:
    void changePicture();
    void setVolume(int volume);
    void setMuted(bool muted);
private:
    QTimer         *m_timer = nullptr;
    QIcon           m_playingIcon = QIcon::fromTheme("music_play1");
    int             m_playingCount = 0;
    // 当前正在播放的playlist的hash
    QString         m_currentPlayListHash;
    // 循环播放模式
    PlaybackMode    m_mode    = Player::RepeatAll;
    int             m_volume      = 50.0;
    // 按资源使用方法添加缓存图片
    QIcon           m_playingIcon1 = QIcon::fromTheme("music_play1");
    QIcon           m_playingIcon2 = QIcon::fromTheme("music_play2");
    QIcon           m_playingIcon3 = QIcon::fromTheme("music_play3");
    QIcon           m_playingIcon4 = QIcon::fromTheme("music_play4");
    // zy--end
public:
    void init();
    //当emitSignal为true时,清空当前播放刷新状态,false只在关闭应用时停止播放动作
    void stop(bool emitSignal = true);

    VlcMediaPlayer *core();
    QStringList supportedSuffixList()const;

signals:
    void readyToResume();

public:
    //bool canControl() const;
    qlonglong position();
    PlaybackMode mode() const;
    qint64 duration();
    double fadeInOutFactor() const;
    bool fadeInOut() const;
    //bool playOnLoaded() const;
    /**
     * @brief initCddTrack 初始化cddatrack值，以免引起因为配置问题无法加载cd的问题
     */
    void initCddTrack();

    void startCdaThread();
signals:
    void canControlChanged(bool canControl);
    void positionChanged(qlonglong position, qlonglong length, qint64 coefficient);
    void sliderReleased(qint64 value);

    void modeChanged(PlaybackMode mode);
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
//    void setLocalMuted(bool muted);
    /*********************
     * to dbus mute
     * *********************/
    void setDbusMuted(bool muted = false);
    void setFadeInOutFactor(double fadeInOutFactor);
    void setFadeInOut(bool fadeInOut);
    void setEqualizer(bool enabled, int curIndex, QList<int> indexbaud);
    void setEqualizerEnable(bool enable);
    void setEqualizerpre(int val);
    void setEqualizerbauds(int index, int val);
    void setEqualizerCurMode(int curIndex);
    void onSleepWhenTaking(bool sleep);
private:
    explicit Player(QObject *parent = nullptr);
    ~Player();
    friend class DMusic::DSingleton<Player>;
private:
    void readSinkInputPath();
    bool setMusicVolume(double volume);
    bool setMusicMuted(bool muted);
    bool isMusicMuted();
    bool isDevValid();
    /***********************************************
     * if player stat is stop state or
     * device does not start
     * **********************************/
    bool isValidDbusMute();
    // begin
    void initVlc();
    void initMpris();//dbus interface
    // 重置DBus中媒体信息
    void resetDBusMpris(const MediaMeta &meta);
    MediaMeta m_ActiveMeta;
    QList<MediaMeta> m_MetaList;
    // end

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
    QStringList m_supportedSuffix;

    VlcInstance             *m_qvinstance = nullptr;
    VlcMedia                *m_qvmedia = nullptr;
    VlcMediaPlayer          *m_qvplayer = nullptr;
    CdaThread               *m_pCdaThread = nullptr;

    // 外部双击打开处理一次
    bool            m_firstPlayOnLoad  = true;
    bool            m_fadeInOut   = false;
    double          m_fadeInOutFactor     = 1.0;
    // 只能用于判断音乐是否正常结束
    qlonglong       m_m_position          = 0;
    // 音乐dbus接口
    MprisPlayer     *m_mpris = nullptr;
    QDBusInterface      *m_pDBus        = nullptr;      //接收休眠信号
    int             m_Vlcstate = -1; //休眠状态缓存

    QPropertyAnimation  *m_fadeInAnimation    = nullptr;
    QPropertyAnimation  *m_fadeOutAnimation   = nullptr;
};

Q_DECLARE_METATYPE(Player::Error)
Q_DECLARE_METATYPE(Player::PlaybackStatus)
