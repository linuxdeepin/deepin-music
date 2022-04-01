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
#include "vlc/Error.h"
#include "vlc/Common.h"
#include "vlc/Enums.h"
#include "vlc/Instance.h"
#include "vlc/Media.h"
#include "vlc/MediaPlayer.h"
#include "vlc/Equalizer.h"
#include "databaseservice.h"
#include "vlc/cda.h"
#include "playerbase.h"
#include "qtplayer.h"
#include "vlcplayer.h"

#include "core/musicsettings.h"
#include <DPushButton>
#include <DPalette>
#include <QThread>

#include "commonservice.h"

DCORE_USE_NAMESPACE

static const int sFadeInOutAnimationDuration = 900; //ms
static int INT_LAST_PROGRESS_FLAG = 1;

QStringList mimeTypes()
{
    QStringList  mimeTypeWhiteList;
    mimeTypeWhiteList << "application/vnd.ms-asf";

    QMimeDatabase mdb;
    for (auto &mt : mdb.allMimeTypes()) {
        if (mt.name().startsWith("audio/") /*|| mt.name().startsWith("video/")*/) {
            mimeTypeWhiteList << mt.name();
        }
        if (mt.name().startsWith("video/")) {
            mimeTypeWhiteList << mt.name();
        }

        if (mt.name().startsWith("application/octet-stream")) {
            mimeTypeWhiteList << mt.name();
        }
    }
    return mimeTypeWhiteList;
}

Player::Player(QObject *parent) : QObject(parent)
{
    qRegisterMetaType<Player::Error>();
    qRegisterMetaType<Player::PlaybackStatus>();

    m_fadeOutAnimation = new QPropertyAnimation(this, "fadeInOutFactor", this);
    m_fadeInAnimation = new QPropertyAnimation(this, "fadeInOutFactor", this);

    //支持格式列表,.cda格式自动加载，无需放在支持列表中
    if (Global::playbackEngineType() == 1) {
        m_supportedSuffix << "*.aac"
                          << "*.amr"
                          << "*.wav"
                          << "*.ogg"
                          << "*.ape"
                          << "*.mp3"
                          << "*.flac"
                          << "*.wma"
                          << "*.m4a"
                          << "*.ac3"
                          << "*.voc"
                          << "*.aiff";
        m_supportedSuffixStr  << "aac"
                              << "amr"
                              << "wav"
                              << "ogg"
                              << "ape"
                              << "mp3"
                              << "flac"
                              << "wma"
                              << "m4a"
                              << "ac3"
                              << "voc"
                              << "aiff";
    } else {
        m_supportedSuffix << "*.wav"
                          << "*.ogg"
                          << "*.mp3"
                          << "*.flac";
        m_supportedSuffixStr << "wav"
                             << "ogg"
                             << "mp3"
                             << "flac";
    }

    initMpris();
    m_volume = MusicSettings::value("base.play.volume").toInt();
    /**
     *  初始获取音量，避免从dbus获取音量时，获取的值为初始值0
     * */
    m_mpris->setVolume((float)(m_volume / 100.0));
    setFadeInOut(MusicSettings::value("base.play.fade_in_out").toBool());
    // 根据记录初始化播放模式
    m_mode = static_cast<PlaybackMode>(MusicSettings::value("base.play.playmode").toInt());

    m_currentPlayListHash = MusicSettings::value("base.play.last_playlist").toString(); //上一次的页面
    initPlayer();
}

void Player::init()
{
    m_basePlayer->init();
    if (nullptr == m_pDBus) {
        m_pDBus = new QDBusInterface("org.freedesktop.login1", "/org/freedesktop/login1",
                                     "org.freedesktop.login1.Manager", QDBusConnection::systemBus());
        connect(m_pDBus, SIGNAL(PrepareForSleep(bool)), this, SLOT(onSleepWhenTaking(bool)));
    }
}

QStringList Player::supportedSuffixList() const
{
    return m_supportedSuffix;
}

QStringList Player::supportedSuffixStrList() const
{
    return m_supportedSuffixStr;
}

void Player::releasePlayer()
{
    stop(false);
    //释放资源
    if (m_basePlayer) {
        m_basePlayer->release();
    }
}

void Player::sortMetas(const QVector<QString> &metaHashs)
{
    auto t_MetaList = m_MetaList;
    m_MetaList.clear();
    // 根据hash排序
    for (auto hash : metaHashs) {
        for (int i = 0; i < t_MetaList.size(); ++i) {
            if (t_MetaList[i].hash == hash) {
                m_MetaList.append(t_MetaList[i]);
                t_MetaList.removeAt(i);
                break;
            }
        }
    }
}

Player::~Player()
{
    DataBaseService::getInstance()->updateMetasforPlayerList();
    releasePlayer();
}

void Player::playMeta(MediaMeta meta)
{
    if (meta.hash != "") {
        if (meta.mmType != MIMETYPE_CDA) {
            int acint = access(meta.localPath.toStdString().c_str(), R_OK);
            bool empty = QFileInfo(m_ActiveMeta.localPath).dir().path().isEmpty();
            bool suffixFlag = m_supportedSuffixStr.contains(QFileInfo(meta.localPath).suffix().toLower());
            if (acint != 0 || empty || !suffixFlag) {
                //文件不存在提示
                emit signalPlaybackStatusChanged(Player::Paused);
                playNextMeta(true);
                INT_LAST_PROGRESS_FLAG = 0;
                return;
            }
        }

        // 防止淡入淡出时切换歌曲
        if (m_fadeInOut) {
            m_fadeOutAnimation->stop();
            setFadeInOutFactor(1.0);
        }

        m_ActiveMeta = meta;
        setActiveMeta(meta);
        m_basePlayer->setMediaMeta(meta);
        m_basePlayer->play();

        //延迟设置进度
        if (INT_LAST_PROGRESS_FLAG && m_ActiveMeta.hash == meta.hash) {
            m_basePlayer->pause();
            qint64 lastOffset = m_ActiveMeta.offset;
            QTimer::singleShot(100, this, [ = ]() {//为了记录进度条生效，在加载的时候让音乐播放100ms
                qDebug() << "seek last position:" << lastOffset;
                m_basePlayer->setTime(lastOffset);
                m_basePlayer->play();
            });
        }
        // 开始后点击播放另一首哥播放保证错误
        INT_LAST_PROGRESS_FLAG = 0;
        /*************************
         * mute to dbus
         * ***********************/

        QTimer::singleShot(100, this, [ = ]() {
            setDbusMuted();
        });

        DRecentData data;
        data.appName = Global::getAppName();
        data.appExec = "deepin-music";
        DRecentManager::addItem(meta.localPath, data);


        this->resetDBusMpris(meta);
        m_mpris->setLoopStatus(Mpris::Playlist);
        m_mpris->setPlaybackStatus(Mpris::Playing);
        m_mpris->setVolume((float)(getVolume() / 100.0));//只获取前两位小数点

        //设置音乐播放
        emit signalPlaybackStatusChanged(Player::Playing);
        // 保存播放歌曲信息
        MusicSettings::setOption("base.play.last_meta", m_ActiveMeta.hash);
    } else {
        //设置音乐播放
        emit signalPlaybackStatusChanged(Player::Paused);
    }
}

void Player::resume()
{
    if (m_ActiveMeta.localPath.isEmpty()) {
        Player::getInstance()->forcePlayMeta();//播放列表第一首歌
        return;
    }

    bool empty = QFileInfo(m_ActiveMeta.localPath).dir().path().isEmpty();
    if (empty && m_ActiveMeta.mmType != MIMETYPE_CDA) {//光盘弹出时，有可能歌曲路径还在，不需要再播放。
        return ;
    }

    /*****************************************************************************************
     * 1.audio service dbus not start
     * 2.audio device not start
     * ****************************************************************************************/
    if (m_basePlayer->state() == PlayerBase::Stopped  || (!isValidDbusMute() &&  m_basePlayer->time() == 0)) {
        //reopen data
        m_basePlayer->setMediaMeta(m_ActiveMeta);
    }

    if (m_fadeInOut) {
        setFadeInOutFactor(0.1);
        m_fadeOutAnimation->stop();
    }

    qDebug() << "resume top";
    m_basePlayer->play();
    if (m_fadeInOut && m_fadeInAnimation->state() != QPropertyAnimation::Running) {
        m_fadeInAnimation->setEasingCurve(QEasingCurve::InCubic);
        m_fadeInAnimation->setStartValue(0.1000);
        m_fadeInAnimation->setEndValue(1.0000);
        m_fadeInAnimation->setDuration(sFadeInOutAnimationDuration);
        m_fadeInAnimation->start();
    }

    this->resetDBusMpris(m_ActiveMeta);
    m_mpris->setLoopStatus(Mpris::Playlist);
    m_mpris->setPlaybackStatus(Mpris::Playing);

    m_mpris->setVolume((float)(getVolume() / 100.0));//只获取前两位小数点

    //设置音乐播放
    emit signalPlaybackStatusChanged(Player::Playing);
}

void Player::pause()
{
    if (m_fadeInOut) {
        m_fadeInAnimation->stop();
    }

    if (m_fadeInOut && m_fadeOutAnimation->state() != QPropertyAnimation::Running) {
        m_fadeOutAnimation->setEasingCurve(QEasingCurve::OutCubic);
        m_fadeOutAnimation->setStartValue(1.0000);
        m_fadeOutAnimation->setEndValue(0.1000);
        m_fadeOutAnimation->setDuration(sFadeInOutAnimationDuration * 2);
        m_fadeOutAnimation->start();
        connect(m_fadeOutAnimation, &QPropertyAnimation::finished,
        this, [ = ]() {
            m_basePlayer->pause();
            setFadeInOutFactor(1.0);
        });
    } else {
        m_basePlayer->pause();
        setFadeInOutFactor(1.0);
    }

    //设置音乐播放
    emit signalPlaybackStatusChanged(Player::Paused);
}

void Player::pauseNow()
{
    m_basePlayer->pause();
    //设置音乐播放
    emit signalPlaybackStatusChanged(Player::Paused);
}

void Player::playPreMeta()
{
    QList<MediaMeta> curMetaList;
    for (int i = 0; i < m_MetaList.size(); i++) {
        if ((QFile::exists(m_MetaList[i].localPath) && m_supportedSuffixStr.contains(QFileInfo(m_MetaList[i].localPath).suffix().toLower())) || m_MetaList[i].mmType == MIMETYPE_CDA)
            curMetaList.append(m_MetaList[i]);
    }
    if (curMetaList.size() > 0) {
        //播放模式todo
        int index = 0;
        for (int i = 0; i < curMetaList.size(); i++) {
            if (curMetaList.at(i).hash == m_ActiveMeta.hash) {
                index = i;
                break;
            }
        }
        //根据播放模式确定下一首
        switch (m_mode) {
        case RepeatAll:
        case RepeatSingle: {
            if (index == 0) {
                index = curMetaList.size() - 1;
            } else {
                index--;
            }
            break;
        }
        case Shuffle: {
            // 多个随机处理
            if (curMetaList.size() > 1) {
                QTime time;
                time = QTime::currentTime();
                int curIndex = static_cast<int>(QRandomGenerator::global()->bounded(time.msec() + time.msec() * 1000)) % curMetaList.size();
                // 随机相同时特殊处理
                if (curIndex == index) {
                    if (curIndex == 0) {
                        index = 1;
                    } else if (curIndex == curMetaList.size() - 1) {
                        index = 0;
                    } else {
                        index = curIndex - 1;
                    }
                } else {
                    index = curIndex;
                }
            } else {
                index = 0;
            }
            break;
        }
        default: {
            if (index == 0) {
                index = curMetaList.size() - 1;
            } else {
                index--;
            }
            break;
        }
        }
        m_ActiveMeta = curMetaList.at(index);
        playMeta(m_ActiveMeta);
    }
}

void Player::playNextMeta(bool isAuto)
{
    QList<MediaMeta> curMetaList;
    for (int i = 0; i < m_MetaList.size(); i++) {
        if ((QFile::exists(m_MetaList[i].localPath) && m_supportedSuffixStr.contains(QFileInfo(m_MetaList[i].localPath).suffix().toLower())) || m_MetaList[i].mmType == MIMETYPE_CDA)
            curMetaList.append(m_MetaList[i]);
    }
    if (curMetaList.size() > 0) {
        int index = 0;
        for (int i = 0; i < curMetaList.size(); i++) {
            if (curMetaList.at(i).hash == m_ActiveMeta.hash) {
                index = i;
                break;
            }
        }

        switch (m_mode) {
        case RepeatAll: {
            if (index == (curMetaList.size() - 1)) {
                index = 0;
            } else {
                index++;
            }
            break;
        }
        case RepeatSingle: {
            if (!isAuto) {
                if (index == (curMetaList.size() - 1)) {
                    index = 0;
                } else {
                    index++;
                }
            }
            break;
        }
        case Shuffle: {
            //多个随机处理
            if (curMetaList.size() > 1) {
                QTime time;
                time = QTime::currentTime();
                int curIndex = static_cast<int>(QRandomGenerator::global()->bounded(time.msec() + time.msec() * 1000)) % curMetaList.size();
                // 随机相同时特殊处理
                if (curIndex == index) {
                    if (curIndex == 0) {
                        index = 1;
                    } else if (curIndex == curMetaList.size() - 1) {
                        index = 0;
                    } else {
                        index = curIndex + 1;
                    }
                } else {
                    index = curIndex;
                }
            } else {
                index = 0;
            }

            //qsrand(static_cast<uint>((time.msec() + time.second() * 1000)));
            //index = qrand() % m_MetaList.size();
            break;
        }
        default: {
            if (index == (curMetaList.size() - 1)) {
                index = 0;
            } else {
                index++;
            }
            break;
        }
        }
        m_ActiveMeta = curMetaList.at(index);
        playMeta(m_ActiveMeta);
    } else {
        stop();
    }
}

void Player::clearPlayList()
{
    QStringList list;
    for (MediaMeta meta : m_MetaList) {
        list << meta.hash;
    }
    playRmvMeta(list);
}

void Player::playRmvMeta(const QStringList &metalistToDel)
{
    if (m_MetaList.size() == 0) {
        return;
    }

    // 去除无法播放歌曲
    QList<MediaMeta> curMetaList;
    for (int i = 0; i < m_MetaList.size(); i++) {
        if ((QFile::exists(m_MetaList[i].localPath) && m_supportedSuffixStr.contains(QFileInfo(m_MetaList[i].localPath).suffix().toLower())) || m_MetaList[i].mmType == MIMETYPE_CDA)
            curMetaList.append(m_MetaList[i]);
    }

    int index = 0;
    QString nextPlayHash;
    // 记录切换歌单之前播放状态
    PlaybackStatus preStatus = status();
    for (int i = 0; i < curMetaList.size(); i++) {
        if (curMetaList.at(i).hash == m_ActiveMeta.hash) {
            index = i;
            break;
        }
    }

    // 如果要删除的包含了正在播放的歌曲则停止播放
    if (metalistToDel.contains(m_ActiveMeta.hash)) {
        stop();
        MediaMeta metaToPlay;
        metaToPlay.hash = "";
        // 从当前播放的位置寻找下一首
        for (int i = (index + 1); i <= curMetaList.size(); i++) {
            if (i == curMetaList.size()) {
                i = 0;
            }
            if (i == index) {
                break;
            }
            // 如果下一首不在删除队列中，则找到了下一首
            if (!metalistToDel.contains(curMetaList.at(i).hash)) {
                metaToPlay = curMetaList.at(i);
                break;
            }
            // 防止死循环
            if (curMetaList.size() == 1) {
                break;
            }
        }
        if (!metaToPlay.hash.isEmpty()) {
            setActiveMeta(metaToPlay);
            if (preStatus == Player::PlaybackStatus::Playing) {
                playMeta(m_ActiveMeta);
            }
        }
        removeMeta(metalistToDel);
    } else {
        // 不包含正在播放的歌曲，直接删除
        removeMeta(metalistToDel);
    }
}

void Player::removeMeta(const QStringList &metalistToDel)
{
    for (QString delstr : metalistToDel) {
        for (int i = (m_MetaList.size() - 1); i >= 0 ; i--) {
            if (m_MetaList[i].hash == delstr) {
                emit signalPlayQueueMetaRemove(delstr);
                m_MetaList.removeAt(i);
                break;
            }
        }
    }
    // 当前播放队列没有数据，应用左侧不显示动态图
    if (m_MetaList.size() == 0) {
        stop();
        m_currentPlayListHash = "";
        emit signalPlayListChanged();
        // 刷新动态图图标
        emit signalUpdatePlayingIcon();
        MusicSettings::setOption("base.play.last_meta", "");
        MusicSettings::setOption("base.play.last_playlist", "");
    } else {
        // 自定义歌单时才需要查询数量
        if (m_currentPlayListHash != "album"
                && m_currentPlayListHash != "artist"
                && m_currentPlayListHash != "albumResult"
                && m_currentPlayListHash != "artistResult") {
            if (DataBaseService::getInstance()->getPlaylistSongCount(m_currentPlayListHash) == 0) {
                m_currentPlayListHash = "all";
                // 刷新动态图图标
                emit signalUpdatePlayingIcon();
                MusicSettings::setOption("base.play.last_meta", m_ActiveMeta.hash);
                MusicSettings::setOption("base.play.last_playlist", m_currentPlayListHash);
            }
        }
    }
    emit signalPlaylistCountChange();
}

void Player::playListAppendMeta(const MediaMeta &meta)
{
    // 防止重复添加
    if (!m_MetaList.contains(meta)) {
        m_MetaList.append(meta);
        emit signalPlaylistCountChange();
    }
}

void Player::setPlayList(const QList<MediaMeta> &list)
{
    m_MetaList = list;
    // 当前播放队列没有数据，应用左侧不显示动态图
    if (m_MetaList.size() == 0) {
        m_currentPlayListHash = "";
    }
    emit signalUpdatePlayingIcon();
    emit signalPlaylistCountChange();
}

QList<MediaMeta> *Player::getPlayList()
{
    return &m_MetaList;
}

QList<MediaMeta> Player::getCdaPlayList()
{
    return m_basePlayer->getCdaMetaInfo();
}

MprisPlayer *Player::getMpris() const
{
    return m_mpris;
}

void Player::reloadMetaList()
{
    m_MetaList = DataBaseService::getInstance()->customizeMusicInfos("play");
    if (!m_MetaList.isEmpty()) m_currentPlayListHash = "all";
}

void Player::setCurrentPlayListHash(QString hash, bool reloadMetaList)
{
    m_currentPlayListHash = hash;
    if (reloadMetaList) {
        m_MetaList.clear();
        if (hash == "all" || hash == "album" || hash == "artist") {
            m_MetaList = DataBaseService::getInstance()->allMusicInfos();
        } else {
            m_MetaList = DataBaseService::getInstance()->customizeMusicInfos(hash);
        }
    }
    // 当前播放队列没有数据，应用左侧不显示动态图
    if (m_MetaList.size() == 0 && hash != "all" && hash != "album" && hash != "artist") {
        m_currentPlayListHash = "";
    }
    // 保存播放歌曲及歌单信息
    MusicSettings::setOption("base.play.last_playlist", m_currentPlayListHash);
    emit signalUpdatePlayingIcon();
    emit signalPlaylistCountChange();
}

QString Player::getCurrentPlayListHash()
{
    return m_currentPlayListHash;
}

void Player::stop(bool emitSignal)
{
    if (emitSignal) {
        //play停止后，发送清空当前波形图的信号
        emit signalMediaStop("");//不用当前的参数
        if (m_basePlayer) {
            m_basePlayer->pause();
            setActiveMeta(MediaMeta());//清除当前播放音乐；
            m_basePlayer->stop();
        }

        QVariantMap metadata;
        m_mpris->setMetadata(metadata);

        emit signalPlaybackStatusChanged(Player::Stopped);
    } else {
        if (m_basePlayer) {
            m_basePlayer->pause();
            m_basePlayer->stop();
        }
    }

}

Player::PlaybackStatus Player::status()
{
    PlayerBase::PlayState  status = m_basePlayer->state();

    if (status == PlayerBase::Playing) {
        return PlaybackStatus::Playing;
    } else if (status == PlayerBase::Paused) {
        return PlaybackStatus::Paused;
    } else if (status == PlayerBase::Stopped || status == PlayerBase::Idle) {
        return PlaybackStatus::Stopped;
    } else {
        return PlaybackStatus::InvalidPlaybackStatus;
    }
}

MediaMeta Player::getActiveMeta()
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
//bool Player::canControl() const
//{
//    return m_canControl;
//}

qlonglong Player::position()
{
    return m_basePlayer->time();
}

int Player::getVolume() const
{
    return static_cast<int>(m_volume);
}

Player::PlaybackMode Player::mode() const
{
    return  m_mode;
}

bool Player::getMuted()
{
    return MusicSettings::value("base.play.mute").toBool();
}

qint64 Player::duration()
{
    if (m_ActiveMeta.localPath.isEmpty()) {
        return 0;
    }
    return  m_basePlayer->length();
}

double Player::fadeInOutFactor() const
{
    return m_fadeInOutFactor;
}

bool Player::fadeInOut() const
{
    return m_fadeInOut;
}

void Player::initCddTrack()
{
    m_basePlayer->initCddaTrack();
}

//bool Player::playOnLoaded() const
//{
//    return m_playOnLoad;
//}

void Player::setCanControl(bool canControl)
{
    qCritical() << "Never Changed this" << canControl;
}


void Player::setPosition(qlonglong position)
{
    //未加载前设置进度
    if (m_basePlayer->time() == -1) {
        INT_LAST_PROGRESS_FLAG = 1;
        m_ActiveMeta.offset = position;
    }

    if (m_basePlayer->length() == m_ActiveMeta.length) {
        return m_basePlayer->setTime(position);
    } else {
        m_basePlayer->setTime(position);
    }
}

void Player::setMode(Player::PlaybackMode mode)
{
    m_mode = mode;
    // 设置模式后保存
    MusicSettings::setOption("base.play.playmode", m_mode);
}

void Player::setVolume(int volume, bool sync)
{
    if (volume > 100) {
        volume = 100;
    }
    if (volume < 0) {
        volume = 0;
    }

    m_volume = volume;
    if (sync) MusicSettings::setOption("base.play.volume", volume);
    setMuted(volume == 0 ? true : false, sync);

    //+1会造成获取音量值不正确，取消+1
    setMusicVolume((volume) / 100.0);
    emit signalVolumeChanged();
}

void Player::setMuted(bool mute, bool sync)
{
    Q_UNUSED(sync)
    if (!setMusicMuted(mute))  //audio服务未启动
        qDebug() << "audio service not started,mute can not be set";
    //使用本地静音配置
    if (MusicSettings::value("base.play.mute").toBool() != mute)
        MusicSettings::setOption("base.play.mute", mute);
}

void Player::setActiveMeta(const MediaMeta &meta)
{
    m_ActiveMeta = meta;
    emit signalMediaMetaChanged(m_ActiveMeta);
    //保存上一次播放的歌曲
    MusicSettings::setOption("base.play.last_meta", meta.hash);
    MusicSettings::setOption("base.play.last_playlist", m_currentPlayListHash.isEmpty() ? "all" : m_currentPlayListHash);
}

void Player::forcePlayMeta()
{
    qDebug() << "forcePlayMeta in";
    if (m_MetaList.size() == 0) {
        if (DataBaseService::getInstance()->getDelStatus() == true)//判断是否正在删除中
            return;
        QString curHash = (DataBaseService::getInstance()->allMusicInfos().isEmpty() && !getCdaPlayList().isEmpty()) ? "CdaRole" : "all";
        // cd特殊处理
        if (curHash == "CdaRole") {
            // 添加到播放列表
            for (auto meta : getCdaPlayList()) {
                Player::getInstance()->playListAppendMeta(meta);
            }
            setCurrentPlayListHash(curHash, false);
        } else {
            // 更新所有歌曲页面数据
            setCurrentPlayListHash(curHash, true);
        }
        // 通知播放队列刷新
        emit signalPlayListChanged();
        if (m_MetaList.size() == 0)
            return;
    }
    // 如果当前歌曲已设置，则播放当前歌曲
    if (m_ActiveMeta.hash.isEmpty()) {
        playMeta(m_MetaList.first());
    } else {
        playMeta(m_ActiveMeta);
    }
}

void Player::initEqualizerCfg()
{
    setEqualizerEnable(true);
    //读取均衡器使能开关配置
    auto eqSwitch = MusicSettings::value("equalizer.all.switch").toBool();
    //自定义频率
    QList<int > allBauds;
    allBauds.clear();
    allBauds.append(MusicSettings::value("equalizer.all.baud_pre").toInt());
    allBauds.append(MusicSettings::value("equalizer.all.baud_60").toInt());
    allBauds.append(MusicSettings::value("equalizer.all.baud_170").toInt());
    allBauds.append(MusicSettings::value("equalizer.all.baud_310").toInt());
    allBauds.append(MusicSettings::value("equalizer.all.baud_600").toInt());
    allBauds.append(MusicSettings::value("equalizer.all.baud_1K").toInt());
    allBauds.append(MusicSettings::value("equalizer.all.baud_3K").toInt());
    allBauds.append(MusicSettings::value("equalizer.all.baud_6K").toInt());
    allBauds.append(MusicSettings::value("equalizer.all.baud_12K").toInt());
    allBauds.append(MusicSettings::value("equalizer.all.baud_14K").toInt());
    allBauds.append(MusicSettings::value("equalizer.all.baud_16K").toInt());
    //载入当前设置音效
    auto eqCurEffect = MusicSettings::value("equalizer.all.curEffect").toInt();

    if (eqSwitch) {
        setEqualizer(eqSwitch, eqCurEffect, allBauds);
    }
}

void Player::setDbusMuted(bool muted)
{
    Q_UNUSED(muted)
    bool bvalid = isValidDbusMute();
    qDebug() << __FUNCTION__ << bvalid << isMusicMuted();
    if (bvalid && Global::playbackEngineType() == 1) {
        QDBusInterface ainterface("com.deepin.daemon.Audio", m_sinkInputPath,
                                  "com.deepin.daemon.Audio.SinkInput",
                                  QDBusConnection::sessionBus());
        if (!ainterface.isValid()) {
            return ;
        }

        //调用设置音量
        ainterface.call(QLatin1String("SetVolume"), (m_volume) / 100.0, false);//取消+1，保证音量值正确

        if (qFuzzyCompare(m_volume, 0.0))
            ainterface.call(QLatin1String("SetMute"), true);

        //调用设置静音
        bool localmute = MusicSettings::value("base.play.mute").toBool();
        if (localmute != isMusicMuted())
            ainterface.call(QLatin1String("SetMute"), localmute);
    } else if (Global::playbackEngineType() != 1) {
        m_basePlayer->setVolume(m_volume);
        bool localmute = MusicSettings::value("base.play.mute").toBool();
        m_basePlayer->setMute(localmute);
    }
}

void Player::setFadeInOutFactor(double fadeInOutFactor)
{
    m_fadeInOutFactor = fadeInOutFactor;
    m_basePlayer->blockSignals(true);
    m_basePlayer->setPreamplification(static_cast<float>(12 * m_fadeInOutFactor));
    m_basePlayer->blockSignals(false);
}

void Player::setFadeInOut(bool fadeInOut)
{
    m_fadeInOut = fadeInOut;
}

void Player::setEqualizer(bool enabled, int curIndex, QList<int> indexbaud)
{
    if (enabled) {
        //非自定义模式时
        if (curIndex > 0) {
            m_basePlayer->loadFromPreset(uint(curIndex - 1));
            //设置放大值
            m_basePlayer->setPreamplification(m_basePlayer->preamplification());
            for (int i = 0 ; i < 10; i++) {
                //设置频率值
                m_basePlayer->setAmplificationForBandAt(m_basePlayer->amplificationForBandAt(uint(i)), uint(i));
            }
        } else {
            if (indexbaud.size() == 0) {
                return;
            } else {
                m_basePlayer->setPreamplification(indexbaud.at(0));
                for (int i = 1; i < 11; i++) {
                    m_basePlayer->setAmplificationForBandAt(indexbaud.at(i), uint(i - 1));
                }
            }
        }
    }
}

void Player::setEqualizerEnable(bool enable)
{
    m_basePlayer->setEqualizerEnabled(enable);
}

void Player::setEqualizerpre(int val)
{
    m_basePlayer->setPreamplification(val);
}

void Player::setEqualizerbauds(int index, int val)
{
    m_basePlayer->setAmplificationForBandAt(uint(val), uint(index));
}

void Player::setEqualizerCurMode(int curIndex)
{
    //非自定义模式时
    if (curIndex != 0) {
        m_basePlayer->loadFromPreset(uint(curIndex - 1));
        //设置放大值
        m_basePlayer->setPreamplification(m_basePlayer->preamplification());
        for (int i = 0 ; i < 10; i++) {
            //设置频率值
            m_basePlayer->setAmplificationForBandAt(m_basePlayer->amplificationForBandAt(uint(i)), uint(i));
        }
    }
}

void Player::onSleepWhenTaking(bool sleep)
{
    qDebug() << "onSleepWhenTaking:" << sleep;
    if (sleep) {
        //休眠记录状态
        if (m_basePlayer->state() == PlayerBase::Playing) {
            //休眠唤醒前设置音量为1%
            readSinkInputPath();
            if (!m_sinkInputPath.isEmpty()) {
                QDBusInterface ainterface("com.deepin.daemon.Audio", m_sinkInputPath,
                                          "com.deepin.daemon.Audio.SinkInput",
                                          QDBusConnection::sessionBus());
                if (!ainterface.isValid()) {
                    return ;
                }
                //停止播放并记录播放位置
                m_Vlcstate = Vlc::Playing;
                m_basePlayer->pause();
                qlonglong time = position();
                INT_LAST_PROGRESS_FLAG = 1;
                m_ActiveMeta.offset = time;
                m_basePlayer->stop();
            }
        }
    } else { //设置状态
        if (m_Vlcstate == Vlc::Playing) {
            //播放
            QTimer::singleShot(2000, [ = ]() {
                playMeta(m_ActiveMeta);
            });
            m_Vlcstate = -1;
        }
    }
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
                                                     "com.deepin.daemon.Audio.SinkInput", "Icon");

        if (!nameV.isValid() || nameV.toString() != "deepin-music")
            continue;

        m_sinkInputPath = curPath.path();
        break;
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

bool Player::setMusicVolume(double volume)
{
    if (volume > 1.0) {
        volume = 1.000;
    }

    m_mpris->setVolume(volume);

    m_basePlayer->setVolume(int(volume * 100));

    return false;
}

bool Player::setMusicMuted(bool muted)
{
    if (!QFile::exists(m_ActiveMeta.localPath) && m_ActiveMeta.mmType != MIMETYPE_CDA) {
        return false;
    }
    m_basePlayer->setMute(muted);

    return true;
}

bool Player::isMusicMuted()
{
    return m_basePlayer->getMute();
}

void Player::initPlayer()
{
    if (Global::playbackEngineType() != 1) {
        m_basePlayer = new QtPlayer(this);
        qDebug() << __func__ << "QtPlayer";
    } else {
        m_basePlayer = new VlcPlayer(this);
        qDebug() << __func__ << "VlcPlayer";
    }

    m_timer = new QTimer(this);
    m_timer->setInterval(300);
    connect(m_timer, &QTimer::timeout, this, &Player::changePicture);
    connect(m_basePlayer, &PlayerBase::timeChanged,
    this, [ = ](qint64 position) {
        Q_EMIT positionChanged(position,  m_ActiveMeta.length, 1); //直接上报当前位置，offset无实质意义
    });
    connect(m_basePlayer, &PlayerBase::positionChanged,
    this, [ = ](float position) {
        qint64 curPosition = static_cast<qint64>(position * m_ActiveMeta.length);
        Q_EMIT positionChanged(curPosition,  m_ActiveMeta.length, 1); //直接上报当前位置，offset无实质意义
        if (INT_LAST_PROGRESS_FLAG == 0) {
            m_ActiveMeta.offset = curPosition;
            MusicSettings::setOption("base.play.last_position", curPosition);
            // cd特殊处理
            if (!QFile::exists(m_ActiveMeta.localPath) && m_ActiveMeta.mmType != MIMETYPE_CDA) {
                playNextMeta(true);
            }
        }
    });

    //vlc stateChanged
    connect(m_basePlayer, &PlayerBase::stateChanged,
    this, [ = ](PlayerBase::PlayState status) {
        switch (status) {
        case PlayerBase::Playing: {
            //emit signalPlaybackStatusChanged(Player::Playing);
            if (!m_timer->isActive()) {
                m_timer->start();
            }
            break;
        }
        case PlayerBase::Paused: {
            //emit signalPlaybackStatusChanged(Player::Paused);
            m_timer->stop();
            break;
        }
        case PlayerBase::Stopped: {
            //emit signalPlaybackStatusChanged(Player::Stopped);
            m_timer->stop();
            //emit signalMediaMetaChanged(MediaMeta()); //由setActiveMeta统一发送信号
            break;
        }
        default:
            break;
        }
    });

    connect(m_basePlayer, &PlayerBase::end,
    this, [ = ]() {
        playNextMeta(true);//just sync with Vlc::Ended
    });

    connect(m_basePlayer, &PlayerBase::sigSendCdaStatus, CommonService::getInstance(),
            &CommonService::signalCdaSongListChanged);
}

void Player::initMpris()
{
    m_mpris =  new MprisPlayer();
    m_mpris->setServiceName("DeepinMusic");

    m_mpris->setSupportedMimeTypes(mimeTypes());

    m_mpris->setSupportedUriSchemes(QStringList() << "file");
    m_mpris->setCanQuit(true);
    m_mpris->setCanRaise(true);
    m_mpris->setCanSetFullscreen(false);
    m_mpris->setHasTrackList(true);
    // setDesktopEntry: see https://specifications.freedesktop.org/mpris-spec/latest/Media_Player.html#Property:DesktopEntry for more
    m_mpris->setDesktopEntry("deepin-music");
    m_mpris->setIdentity("Deepin Music Player");

    m_mpris->setCanControl(true);
    m_mpris->setCanPlay(true);
    m_mpris->setCanGoNext(true);
    m_mpris->setCanGoPrevious(true);
    m_mpris->setCanPause(true);
    m_mpris->setCanSeek(true);

    connect(this, &Player::signalPlaybackStatusChanged,
    this, [ = ](Player::PlaybackStatus playbackStatus) {
        switch (playbackStatus) {
        case Player::InvalidPlaybackStatus:
        case Player::Stopped:
            m_mpris->setPlaybackStatus(Mpris::Stopped);
            break;
        case Player::Playing:
            m_mpris->setPlaybackStatus(Mpris::Playing);
            break;
        case Player::Paused:
            m_mpris->setPlaybackStatus(Mpris::Paused);
            break;
        }
    });

    connect(m_mpris, &MprisPlayer::seekRequested,
    this, [ = ](qlonglong offset) {
        setPosition(position() + offset);
    });

    connect(m_mpris, &MprisPlayer::setPositionRequested,
    this, [ = ](const QDBusObjectPath & trackId, qlonglong offset) {
        Q_UNUSED(trackId)
        setPosition(offset);
    });

    connect(m_mpris, &MprisPlayer::stopRequested,
    this, [ = ]() {
        stop();
        m_mpris->setPlaybackStatus(Mpris::Stopped);
        m_mpris->setMetadata(QVariantMap());
    });

    connect(m_mpris, &MprisPlayer::playRequested,
    this, [ = ]() {
        // dbus暂停立即执行
        if (status() == Player::Paused) {
            bool curFadeInOut = m_fadeInOut;
            m_fadeInOut = false;
            resume();
            m_fadeInOut = curFadeInOut;
        } else {
            if (status() != Player::Playing) {
                //播放列表为空时也应考虑，不然会出现dbus调用无效的情况
                forcePlayMeta();
            }
        }
    });

    connect(m_mpris, &MprisPlayer::pauseRequested,
    this, [ = ]() {
        pauseNow();
    });

    connect(m_mpris, &MprisPlayer::nextRequested,
    this, [ = ]() {
        if (m_MetaList.size() == 0) {
            if (DataBaseService::getInstance()->getDelStatus() == true)//判断是否正在删除中
                return;
            // 更新所有歌曲页面数据
            setCurrentPlayListHash("all", true);
            // 通知播放队列刷新
            emit signalPlayListChanged();
            if (m_MetaList.size() == 0)
                return;
        }
        playNextMeta(false);
    });

    connect(m_mpris, &MprisPlayer::previousRequested,
    this, [ = ]() {
        if (m_MetaList.size() == 0) {
            if (DataBaseService::getInstance()->getDelStatus() == true)//判断是否正在删除中
                return;
            // 更新所有歌曲页面数据
            setCurrentPlayListHash("all", true);
            // 通知播放队列刷新
            emit signalPlayListChanged();
            if (m_MetaList.size() == 0)
                return;
        }
        playPreMeta();
    });

    connect(m_mpris, &MprisPlayer::openUriRequested, this, [ = ](const QUrl & url) {
        qDebug() << __FUNCTION__ << "toString = " << url.toString();
        qDebug() << __FUNCTION__ << "toLocalFile = " << url.toLocalFile();
        QString path = url.toLocalFile();
        if (path.isEmpty()) {
            path = url.toString().isEmpty() ? url.path() : url.toString(); //复杂名称可能出现tostring为空的问题，直接取path()
            if (path.isEmpty()) {
                return;
            }
        }
        QFileInfo fileInfo(path);
        //符号连接无法正常播放
        if (fileInfo.isSymLink())
            path = fileInfo.symLinkTarget();
        DataBaseService::getInstance()->setFirstSong(path);
        DataBaseService::getInstance()->importMedias("all", QStringList() << path);
    });
}

void Player::resetDBusMpris(const MediaMeta &meta)
{
    QVariantMap metadata;
    metadata.insert(Mpris::metadataToString(Mpris::Title), meta.title);
    metadata.insert(Mpris::metadataToString(Mpris::Artist), meta.singer);
    metadata.insert(Mpris::metadataToString(Mpris::Album), meta.album);
    metadata.insert(Mpris::metadataToString(Mpris::Length), meta.length);
    QString str = Global::cacheDir() + "/images/" + meta.hash + ".jpg";
    QFileInfo info(str);
    if (!info.exists()) {
        str =  Global::cacheDir() + "/images/" + "default_cover_max.jpg";
        info.setFile(str);
        if (!info.exists()) {
            QIcon icon = QIcon::fromTheme("cover_max");
            icon.pixmap(QSize(50, 50)).save(str);
        }
    }
    str = "file://" + str;
    metadata.insert(Mpris::metadataToString(Mpris::ArtUrl), str);
    m_mpris->setMetadata(metadata);
}

void Player::changePicture()
{
    //切换播放状态动态图
    int curCount = m_playingCount % 4;
    m_playingCount = curCount + 1;
    switch (curCount) {
    case 0:
        // 使用已加载的缓存图片，不必要每次都加载
        m_playingIcon = m_playingIcon1;
        break;
    case 1:
        m_playingIcon = m_playingIcon2;
        break;
    case 2:
        m_playingIcon = m_playingIcon3;
        break;
    case 3:
        m_playingIcon = m_playingIcon4;
        break;
    default:
        m_playingIcon = m_playingIcon1;
        break;
    }
    emit signalUpdatePlayingIcon();
}
