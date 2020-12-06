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

#include "player.h"

#include <QDebug>
#include <QTimer>
#include <QMimeDatabase>
#include <QMediaPlayer>
#include <QPropertyAnimation>
#include <QAudioProbe>
#include <QFileSystemWatcher>
#include <QMimeDatabase>
#include <QDBusObjectPath>
#include <QDBusInterface>
#include <QDBusReply>
#include <QThread>
#include <QFileInfo>
#include <QDir>
#include <QTimer>
#include <QTime>
#include <QMutex>
#include <DRecentManager>
#include <QIcon>
#include <QPainter>

#include "util/dbusutils.h"
#include "util/global.h"
#include <unistd.h>


#include <vlc/vlc.h>
#include "vlc/Audio.h"
#include "vlc/Error.h"
#include "vlc/Common.h"
#include "vlc/Enums.h"
#include "vlc/Instance.h"
#include "vlc/Media.h"
#include "vlc/MediaPlayer.h"
#include "vlc/Equalizer.h"

#include "core/musicsettings.h"
#include <DPushButton>
#include <DPalette>

DCORE_USE_NAMESPACE
static QMap<QString, bool>  sSupportedSuffix;
static QStringList          sSupportedSuffixList;
static QStringList          sSupportedFiterList;
static QStringList          sSupportedMimeTypes;

static const int sFadeInOutAnimationDuration = 900; //ms

void initMiniTypes();

//QStringList Player::supportedFilterStringList() const
//{
//    return sSupportedFiterList;
//}

Player::Player(QObject *parent) : QObject(parent)
{
    m_qvinstance = new VlcInstance(VlcCommon::args(), nullptr);
    m_qvplayer = new VlcMediaPlayer(m_qvinstance);
    m_qvplayer->equalizer()->setPreamplification(12);
    m_qvmedia = new VlcMedia();
    m_playingIcon = QIcon();
    initMiniTypes();
    init();
}

void Player::init()
{
    qRegisterMetaType<Player::Error>();
    qRegisterMetaType<Player::PlaybackStatus>();

    m_fadeOutAnimation = new QPropertyAnimation(this, "fadeInOutFactor");
    m_fadeInAnimation = new QPropertyAnimation(this, "fadeInOutFactor");

    initConnection();

    initMpris();
}

QStringList Player::supportedSuffixList() const
{
    return sSupportedSuffixList;
}

QStringList Player::supportedMimeTypes() const
{
    return sSupportedMimeTypes;
}

Player::~Player()
{
    qDebug() << "destroy Player";
    delete m_qvmedia;
    delete m_qvplayer;
    delete m_qvinstance;
    delete m_fadeOutAnimation;
    delete m_fadeInAnimation;

    qDebug() << "Player destroyed";
}

void Player::playMeta(MediaMeta meta)
{
    if (meta.hash != "") {
        m_ActiveMeta = meta;
        setActiveMeta(meta);
        /*************************
         * mute to dbus
         * ***********************/
        setDbusMuted();
        //    m_activeMeta = curMeta;
        m_isamr = true;
        m_qvmedia->initMedia(meta.localPath, true, m_qvinstance);
        m_qvplayer->open(m_qvmedia);
        m_qvplayer->setTime(meta.offset);
        m_qvplayer->play();
        //    emit playbackStatusChanged(status());

        DRecentData data;
        data.appName = Global::getAppName();
        data.appExec = "deepin-music";
        DRecentManager::addItem(meta.localPath, data);

        m_fadeOutAnimation->stop();
        if (m_fadeInOut && m_fadeInAnimation->state() != QPropertyAnimation::Running) {
            qDebug() << "start fade in";
            m_fadeInAnimation->setEasingCurve(QEasingCurve::InCubic);
            m_fadeInAnimation->setStartValue(0.10000);
            m_fadeInAnimation->setEndValue(1.0000);
            m_fadeInAnimation->setDuration(sFadeInOutAnimationDuration);
            m_fadeInAnimation->start();
        }
    }
}

void Player::resume()
{
    if (m_ActiveMeta.localPath.isEmpty()) {
        return;
    }

//    if (QFileInfo(meta->localPath).dir().isEmpty()) {
//        Q_EMIT mediaError(playlist, meta, Player::ResourceError);
//        return ;
//    }

    /*****************************************************************************************
     * 1.audio service dbus not start
     * 2.audio device not start
     * ****************************************************************************************/
    if (m_qvplayer->state() == Vlc::Stopped  || (!isDevValid() &&  m_qvplayer->time() == 0)) {
        //reopen data
        m_qvmedia->initMedia(m_ActiveMeta.localPath, true, m_qvinstance);
        m_qvplayer->open(m_qvmedia);
        m_qvplayer->setTime(m_ActiveMeta.offset);
    }

    if (m_fadeOutAnimation) {
        setFadeInOutFactor(1.0);
        m_fadeOutAnimation->stop();
    }

    qDebug() << "resume top";
    //增大音乐自动开始播放时间，给setposition留足空间
//    QTimer::singleShot(100, this, [ = ]() {
    m_qvplayer->play();
//    emit playbackStatusChanged(status());
//    });

    if (m_fadeInOut && m_fadeInAnimation->state() != QPropertyAnimation::Running) {
        m_fadeInAnimation->setEasingCurve(QEasingCurve::InCubic);
        m_fadeInAnimation->setStartValue(0.1000);
        m_fadeInAnimation->setEndValue(1.0000);
        m_fadeInAnimation->setDuration(sFadeInOutAnimationDuration);
        m_fadeInAnimation->start();
    }
}

void Player::pause()
{
    /*--------suspend--------*/
//    d->ioPlayer->suspend();

    if (m_fadeInAnimation) {
        m_fadeInAnimation->stop();
    }

    if (m_fadeInOut && m_fadeOutAnimation->state() != QPropertyAnimation::Running) {

        m_fadeOutAnimation->setEasingCurve(QEasingCurve::OutCubic);
        m_fadeOutAnimation->setStartValue(1.0000);
        m_fadeOutAnimation->setEndValue(0.1000);
        m_fadeOutAnimation->setDuration(sFadeInOutAnimationDuration);
        m_fadeOutAnimation->start();
        connect(m_fadeOutAnimation, &QPropertyAnimation::finished,
        this, [ = ]() {
            m_qvplayer->pause();
            QTimer::singleShot(50, this, [ = ]() {
                setFadeInOutFactor(1.0);
            });
        });
    } else {
        m_qvplayer->pause();
        setFadeInOutFactor(1.0);
    }
}

void Player::pauseNow()
{
    m_qvplayer->pause();
}

void Player::playPreMeta()
{
    if (m_MetaList.size() > 0) {
        //播放模式todo
        int index = 0;
        for (int i = 0; i < m_MetaList.size(); i++) {
            if (m_MetaList.at(i).hash == m_ActiveMeta.hash) {
                index = i;
                break;
            }
        }
        //根据播放模式确定下一首
        switch (m_mode) {
        case RepeatAll:
        case RepeatSingle: {
            if (index == 0) {
                index = m_MetaList.size() - 1;
            } else {
                index--;
            }
            break;
        }
        case Shuffle: {
            QTime time;
            time = QTime::currentTime();
            qsrand(static_cast<uint>((time.msec() + time.second() * 1000)));
            index = qrand() % m_MetaList.size();
            break;
        }
        default: {
            if (index == 0) {
                index = m_MetaList.size() - 1;
            } else {
                index--;
            }
            break;
        }
        }

        setActiveMeta(m_MetaList.at(index));
        playMeta(m_ActiveMeta);
    }
}

void Player::playNextMeta(bool isAuto)
{
    if (m_MetaList.size() > 0) {
        int index = 0;
        for (int i = 0; i < m_MetaList.size(); i++) {
            if (m_MetaList.at(i).hash == m_ActiveMeta.hash) {
                index = i;
                break;
            }
        }

        switch (m_mode) {
        case RepeatAll: {
            if (index == (m_MetaList.size() - 1)) {
                index = 0;
            } else {
                index++;
            }
            break;
        }
        case RepeatSingle: {
            if (!isAuto) {
                if (index == (m_MetaList.size() - 1)) {
                    index = 0;
                } else {
                    index++;
                }
            }
            break;
        }
        case Shuffle: {
            QTime time;
            time = QTime::currentTime();
            qsrand(static_cast<uint>((time.msec() + time.second() * 1000)));
            index = qrand() % m_MetaList.size();
            break;
        }
        default: {
            if (index == (m_MetaList.size() - 1)) {
                index = 0;
            } else {
                index++;
            }
            break;
        }
        }

        setActiveMeta(m_MetaList.at(index));
        playMeta(m_ActiveMeta);
    }
}

void Player::clearPlayList()
{
    m_MetaList.clear();
}

void Player::playListAppendMeta(MediaMeta meta)
{
    m_MetaList.append(meta);
}

void Player::setPlayList(QList<MediaMeta> list)
{
    m_MetaList = list;
}

QList<MediaMeta> *Player::getPlayList()
{
    return &m_MetaList;
}

MprisPlayer *Player::getMpris() const
{
    return m_pMpris;
}

void Player::setCurrentPlayListHash(QString hash)
{
    m_currentPlayListHash = hash;
}

QString Player::getCurrentPlayListHash()
{
    return m_currentPlayListHash;
}

void Player::stop()
{
    m_qvplayer->pause();
    setActiveMeta(MediaMeta());//清除当前播放音乐；
//    m_activeMeta.clear(); //清除当前播放音乐；
    m_qvplayer->stop();
}
VlcMediaPlayer *Player::core()
{
    return m_qvplayer;
}


Player::PlaybackStatus Player::status()
{
    Vlc::State  status = m_qvplayer->state();

    if (status == Vlc::Playing) {
        return PlaybackStatus::Playing;
    } else if (status == Vlc::Paused) {
        return PlaybackStatus::Paused;
    } else if (status == Vlc::Stopped || status == Vlc::Idle) {
        return PlaybackStatus::Stopped;
    } else {
        return PlaybackStatus::InvalidPlaybackStatus;
    }
}

MediaMeta Player::activeMeta()
{
    return m_ActiveMeta;
}

QIcon Player::playingIcon()
{
    return m_playingIcon;
}

//!
//! \brief canControl
//! Always be true
bool Player::canControl() const
{
    return m_canControl;
}

qlonglong Player::position() const
{
    return m_qvplayer->time();
}

int Player::volume() const
{
    return static_cast<int>(m_volume);
}

Player::PlaybackMode Player::mode() const
{
    return  m_mode;
}

bool Player::muted()
{
    return this->isMusicMuted();
}

qint64 Player::duration() const
{
    if (m_ActiveMeta.localPath.isEmpty()) {
        return 0;
    }
    return  m_qvplayer->length();
}

double Player::fadeInOutFactor() const
{
    return m_fadeInOutFactor;
}

bool Player::fadeInOut() const
{
    return m_fadeInOut;
}

bool Player::playOnLoaded() const
{
    return m_playOnLoad;
}

void Player::setCanControl(bool canControl)
{
    qCritical() << "Never Changed this" << canControl;
}


void Player::setPosition(qlonglong position)
{
//    if (m_activeMeta.isNull()) {
//        return;
//    }

    if (m_qvplayer->length() == m_ActiveMeta.length) {
        return m_qvplayer->setTime(position);
    } else {
        m_qvplayer->setTime(position + m_ActiveMeta.offset);
    }
}

void Player::setMode(Player::PlaybackMode mode)
{
    m_mode = mode;
}

void Player::setVolume(int volume)
{
    if (volume > 100) {
        volume = 100;
    }
    if (volume < 0) {
        volume = 0;
    }
    m_volume = volume;

    setMusicVolume((volume + 0.1) / 100.0);//设置到dbus的音量必须大1，设置才会生效
}

void Player::setMuted(bool mute)
{
    setMusicMuted(mute);
}

void Player::setActiveMeta(const MediaMeta &meta)
{
    m_ActiveMeta = meta;
}

void Player::setLocalMuted(bool muted)
{
    m_qvplayer->audio()->setMute(muted);
    if (isValidDbusMute()) {
        QDBusInterface ainterface("com.deepin.daemon.Audio", m_sinkInputPath,
                                  "com.deepin.daemon.Audio.SinkInput",
                                  QDBusConnection::sessionBus());
        if (!ainterface.isValid()) {
            return ;
        }

        //调用设置音量
        ainterface.call(QLatin1String("SetMute"), muted);
    }
}

void Player::setDbusMuted(bool muted)
{
    Q_UNUSED(muted)
    if (isValidDbusMute()) {
        QDBusInterface ainterface("com.deepin.daemon.Audio", m_sinkInputPath,
                                  "com.deepin.daemon.Audio.SinkInput",
                                  QDBusConnection::sessionBus());
        if (!ainterface.isValid()) {
            return ;
        }
        //调用设置音量
        if (MusicSettings::value("base.play.mute").toBool() !=  m_qvplayer->audio()->getMute())
            ainterface.call(QLatin1String("SetMute"), MusicSettings::value("base.play.mute").toBool());
    }
}

void Player::setFadeInOutFactor(double fadeInOutFactor)
{
    m_fadeInOutFactor = fadeInOutFactor;
    m_qvplayer->equalizer()->blockSignals(true);
    m_qvplayer->equalizer()->setPreamplification(static_cast<float>(12 * m_fadeInOutFactor));
    m_qvplayer->equalizer()->blockSignals(false);
}

void Player::setFadeInOut(bool fadeInOut)
{
    m_fadeInOut = fadeInOut;
}

void Player::setPlayOnLoaded(bool playOnLoaded)
{
    m_playOnLoad = playOnLoaded;
}

void Player::musicFileMiss()
{
}

void Player::setEqualizer(bool enabled, int curIndex, QList<int> indexbaud)
{
    if (enabled) {
        //非自定义模式时
        if (curIndex > 0) {
            m_qvplayer->equalizer()->loadFromPreset(uint(curIndex - 1));
            //设置放大值
            m_qvplayer->equalizer()->setPreamplification(m_qvplayer->equalizer()->preamplification());
            for (int i = 0 ; i < 10; i++) {
                //设置频率值
                m_qvplayer->equalizer()->setAmplificationForBandAt(m_qvplayer->equalizer()->amplificationForBandAt(uint(i)), uint(i));
            }
        } else {
            if (indexbaud.size() == 0) {
                return;
            } else {
                m_qvplayer->equalizer()->setPreamplification(indexbaud.at(0));
                for (int i = 1; i < 11; i++) {
                    m_qvplayer->equalizer()->setAmplificationForBandAt(indexbaud.at(i), uint(i - 1));
                }
            }
        }
    }
}

void Player::setEqualizerEnable(bool enable)
{
    m_qvplayer->equalizer()->setEnabled(enable);
}

void Player::setEqualizerpre(int val)
{
    m_qvplayer->equalizer()->setPreamplification(val);
}

void Player::setEqualizerbauds(int index, int val)
{
    m_qvplayer->equalizer()->setAmplificationForBandAt(uint(val), uint(index));
}

void Player::setEqualizerCurMode(int curIndex)
{
    //非自定义模式时
    if (curIndex != 0) {
        m_qvplayer->equalizer()->loadFromPreset(uint(curIndex - 1));
        //设置放大值
        m_qvplayer->equalizer()->setPreamplification(m_qvplayer->equalizer()->preamplification());
        for (int i = 0 ; i < 10; i++) {
            //设置频率值
            m_qvplayer->equalizer()->setAmplificationForBandAt(m_qvplayer->equalizer()->amplificationForBandAt(uint(i)), uint(i));
        }
    }
}

bool Player::isValidDbusMute()
{
    readSinkInputPath();
    if (!m_sinkInputPath.isEmpty()) {
        QVariant MuteV = DBusUtils::readDBusProperty("com.deepin.daemon.Audio", m_sinkInputPath,
                                                     "com.deepin.daemon.Audio.SinkInput", "Mute");

        return MuteV.isValid();
    }

    return false;
}

void Player::readSinkInputPath()
{
    QVariant v = DBusUtils::readDBusProperty("com.deepin.daemon.Audio", "/com/deepin/daemon/Audio",
                                             "com.deepin.daemon.Audio", "SinkInputs");

    if (!v.isValid())
        return;

    QList<QDBusObjectPath> allSinkInputsList = v.value<QList<QDBusObjectPath> >();

    for (auto curPath : allSinkInputsList) {
        QVariant nameV = DBusUtils::readDBusProperty("com.deepin.daemon.Audio", curPath.path(),
                                                     "com.deepin.daemon.Audio.SinkInput", "Name");

        if (!nameV.isValid() || nameV.toString() != Global::getAppName())
            continue;

        m_sinkInputPath = curPath.path();
        break;
    }
}

bool Player::setMusicVolume(double volume)
{
    if (volume > 1.0) {
        volume = 1.000;
    }
    readSinkInputPath();
    if (!m_sinkInputPath.isEmpty()) {
        QDBusInterface ainterface("com.deepin.daemon.Audio", m_sinkInputPath,
                                  "com.deepin.daemon.Audio.SinkInput",
                                  QDBusConnection::sessionBus());
        if (!ainterface.isValid()) {
            return false;
        }

        //调用设置音量
        ainterface.call(QLatin1String("SetVolume"), volume, false);

        if (qFuzzyCompare(volume, 0.0))
            ainterface.call(QLatin1String("SetMute"), true);
    }

    return false;
}

bool Player::setMusicMuted(bool muted)
{
    readSinkInputPath();
    if (!m_sinkInputPath.isEmpty()) {
        QDBusInterface ainterface("com.deepin.daemon.Audio", m_sinkInputPath,
                                  "com.deepin.daemon.Audio.SinkInput",
                                  QDBusConnection::sessionBus());
        if (!ainterface.isValid()) {
            return false;
        }

        //调用设置音量
        ainterface.call(QLatin1String("SetMute"), muted);
        Q_EMIT mutedChanged(muted);
    }

    return false;
}

bool Player::isMusicMuted()
{
    readSinkInputPath();
    if (!m_sinkInputPath.isEmpty()) {
        QVariant MuteV = DBusUtils::readDBusProperty("com.deepin.daemon.Audio", m_sinkInputPath,
                                                     "com.deepin.daemon.Audio.SinkInput", "Mute");

        if (!MuteV.isValid()) {
            return false;
        }

        return MuteV.toBool();
    }

    return false;
}

bool Player::isDevValid()
{
    readSinkInputPath();
    if (!m_sinkInputPath.isEmpty()) {
        QVariant MuteV = DBusUtils::readDBusProperty("com.deepin.daemon.Audio", m_sinkInputPath,
                                                     "com.deepin.daemon.Audio.SinkInput", "Mute");
        return MuteV.isValid();
    }

    return false;
}

void Player::initConnection()
{
    m_timer = new QTimer(this);
    m_timer->setInterval(250);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(changePicture()));
    connect(m_qvplayer, &VlcMediaPlayer::timeChanged,
    this, [ = ](qint64 position) {
        Q_EMIT positionChanged(position - m_ActiveMeta.offset,  m_ActiveMeta.length, 1);
    });

    //vlc stateChanged
    connect(m_qvmedia, &VlcMedia::stateChanged,
    this, [ = ](Vlc::State status) {

        switch (status) {

        case Vlc::Idle: {
            /**************************************
             * if settings is mute ,then setmute to dbus
             * ************************************/
            if (MusicSettings::value("base.play.mute").toBool())
                setMusicMuted(true);
            break;
        }
        case Vlc::Opening: {
            emit signalMediaMetaChanged();
            break;
        }
        case Vlc::Buffering: {

            break;
        }
        case Vlc::Playing: {
            emit signalPlaybackStatusChanged(Player::Playing);
            if (!m_timer->isActive()) {
                m_timer->start();
            }
            break;
        }
        case Vlc::Paused: {
            emit signalPlaybackStatusChanged(Player::Paused);
            m_timer->stop();
            break;
        }
        case Vlc::Stopped: {
            emit signalPlaybackStatusChanged(Player::Stopped);
            m_timer->stop();
            break;
        }
        case Vlc::Ended: {
//            playNextMeta();//just sync with Vlc::Ended
            break;
        }
//        case Vlc::Error: {
//            if (!m_activeMeta.isNull() /*&& !QFile::exists(activeMeta->localPath)*/) {
//                MetaPtrList removeMusicList;
//                removeMusicList.append(m_activeMeta);
//                m_curPlaylist->removeMusicList(removeMusicList);
//                Q_EMIT mediaError(m_activePlaylist, m_activeMeta, Player::ResourceError);
//            }
//            break;
//        }

        }
    });



    connect(m_qvplayer, &VlcMediaPlayer::end,
    this, [ = ]() {
        playNextMeta(true);//just sync with Vlc::Ended
    });


    connect(m_qvplayer->audio(), &VlcAudio::muteChanged,
    this, [ = ](bool mute) {
        if (isDevValid()) {
            Q_EMIT mutedChanged(mute);
        } else {
            qDebug() << "device does not start";
        }
    });
}

void Player::initMpris()
{
    m_pMpris =  new MprisPlayer();
    m_pMpris->setServiceName("DeepinMusic");

    m_pMpris->setSupportedMimeTypes(supportedMimeTypes());
    m_pMpris->setSupportedUriSchemes(QStringList() << "file");
    m_pMpris->setCanQuit(true);
    m_pMpris->setCanRaise(true);
    m_pMpris->setCanSetFullscreen(false);
    m_pMpris->setHasTrackList(true);
    // setDesktopEntry: see https://specifications.freedesktop.org/mpris-spec/latest/Media_Player.html#Property:DesktopEntry for more
    m_pMpris->setDesktopEntry("deepin-music");
    m_pMpris->setIdentity("Deepin Music Player");

    m_pMpris->setCanControl(true);
    m_pMpris->setCanPlay(true);
    m_pMpris->setCanGoNext(true);
    m_pMpris->setCanGoPrevious(true);
    m_pMpris->setCanPause(true);
    m_pMpris->setCanSeek(true);
}

void Player::changePicture()
{
    //切换播放状态动态图
    int curCount = m_playingCount % 4;
    m_playingCount = curCount + 1;
    switch (curCount) {
    case 0:
        m_playingIcon = QIcon::fromTheme("music_play1");
        break;
    case 1:
        m_playingIcon = QIcon::fromTheme("music_play2");
        break;
    case 2:
        m_playingIcon = QIcon::fromTheme("music_play3");
        break;
    case 3:
        m_playingIcon = QIcon::fromTheme("music_play4");
        break;
    default:
        m_playingIcon = QIcon::fromTheme("music_play1");
        break;
    }
    emit signalUpdatePlayingIcon();
}

void initMiniTypes()
{
    //black list
    QHash<QString, bool> suffixBlacklist;
    suffixBlacklist.insert("m3u", true);
    suffixBlacklist.insert("mid", true);
    suffixBlacklist.insert("midi", true);
    suffixBlacklist.insert("imy", true);
    suffixBlacklist.insert("xmf", true);
    suffixBlacklist.insert("mp4", true);
    suffixBlacklist.insert("mkv", true);
    suffixBlacklist.insert("avi", true);
    suffixBlacklist.insert("mpeg4", true);
    suffixBlacklist.insert("3gp", true);
    suffixBlacklist.insert("flv", true);
    suffixBlacklist.insert("ass", true);

    QHash<QString, bool> suffixWhitelist;
    suffixWhitelist.insert("cue", true);

    QStringList  mimeTypeWhiteList;
    mimeTypeWhiteList << "application/vnd.ms-asf";

    QMimeDatabase mdb;
    for (auto &mt : mdb.allMimeTypes()) {
        if (mt.name().startsWith("audio/") /*|| mt.name().startsWith("video/")*/) {
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


