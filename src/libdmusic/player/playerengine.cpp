// Copyright (C) 2020 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "playerengine.h"

#include <QMimeDatabase>
#include <QTimer>
#include <QFile>
#include <QFileInfo>
#include <QRandomGenerator>
#include <QPropertyAnimation>
#include <QIcon>

#include <MprisPlayer>

#include "qtplayer.h"
#include "vlcplayer.h"
#include "util/log.h"

static const int sFadeInOutAnimationDuration = 900; //ms
static int INT_LAST_PROGRESS_FLAG = 1;
static qint64 INT_LAST_PROGRESS_VALUE = 0;

static QStringList mimeTypes()
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

class PlayerEnginePrivate
{
private:
    explicit PlayerEnginePrivate(PlayerEngine *parent);
    friend class PlayerEngine;
    PlayerEngine               *m_playerEngine          = nullptr;
    QList<MediaMeta>            m_metaList;
    QTimer                     *m_changePictureTimer    = nullptr;
    PlayerBase                 *m_player                = nullptr;
    MprisPlayer                *m_mprisPlayer           = nullptr;
    QString                     m_playlistHash;
    DmGlobal::PlaybackMode      m_playbackMode          = DmGlobal::RepeatNull;
    int                         m_playingCount          = 0;
    double                      m_fadeInOutFactor       = 1.0;
    QPropertyAnimation         *m_fadeInAnimation       = nullptr;
    QPropertyAnimation         *m_fadeOutAnimation      = nullptr;
    bool                        m_fadeInOut             = false;
};

PlayerEnginePrivate::PlayerEnginePrivate(PlayerEngine *parent)
    : m_playerEngine(parent)
{
    qCDebug(dmMusic) << "Initializing PlayerEnginePrivate";
    if (DmGlobal::playbackEngineType() != 1) {
        m_player = new QtPlayer(m_playerEngine);
        qCDebug(dmMusic) << "Initializing QtPlayer engine";
    } else {
        m_player = new VlcPlayer(m_playerEngine);
        qCDebug(dmMusic) << "Initializing VlcPlayer engine";
    }

    m_changePictureTimer = new QTimer(m_playerEngine);
    m_changePictureTimer->setInterval(300);
}

PlayerEngine::PlayerEngine(QObject *parent)
    : QObject(parent)
    , m_data(new PlayerEnginePrivate(this))
{
    qCDebug(dmMusic) << "Initializing PlayerEngine";
    connect(m_data->m_changePictureTimer, &QTimer::timeout, this, [ = ]() {
        int curNum = m_data->m_playingCount % 4 + 1;
        m_data->m_playingCount = curNum;
        emit playPictureChanged(QString("music_play%1").arg(curNum));
    });
    connect(m_data->m_player, &PlayerBase::metaChanged, this, &PlayerEngine::metaChanged);
    connect(m_data->m_player, &PlayerBase::timeChanged,
    this, [ = ](qint64 position) {
        Q_EMIT positionChanged(position,  m_data->m_player->getMediaMeta().length); //直接上报当前位置，offset无实质意义
    });

    connect(m_data->m_player, &PlayerBase::positionChanged,
    this, [ = ](float position) {
        qint64 curPosition = static_cast<qint64>(position * m_data->m_player->getMediaMeta().length);
        Q_EMIT positionChanged(curPosition,  m_data->m_player->getMediaMeta().length); //直接上报当前位置，offset无实质意义
    });

    //vlc stateChanged
    connect(m_data->m_player, &PlayerBase::stateChanged,
    this, [ = ](DmGlobal::PlaybackStatus status) {
        switch (status) {
        case DmGlobal::Playing:
            qCInfo(dmMusic) << "Playback state changed to Playing";
            if (!m_data->m_changePictureTimer->isActive()) {
                m_data->m_changePictureTimer->start();
            }
            break;
        case DmGlobal::Paused:
            qCInfo(dmMusic) << "Playback state changed to Paused";
            m_data->m_changePictureTimer->stop();
            break;
        default:
            qCInfo(dmMusic) << "Playback state changed to Stopped";
            m_data->m_changePictureTimer->stop();
            break;
        }
        emit playbackStatusChanged(playbackStatus());
    });

    connect(m_data->m_player, &PlayerBase::end,
    this, [ = ]() {
        qCDebug(dmMusic) << "Current track playback ended";
        playNextMeta(true);
    });
    connect(m_data->m_player, &PlayerBase::sigSendCdaStatus,
            this, &PlayerEngine::sendCdaStatus);

    m_data->m_fadeOutAnimation = new QPropertyAnimation(this, "fadeInOutFactor", this);
    m_data->m_fadeOutAnimation->setEasingCurve(QEasingCurve::OutCubic);
    m_data->m_fadeOutAnimation->setStartValue(1.0000);
    m_data->m_fadeOutAnimation->setEndValue(0.1000);
    m_data->m_fadeOutAnimation->setDuration(sFadeInOutAnimationDuration * 2);
    connect(m_data->m_fadeOutAnimation, &QPropertyAnimation::finished,
    this, [ = ]() {
        m_data->m_player->pause();
        setFadeInOutFactor(1.0);
    });

    m_data->m_fadeInAnimation = new QPropertyAnimation(this, "fadeInOutFactor", this);
    m_data->m_fadeInAnimation->setEasingCurve(QEasingCurve::InCubic);
    m_data->m_fadeInAnimation->setStartValue(0.1000);
    m_data->m_fadeInAnimation->setEndValue(1.0000);
    m_data->m_fadeInAnimation->setDuration(sFadeInOutAnimationDuration);
    qCDebug(dmMusic) << "Initializing fade in animation";
}

PlayerEngine::~PlayerEngine()
{
    qCDebug(dmMusic) << "Destroying PlayerEngine";
    if (m_data->m_mprisPlayer) {
        qCDebug(dmMusic) << "Destroying MprisPlayer";
        delete m_data->m_mprisPlayer;
        m_data->m_mprisPlayer = nullptr;
    }
}

double PlayerEngine::fadeInOutFactor() const
{
    double factor = 1.0;
    if (m_data != nullptr) {
        factor = m_data->m_fadeInOutFactor;
    }
    qCDebug(dmMusic) << "Current fade in/out factor:" << factor;
    return factor;
}

void PlayerEngine::setFadeInOut(bool flag)
{
    qCDebug(dmMusic) << "Setting fade in/out flag to:" << flag;
    if (m_data != nullptr) {
        qCDebug(dmMusic) << "Fade in/out flag set to:" << flag;
        m_data->m_fadeInOut = flag;
    }
}

void PlayerEngine::setMprisPlayer(const QString &serviceName, const QString &desktopEntry, const QString &identity)
{
    qCDebug(dmMusic) << "Initializing MprisPlayer with service:" << serviceName;
    //init Mpris
    m_data->m_mprisPlayer = new MprisPlayer(this);
    m_data->m_mprisPlayer->setServiceName(serviceName);
    m_data->m_mprisPlayer->setSupportedMimeTypes(mimeTypes());
    m_data->m_mprisPlayer->setSupportedUriSchemes(QStringList() << "file");
    m_data->m_mprisPlayer->setCanQuit(true);
    m_data->m_mprisPlayer->setCanRaise(true);
    m_data->m_mprisPlayer->setCanSetFullscreen(false);
    m_data->m_mprisPlayer->setHasTrackList(true);
    m_data->m_mprisPlayer->setDesktopEntry(desktopEntry);
    m_data->m_mprisPlayer->setIdentity(identity);
    m_data->m_mprisPlayer->setCanControl(true);
    m_data->m_mprisPlayer->setCanPlay(true);
    m_data->m_mprisPlayer->setCanGoNext(true);
    m_data->m_mprisPlayer->setCanGoPrevious(true);
    m_data->m_mprisPlayer->setCanPause(true);
    m_data->m_mprisPlayer->setCanSeek(true);

    connect(this, &PlayerEngine::playbackStatusChanged,
    this, [ = ](DmGlobal::PlaybackStatus playbackStatus) {
        switch (playbackStatus) {
        case DmGlobal::Playing:
            m_data->m_mprisPlayer->setPlaybackStatus(Mpris::Playing);
            break;
        case DmGlobal::Paused:
            m_data->m_mprisPlayer->setPlaybackStatus(Mpris::Paused);
            break;
        default:
            m_data->m_mprisPlayer->setPlaybackStatus(Mpris::Stopped);
            break;
        }
    });

    connect(m_data->m_mprisPlayer, &MprisPlayer::seekRequested,
    this, [ = ](qlonglong offset) {
        setTime(time() + offset);
    });

    connect(m_data->m_mprisPlayer, &MprisPlayer::setPositionRequested,
    this, [ = ](const QDBusObjectPath & trackId, qlonglong offset) {
        Q_UNUSED(trackId)
        setTime(offset);
    });

    connect(m_data->m_mprisPlayer, &MprisPlayer::stopRequested,
    this, [ = ]() {
        stop();
        m_data->m_mprisPlayer->setPlaybackStatus(Mpris::Stopped);
        m_data->m_mprisPlayer->setMetadata(QVariantMap());
    });

    connect(m_data->m_mprisPlayer, &MprisPlayer::playRequested,
    this, [ = ]() {
        // dbus暂停立即执行
        if (isEmpty()) {
            Q_EMIT playPlaylistRequested("all");
        } else {
            if (playbackStatus() == DmGlobal::Paused) {
                resume();
            } else {
                if (playbackStatus() != DmGlobal::Playing) {
                    //播放列表为空时也应考虑，不然会出现dbus调用无效的情况
                    playNextMeta(false);
                } else {
                    pauseNow();
                }
            }
        }
    });

    connect(m_data->m_mprisPlayer, &MprisPlayer::pauseRequested, this, &PlayerEngine::pauseNow);

    connect(m_data->m_mprisPlayer, &MprisPlayer::nextRequested,
    this, [ = ]() {
        playNextMeta(false);
    });

    connect(m_data->m_mprisPlayer, &MprisPlayer::previousRequested,
    this, [ = ]() {
        playPreMeta();
    });

    connect(m_data->m_mprisPlayer, &MprisPlayer::openUriRequested, this, &PlayerEngine::openUriRequested);

    connect(m_data->m_mprisPlayer, &MprisPlayer::quitRequested, this, &PlayerEngine::quitRequested);
    connect(m_data->m_mprisPlayer, &MprisPlayer::raiseRequested, this, &PlayerEngine::raiseRequested);
    qCDebug(dmMusic) << "MprisPlayer initialized";
}

void PlayerEngine::setMediaMeta(const QString &metaHash)
{
    qCDebug(dmMusic) << "Setting media meta hash to:" << metaHash;
    for (int i = 0; i < m_data->m_metaList.size(); ++i) {
        if (metaHash == m_data->m_metaList[i].hash) {
            setMediaMeta(m_data->m_metaList[i]);
            break;
        }
    }
}

void PlayerEngine::setMediaMeta(const MediaMeta &meta)
{
    qCInfo(dmMusic) << "Setting media meta - Title:" << meta.title << "Artist:" << meta.artist;
    if (!m_data->m_player->getMediaMeta().localPath.isEmpty())
        INT_LAST_PROGRESS_FLAG = 0;
    m_data->m_player->setMediaMeta(meta);
    resetDBusMpris(meta);
}

QStringList PlayerEngine::supportedSuffixList() const
{
    qCDebug(dmMusic) << "Supported suffix list";
    return m_data->m_player->supportedSuffixList();
}

MediaMeta PlayerEngine::getMediaMeta()
{
    qCDebug(dmMusic) << "Getting media meta";
    return m_data->m_player->getMediaMeta();
}

void PlayerEngine::addMetasToPlayList(const QList<MediaMeta> &metaList)
{
    qCDebug(dmMusic) << "Adding metas to play list";
    m_data->m_metaList << metaList;
}

void PlayerEngine::removeMetaFromPlayList(const QString &metaHash)
{
    qCDebug(dmMusic) << "Removing meta from play list, hash:" << metaHash;
    for (int i = 0; i < m_data->m_metaList.size(); i++) {
        if (m_data->m_metaList[i].hash == metaHash) {
            m_data->m_metaList.removeAt(i);
            qCDebug(dmMusic) << "Meta removed from play list";
            break;
        }
    }
}

void PlayerEngine::removeMetasFromPlayList(const QStringList &metaHashs)
{
    qCDebug(dmMusic) << "Removing metas from play list";
    QStringList curMetaHashs = metaHashs;
    QString playHash = getMediaMeta().hash;
    bool playFlag = curMetaHashs.contains(playHash);
    int curIndex = -1;
    for (int i = m_data->m_metaList.size() - 1; i >= 0; i--) {
        if (curIndex >= 0) curIndex = i - 1;
        if (playFlag && m_data->m_metaList[i].hash == playHash) curIndex = i - 1;
        if (curMetaHashs.contains(m_data->m_metaList[i].hash)) {
            curMetaHashs.removeOne(m_data->m_metaList[i].hash);
            m_data->m_metaList.removeAt(i);
            qCDebug(dmMusic) << "Meta removed from play list";
        }
        if (curMetaHashs.isEmpty()) {
            qCDebug(dmMusic) << "All metas removed from play list, break";
            break;
        }
    }
    if (m_data->m_metaList.isEmpty()) {
        qCDebug(dmMusic) << "Play list is empty, stop";
        stop();
    } else if (playFlag) {
        qCDebug(dmMusic) << "Play flag is true, play next meta";
        if (curIndex >= 0) {
            qCDebug(dmMusic) << "Cur index is valid, play next meta";
            playNextMeta(m_data->m_metaList[curIndex], true, playbackStatus() == DmGlobal::Playing);
        } else {
            qCDebug(dmMusic) << "Cur index is invalid, play first meta";
            playNextMeta(true, playbackStatus() == DmGlobal::Playing);
        }
    }

}

void PlayerEngine::clearPlayList(bool stopFlag)
{
    qCDebug(dmMusic) << "Clearing play list";
    m_data->m_metaList.clear();
    if (stopFlag && !getMediaMeta().hash.isEmpty()) {
        qCDebug(dmMusic) << "Stop flag is true, stop player";
        stop();
    }
}

QList<MediaMeta> PlayerEngine::getMetas()
{
    qCDebug(dmMusic) << "Getting play list, size:" << m_data->m_metaList.size();
    return m_data->m_metaList;
}

bool PlayerEngine::isEmpty()
{
    bool bEmpty = m_data->m_metaList.isEmpty();
    qCDebug(dmMusic) << "Play list is empty:" << bEmpty;
    return bEmpty;
}

void PlayerEngine::play()
{
    qCDebug(dmMusic) << "Play requested";
    // 防止淡入淡出时切换歌曲
    if (m_data->m_fadeInOut) {
        qCDebug(dmMusic) << "Fade in out is true, stop fade out animation";
        m_data->m_fadeOutAnimation->stop();
        setFadeInOutFactor(1.0);
    }

    if (m_data->m_player->getMediaMeta().localPath.isEmpty()) {
        qCDebug(dmMusic) << "Current meta local path is empty, force play";
        forcePlay();
    } else {
        qCDebug(dmMusic) << "Current meta local path is not empty, play normally";
        m_data->m_player->play();
    }

    if (INT_LAST_PROGRESS_FLAG == 1) {
        qCDebug(dmMusic) << "Last progress flag is 1, pause and play after 150ms";
        pause();
        QTimer::singleShot(150, this, [ = ]() {//为了记录进度条生效，在加载的时候让音乐播放150ms
            setTime(INT_LAST_PROGRESS_VALUE);
            play();
        });
    }

    INT_LAST_PROGRESS_FLAG = 0;
}

void PlayerEngine::forcePlay()
{
    qCDebug(dmMusic) << "Force play requested";
    if (m_data->m_metaList.isEmpty()) return;
    setMediaMeta(m_data->m_metaList.first());
    play();
}

void PlayerEngine::pause()
{
    qCDebug(dmMusic) << "Pause requested";
    if (m_data->m_fadeInOut) {
        m_data->m_fadeInAnimation->stop();
    }
    if (m_data->m_fadeInOut && m_data->m_fadeOutAnimation->state() != QPropertyAnimation::Running) {
        m_data->m_fadeOutAnimation->start();
    } else {
        m_data->m_player->pause();
        setFadeInOutFactor(1.0);
    }
}

void PlayerEngine::pauseNow()
{
    qCDebug(dmMusic) << "Pause now requested";
    m_data->m_player->pause();
}

void PlayerEngine::playPause()
{
    qCDebug(dmMusic) << "Play pause requested";
    // dbus暂停立即执行
    if (playbackStatus() == DmGlobal::Paused) {
        qCDebug(dmMusic) << "Player is paused, resume";
        resume();
    } else {
        qCDebug(dmMusic) << "Player is playing, pause";
        if (playbackStatus() != DmGlobal::Playing) {
            qCDebug(dmMusic) << "Player is not playing, play pause";
            //播放列表为空时也应考虑，不然会出现dbus调用无效的情况
            if (getMediaMeta().localPath.isEmpty()) {
                qCDebug(dmMusic) << "Current meta local path is empty, play next meta";
                playNextMeta(false);
            } else {
                qCDebug(dmMusic) << "Current meta local path is not empty, play pause";
                play();
            }
        } else {
            qCDebug(dmMusic) << "Player is playing, pause";
            pause();
        }
    }
    qCDebug(dmMusic) << "Play pause finished";
}

void PlayerEngine::resume()
{
    qCDebug(dmMusic) << "Resume requested";
    if (m_data->m_fadeInOut) {
        qCDebug(dmMusic) << "Fade in out is true, stop fade in animation";
        setFadeInOutFactor(0.1);
        m_data->m_fadeOutAnimation->stop();
    }
    if (m_data->m_player->getMediaMeta().localPath.isEmpty()) {
        qCDebug(dmMusic) << "Current meta local path is empty, force play";
        forcePlay();
    } else {
        qCDebug(dmMusic) << "Current meta local path is not empty, play resume";
        m_data->m_player->play();
    }
    if (m_data->m_fadeInOut && m_data->m_fadeInAnimation->state() != QPropertyAnimation::Running) {
        qCDebug(dmMusic) << "Fade in out is true, start fade in animation";
        m_data->m_fadeInAnimation->start();
    }
    qCDebug(dmMusic) << "Resume finished";
}

void PlayerEngine::playPreMeta()
{
    qCDebug(dmMusic) << "Playing previous track";
    MediaMeta curMeta = m_data->m_player->getMediaMeta();
    QList<MediaMeta> &allMetas = m_data->m_metaList;
    // 歌单顺序播放
    if (m_data->m_playbackMode == DmGlobal::RepeatNull) {
        qCDebug(dmMusic) << "Playback mode is RepeatNull, play previous track";
        int index = -1, preIndex = -1;
        for (int i = allMetas.size() - 1; i >= 0; i--) {
            if (index != -1) {
                if ((QFile::exists(allMetas[i].localPath) && m_data->m_player->supportedSuffixList().contains(
                            QFileInfo(allMetas[i].localPath).suffix().toLower())) || allMetas[i].mmType == DmGlobal::MimeTypeCDA) {
                    preIndex = i;
                    break;
                }
            } else if (allMetas.at(i).hash == curMeta.hash) {
                index = i;
            }
        }

        if (preIndex != -1) {
            switchToNewTrackWithFade(allMetas.at(preIndex), true);
        } else {
            stop();
        }
        return;
    }

    QList<MediaMeta> curMetaList;

    for (int i = 0; i < allMetas.size(); i++) {
        if ((QFile::exists(allMetas[i].localPath)
                && m_data->m_player->supportedSuffixList().contains(
                    QFileInfo(allMetas[i].localPath).suffix().toLower())) || allMetas[i].mmType == DmGlobal::MimeTypeCDA)
            curMetaList.append(allMetas[i]);
    }
    if (curMetaList.size() > 0) {
        qCDebug(dmMusic) << "Current meta list size:" << curMetaList.size();
        //播放模式todo
        int index = 0;
        for (int i = 0; i < curMetaList.size(); i++) {
            if (curMetaList.at(i).hash == curMeta.hash) {
                index = i;
                break;
            }
        }
        //根据播放模式确定下一首
        switch (m_data->m_playbackMode) {
        case DmGlobal::RepeatAll:
        case DmGlobal::RepeatSingle: {
            if (index == 0) {
                index = curMetaList.size() - 1;
            } else {
                index--;
            }
            break;
        }
        case DmGlobal::Shuffle: {
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
        switchToNewTrackWithFade(curMetaList.at(index), true);
    }
    qCDebug(dmMusic) << "Play previous track finished";
}

void PlayerEngine::playNextMeta(const DMusic::MediaMeta &meta, bool isAuto, bool playFlag)
{
    qCDebug(dmMusic) << "Playing next track";
    QList<MediaMeta> &allMetas = m_data->m_metaList;
    // 歌单顺序播放
    if (m_data->m_playbackMode == DmGlobal::RepeatNull) {
        qCDebug(dmMusic) << "Playback mode is RepeatNull, play next track";
        int index = -1, newIndex = -1;
        for (int i = 0; i < allMetas.size(); i++) {
            if (index != -1) {
                if ((QFile::exists(allMetas[i].localPath) && m_data->m_player->supportedSuffixList().contains(
                            QFileInfo(allMetas[i].localPath).suffix().toLower())) || allMetas[i].mmType == DmGlobal::MimeTypeCDA) {
                    newIndex = i;
                    break;
                }
            } else if (allMetas.at(i).hash == meta.hash) {
                index = i;
            }
        }
        // If current track not found in list (possibly removed)
        // and no next track found, play the first track
        if (index == -1 && newIndex == -1 && !allMetas.isEmpty())
            newIndex = 0;
        if (newIndex != -1) {
            switchToNewTrackWithFade(allMetas.at(newIndex), playFlag);
        } else if (!isAuto) {
            for (int i = 0; i < allMetas.size(); i++) {
                if ((QFile::exists(allMetas[i].localPath) && m_data->m_player->supportedSuffixList().contains(
                            QFileInfo(allMetas[i].localPath).suffix().toLower())) || allMetas[i].mmType == DmGlobal::MimeTypeCDA) {
                    index = i;
                    break;
                }
            }
            if (index != -1) {
                switchToNewTrackWithFade(allMetas[index], playFlag);
            }
        } else {
            qCDebug(dmMusic) << "No next track found";
            stop();
        }
        return;
    }

    QList<QPair<int, MediaMeta> > curMetaList;
    for (int i = 0; i < allMetas.size(); i++) {
        if ((QFile::exists(allMetas[i].localPath) && m_data->m_player->supportedSuffixList().contains(
                 QFileInfo(allMetas[i].localPath).suffix().toLower())) || allMetas[i].mmType == DmGlobal::MimeTypeCDA) {
            auto&& indexRef = i; // 创建一个引用
            curMetaList.append(qMakePair(std::move(indexRef), allMetas[i]));
        }
    }
    if (curMetaList.size() > 0) {
        qCDebug(dmMusic) << "Current meta list size:" << curMetaList.size();
        int index = -1;
        for (int i = 0; i < curMetaList.size(); i++) {
            if (curMetaList.at(i).second.hash == meta.hash) {
                index = i;
                break;
            }
        }

        if (index == -1 && !meta.hash.isEmpty()) {
            for (int i = 0; i < allMetas.size(); i++) {
                if (allMetas.at(i).hash == meta.hash) {
                    index = i;
                    break;
                }
            }
            if (index != -1) {
                for (int i = 0; i < curMetaList.size(); i++) {
                    if (curMetaList.at(i).first > index && i >= 0) {
                        index = i - 1;
                        break;
                    }
                }
            }
        }

        switch (m_data->m_playbackMode) {
        case DmGlobal::RepeatAll: {
            qCDebug(dmMusic) << "Playback mode is RepeatAll, play next track";
            if (index == (curMetaList.size() - 1)) {
                index = 0;
            } else {
                index++;
            }
            break;
        }
        case DmGlobal::RepeatSingle: {
            qCDebug(dmMusic) << "Playback mode is RepeatSingle, play next track";
            if (!isAuto) {
                if (index == (curMetaList.size() - 1)) {
                    index = 0;
                } else {
                    index++;
                }
            }
            break;
        }
        case DmGlobal::Shuffle: {
            qCDebug(dmMusic) << "Playback mode is Shuffle, play next track";
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
            break;
        }
        default: {
            qCDebug(dmMusic) << "Unknown playback mode, play next track";
            if (index == (curMetaList.size() - 1)) {
                index = 0;
            } else {
                index++;
            }
            break;
        }
        }

        index = (index == -1 ? 0 : index);
        index = (index >= curMetaList.size() ? 0 : index);
        switchToNewTrackWithFade(curMetaList.at(index).second, playFlag);
        qCDebug(dmMusic) << "Playing track at index:" << index;
    } else {
        qCDebug(dmMusic) << "No media meta set, stopping player";
        stop();
    }
    qCDebug(dmMusic) << "Player stopped";
}

void PlayerEngine::resetDBusMpris(const DMusic::MediaMeta &meta)
{
    qCDebug(dmMusic) << "Resetting DBus Mpris with meta:" << meta.title;
    QVariantMap metadata;
    metadata.insert(Mpris::metadataToString(Mpris::Title), meta.title);
    metadata.insert(Mpris::metadataToString(Mpris::Artist), meta.artist);
    metadata.insert(Mpris::metadataToString(Mpris::Album), meta.album);
    metadata.insert(Mpris::metadataToString(Mpris::Length), meta.length);
    QString str = DmGlobal::cachePath() + "/images/" + meta.hash + ".jpg";
    QFileInfo info(str);
    if (!info.exists()) {
        qCDebug(dmMusic) << "Cover image not found, using default";
        str =  DmGlobal::cachePath()  + "/images/" + "default_cover_max.jpg";
        info.setFile(str);
        if (!info.exists()) {
            QIcon icon = QIcon::fromTheme("cover_max");
            icon.pixmap(QSize(50, 50)).save(str);
        }
    }
    str = "file://" + str;
    metadata.insert(Mpris::metadataToString(Mpris::ArtUrl), str);
    m_data->m_mprisPlayer->setMetadata(metadata);
    qCDebug(dmMusic) << "DBus Mpris reset with meta:" << meta.title;
}

void PlayerEngine::playNextMeta(bool isAuto, bool playFlag)
{
    qCDebug(dmMusic) << "Playing next track, auto:" << isAuto << "playFlag:" << playFlag;
    MediaMeta curMeta = m_data->m_player->getMediaMeta();
    playNextMeta(curMeta, isAuto, playFlag);
}

void PlayerEngine::stop()
{
    qCInfo(dmMusic) << "Stop requested";
    m_data->m_player->stop();
    setMediaMeta(MediaMeta());
}

void PlayerEngine::switchToNewTrackWithFade(const DMusic::MediaMeta &meta, bool playFlag)
{
    qCInfo(dmMusic) << "Switching to new track with fade:" << meta.title;
    
    // 检查用户是否启用了淡入淡出效果且当前正在播放
    if (!m_data->m_fadeInOut || playbackStatus() != DmGlobal::Playing) {
        // 没有启用淡入淡出或当前没有播放，直接切换
        qCInfo(dmMusic) << "Direct track switch - fade enabled:" << m_data->m_fadeInOut << "playing:" << (playbackStatus() == DmGlobal::Playing);
        setMediaMeta(meta);
        if (playFlag) {
            play();
        }
        return;
    }
    
    // 启用了淡入淡出且正在播放，使用动画实现淡出-切换-淡入
    qCInfo(dmMusic) << "Starting track switch with fade effect";
    
    QPropertyAnimation *switchFadeOut = new QPropertyAnimation(this, "fadeInOutFactor", this);
    switchFadeOut->setEasingCurve(QEasingCurve::OutQuad);
    switchFadeOut->setStartValue(1.0);
    switchFadeOut->setEndValue(0.0);
    switchFadeOut->setDuration(400);
    
    connect(switchFadeOut, &QPropertyAnimation::finished, this, [=]() {
        qCInfo(dmMusic) << "Track switch fade out completed, switching to:" << meta.title;
        setMediaMeta(meta);
        
        if (playFlag) {
            m_data->m_player->play();
            QPropertyAnimation *switchFadeIn = new QPropertyAnimation(this, "fadeInOutFactor", this);
            switchFadeIn->setEasingCurve(QEasingCurve::InQuad);
            switchFadeIn->setStartValue(0.0);
            switchFadeIn->setEndValue(1.0);
            switchFadeIn->setDuration(600); // 稍慢的淡入
            connect(switchFadeIn, &QPropertyAnimation::finished, switchFadeIn, &QObject::deleteLater);
            switchFadeIn->start();
        }
        switchFadeOut->deleteLater();
    });
    
    switchFadeOut->start();
}

void PlayerEngine::switchToNewTrackWithFade(const QString &metaHash, bool playFlag)
{
    qCInfo(dmMusic) << "Switching to new track with fade by hash:" << metaHash;
    
    for (const auto &meta : m_data->m_metaList) {
        if (meta.hash == metaHash) {
            switchToNewTrackWithFade(meta, playFlag);
            return;
        }
    }
    
    qCInfo(dmMusic) << "Meta not found in playlist for hash:" << metaHash << ", using direct switch";
    setMediaMeta(metaHash);
    if (playFlag) {
        play();
    }
}

void PlayerEngine::setFadeInOutFactor(double fadeInOutFactor)
{
    qCDebug(dmMusic) << "Setting fade in/out factor to:" << fadeInOutFactor;
    m_data->m_fadeInOutFactor = fadeInOutFactor;
    m_data->m_player->blockSignals(true);
    m_data->m_player->setPreamplification(static_cast<float>(12 * m_data->m_fadeInOutFactor));
    m_data->m_player->blockSignals(false);
}

int PlayerEngine::length()
{
    int len = m_data->m_player->length();
    qCDebug(dmMusic) << "Track length:" << len;
    return len;
}

void PlayerEngine::setTime(qint64 time)
{
    qCDebug(dmMusic) << "Setting time to:" << time;
    if (INT_LAST_PROGRESS_FLAG == 1) {
        qCDebug(dmMusic) << "Last progress flag is set, using last progress value:" << INT_LAST_PROGRESS_VALUE;
        INT_LAST_PROGRESS_VALUE = time;
    } else {
        qCDebug(dmMusic) << "Last progress flag is not set, setting time directly";
        m_data->m_player->setTime(time);
    }
    qCDebug(dmMusic) << "Time set successfully";
}

qint64 PlayerEngine::time()
{
    qint64 curTime = INT_LAST_PROGRESS_FLAG == 1 ? INT_LAST_PROGRESS_VALUE : m_data->m_player->time();
    qCDebug(dmMusic) << "Current time:" << curTime;
    return curTime;
}

void PlayerEngine::setVolume(int volume)
{
    qCDebug(dmMusic) << "Setting volume to:" << volume;
    int curVolume = volume <= 0 ? 0 : volume;
    if (m_data->m_player->getVolume() == curVolume) {
        qCDebug(dmMusic) << "Volume unchanged, no need to set";
        emit volumeChanged(m_data->m_player->getVolume());
    } else {
        qCDebug(dmMusic) << "Setting volume to:" << curVolume;
        m_data->m_player->setVolume(curVolume);
        emit volumeChanged(m_data->m_player->getVolume());
        setMute(curVolume == 0);
    }
    qCDebug(dmMusic) << "Volume set successfully";
}

int PlayerEngine::getVolume()
{
    int curVolume = m_data->m_player->getVolume();
    qCDebug(dmMusic) << "Current volume:" << curVolume;
    return curVolume;
}

void PlayerEngine::setMute(bool mute)
{
    if (m_data->m_player->getMute() == mute) {
        qCDebug(dmMusic) << "Mute state unchanged:" << mute;
        emit muteChanged(mute);
    } else {
        qCDebug(dmMusic) << "Setting mute state from" << m_data->m_player->getMute() << "to:" << mute;
        m_data->m_player->setMute(mute);
        qCDebug(dmMusic) << "Mute state changed successfully";
        emit muteChanged(mute);
    }
}

bool PlayerEngine::getMute()
{
    bool curMute = m_data->m_player->getMute();
    qCDebug(dmMusic) << "Current mute state:" << curMute;
    return curMute;
}

DmGlobal::PlaybackStatus PlayerEngine::playbackStatus()
{
    return m_data->m_player->state();
}

void PlayerEngine::setPlaybackMode(DmGlobal::PlaybackMode mode)
{
    qCInfo(dmMusic) << "Setting playback mode from" << m_data->m_playbackMode << "to:" << mode;
    m_data->m_playbackMode = mode;
    qCDebug(dmMusic) << "Playback mode changed successfully";
}

DmGlobal::PlaybackMode PlayerEngine::getPlaybackMode()
{
    DmGlobal::PlaybackMode curMode = m_data->m_playbackMode;
    qCDebug(dmMusic) << "Current playback mode:" << curMode;
    return curMode;
}

void PlayerEngine::setCurrentPlayList(const QString &playlistHash)
{
    qCInfo(dmMusic) << "Setting current playlist:" << playlistHash;
    m_data->m_playlistHash = playlistHash;
}

QString PlayerEngine::getCurrentPlayList()
{
    qCDebug(dmMusic) << "Current playlist:" << m_data->m_playlistHash;
    return m_data->m_playlistHash;
}

QList<MediaMeta> PlayerEngine::getCdaMetaInfo()
{
    qCDebug(dmMusic) << "Getting CD meta info";
    return m_data->m_player->getCdaMetaInfo();
}

void PlayerEngine::setEqualizerEnabled(bool enabled)
{
    qCInfo(dmMusic) << "Setting equalizer enabled:" << enabled;
    m_data->m_player->setEqualizerEnabled(enabled);
    qCDebug(dmMusic) << "Equalizer state changed successfully";
}

void PlayerEngine::loadFromPreset(uint index)
{
    qCDebug(dmMusic) << "Loading equalizer preset:" << index;
    m_data->m_player->loadFromPreset(index);
    qCDebug(dmMusic) << "Equalizer preset loaded successfully";
}

void PlayerEngine::setPreamplification(float value)
{
    qCDebug(dmMusic) << "Setting preamplification to:" << value;
    m_data->m_player->setPreamplification(value);
}

void PlayerEngine::setAmplificationForBandAt(float amp, uint bandIndex)
{
    qCDebug(dmMusic) << "Setting amplification for band at:" << bandIndex << "to:" << amp;
    m_data->m_player->setAmplificationForBandAt(amp, bandIndex);
}

float PlayerEngine::amplificationForBandAt(uint bandIndex)
{
    qCDebug(dmMusic) << "Getting amplification for band at:" << bandIndex;
    return m_data->m_player->amplificationForBandAt(bandIndex);
}

float PlayerEngine::preamplification()
{
    qCDebug(dmMusic) << "Getting preamplification";
    return m_data->m_player->preamplification();
}
