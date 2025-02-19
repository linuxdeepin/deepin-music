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
    if (DmGlobal::playbackEngineType() != 1) {
        m_player = new QtPlayer(m_playerEngine);
        qDebug() << __func__ << "QtPlayer";
    } else {
        m_player = new VlcPlayer(m_playerEngine);
        qDebug() << __func__ << "VlcPlayer";
    }

    m_changePictureTimer = new QTimer(m_playerEngine);
    m_changePictureTimer->setInterval(300);
}

PlayerEngine::PlayerEngine(QObject *parent)
    : QObject(parent)
    , m_data(new PlayerEnginePrivate(this))
{
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
            if (!m_data->m_changePictureTimer->isActive()) {
                m_data->m_changePictureTimer->start();
            }
            break;
        case DmGlobal::Paused:
            m_data->m_changePictureTimer->stop();
            break;
        default:
            m_data->m_changePictureTimer->stop();
            break;
        }
        emit playbackStatusChanged(playbackStatus());
    });

    connect(m_data->m_player, &PlayerBase::end,
    this, [ = ]() {
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
}

PlayerEngine::~PlayerEngine()
{
    if (m_data->m_mprisPlayer) {
        delete m_data->m_mprisPlayer;
        m_data->m_mprisPlayer = nullptr;
    }
}

double PlayerEngine::fadeInOutFactor() const
{
    return m_data != nullptr ? m_data->m_fadeInOutFactor : 1.0;
}

void PlayerEngine::setFadeInOut(bool flag)
{
    if (m_data != nullptr) {
        m_data->m_fadeInOut = flag;
    }
}

void PlayerEngine::setMprisPlayer(const QString &serviceName, const QString &desktopEntry, const QString &identity)
{
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
}

void PlayerEngine::setMediaMeta(const QString &metaHash)
{
    for (int i = 0; i < m_data->m_metaList.size(); ++i) {
        if (metaHash == m_data->m_metaList[i].hash) {
            setMediaMeta(m_data->m_metaList[i]);
            break;
        }
    }
}

void PlayerEngine::setMediaMeta(const MediaMeta &meta)
{
    // 防止重复设置
    if (!m_data->m_player->getMediaMeta().localPath.isEmpty())
        INT_LAST_PROGRESS_FLAG = 0;
    m_data->m_player->setMediaMeta(meta);
    resetDBusMpris(meta);
}

QStringList PlayerEngine::supportedSuffixList() const
{
    return m_data->m_player->supportedSuffixList();
}

MediaMeta PlayerEngine::getMediaMeta()
{
    return m_data->m_player->getMediaMeta();
}

void PlayerEngine::addMetasToPlayList(const QList<MediaMeta> &metaList)
{
    m_data->m_metaList << metaList;
}

void PlayerEngine::removeMetaFromPlayList(const QString &metaHash)
{
    for (int i = 0; i < m_data->m_metaList.size(); i++) {
        if (m_data->m_metaList[i].hash == metaHash) {
            m_data->m_metaList.removeAt(i);
            break;
        }
    }
}

void PlayerEngine::removeMetasFromPlayList(const QStringList &metaHashs)
{
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
        }
        if (curMetaHashs.isEmpty()) break;
    }
    if (m_data->m_metaList.isEmpty())
        stop();
    else if (playFlag) {
        if (curIndex >= 0) {
            playNextMeta(m_data->m_metaList[curIndex], true, playbackStatus() == DmGlobal::Playing);
        } else {
            playNextMeta(true, playbackStatus() == DmGlobal::Playing);
        }
    }

}

void PlayerEngine::clearPlayList(bool stopFlag)
{
    m_data->m_metaList.clear();
    if (stopFlag && !getMediaMeta().hash.isEmpty()) stop();
}

QList<MediaMeta> PlayerEngine::getMetas()
{
    return m_data->m_metaList;
}

bool PlayerEngine::isEmpty()
{
    return m_data->m_metaList.isEmpty();
}

void PlayerEngine::play()
{
    // 防止淡入淡出时切换歌曲
    if (m_data->m_fadeInOut) {
        m_data->m_fadeOutAnimation->stop();
        setFadeInOutFactor(1.0);
    }

    if (m_data->m_player->getMediaMeta().localPath.isEmpty()) {
        forcePlay();
    } else
        m_data->m_player->play();

    if (INT_LAST_PROGRESS_FLAG == 1) {
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
    if (m_data->m_metaList.isEmpty()) return;
    setMediaMeta(m_data->m_metaList.first());
    play();
}

void PlayerEngine::pause()
{
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
    m_data->m_player->pause();
}

void PlayerEngine::playPause()
{
    // dbus暂停立即执行
    if (playbackStatus() == DmGlobal::Paused) {
        resume();
    } else {
        if (playbackStatus() != DmGlobal::Playing) {
            //播放列表为空时也应考虑，不然会出现dbus调用无效的情况
            if (getMediaMeta().localPath.isEmpty()) {
                playNextMeta(false);
            } else {
                play();
            }
        } else {
            pause();
        }
    }
}

void PlayerEngine::resume()
{
    if (m_data->m_fadeInOut) {
        setFadeInOutFactor(0.1);
        m_data->m_fadeOutAnimation->stop();
    }
    if (m_data->m_player->getMediaMeta().localPath.isEmpty()) {
        forcePlay();
    } else {
        m_data->m_player->play();
    }
    if (m_data->m_fadeInOut && m_data->m_fadeInAnimation->state() != QPropertyAnimation::Running) {
        m_data->m_fadeInAnimation->start();
    }
}

void PlayerEngine::playPreMeta()
{
    MediaMeta curMeta = m_data->m_player->getMediaMeta();
    QList<MediaMeta> &allMetas = m_data->m_metaList;
    // 歌单顺序播放
    if (m_data->m_playbackMode == DmGlobal::RepeatNull) {
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
            setMediaMeta(allMetas.at(preIndex));
            play();
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
        setMediaMeta(curMetaList.at(index));
        play();
    }
}

void PlayerEngine::playNextMeta(const DMusic::MediaMeta &meta, bool isAuto, bool playFlag)
{
    QList<MediaMeta> &allMetas = m_data->m_metaList;
    // 歌单顺序播放
    if (m_data->m_playbackMode == DmGlobal::RepeatNull) {
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
        if (newIndex != -1) {
            setMediaMeta(allMetas.at(newIndex));
            if (playFlag)
                play();
        } else if (!isAuto) {
            for (int i = 0; i < allMetas.size(); i++) {
                if ((QFile::exists(allMetas[i].localPath) && m_data->m_player->supportedSuffixList().contains(
                            QFileInfo(allMetas[i].localPath).suffix().toLower())) || allMetas[i].mmType == DmGlobal::MimeTypeCDA) {
                    index = i;
                    break;
                }
            }
            if (index != -1) {
                setMediaMeta(allMetas[index]);
                if (playFlag)
                    play();
            }
        } else {
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
            if (index == (curMetaList.size() - 1)) {
                index = 0;
            } else {
                index++;
            }
            break;
        }
        case DmGlobal::RepeatSingle: {
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
        setMediaMeta(curMetaList.at(index).second);
        if (playFlag)
            play();
    } else {
        stop();
    }
}

void PlayerEngine::resetDBusMpris(const DMusic::MediaMeta &meta)
{
    QVariantMap metadata;
    metadata.insert(Mpris::metadataToString(Mpris::Title), meta.title);
    metadata.insert(Mpris::metadataToString(Mpris::Artist), meta.artist);
    metadata.insert(Mpris::metadataToString(Mpris::Album), meta.album);
    metadata.insert(Mpris::metadataToString(Mpris::Length), meta.length);
    QString str = DmGlobal::cachePath() + "/images/" + meta.hash + ".jpg";
    QFileInfo info(str);
    if (!info.exists()) {
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
}

void PlayerEngine::playNextMeta(bool isAuto, bool playFlag)
{
    MediaMeta curMeta = m_data->m_player->getMediaMeta();

    playNextMeta(curMeta, isAuto, playFlag);
}

void PlayerEngine::stop()
{
    m_data->m_player->stop();
    setMediaMeta(MediaMeta());
}

void PlayerEngine::setFadeInOutFactor(double fadeInOutFactor)
{
    m_data->m_fadeInOutFactor = fadeInOutFactor;
    m_data->m_player->blockSignals(true);
    m_data->m_player->setPreamplification(static_cast<float>(12 * m_data->m_fadeInOutFactor));
    m_data->m_player->blockSignals(false);
}

int PlayerEngine::length()
{
    return m_data->m_player->length();
}

void PlayerEngine::setTime(qint64 time)
{
    if (INT_LAST_PROGRESS_FLAG == 1)
        INT_LAST_PROGRESS_VALUE = time;
    else {
        m_data->m_player->setTime(time);
    }
}

qint64 PlayerEngine::time()
{
    return INT_LAST_PROGRESS_FLAG == 1 ? INT_LAST_PROGRESS_VALUE : m_data->m_player->time();
}

void PlayerEngine::setVolume(int volume)
{
    int curVolume = volume <= 0 ? 0 : volume;
    if (m_data->m_player->getVolume() == curVolume) {
        emit volumeChanged(m_data->m_player->getVolume());
    } else {
        m_data->m_player->setVolume(curVolume);
        emit volumeChanged(m_data->m_player->getVolume());
        setMute(curVolume == 0);
    }
}

int PlayerEngine::getVolume()
{
    return m_data->m_player->getVolume();
}

void PlayerEngine::setMute(bool mute)
{
    if (m_data->m_player->getMute() == mute) {
        emit muteChanged(mute);
    } else {
        m_data->m_player->setMute(mute);
        emit muteChanged(mute);
    }
}

bool PlayerEngine::getMute()
{
    return m_data->m_player->getMute();
}

DmGlobal::PlaybackStatus PlayerEngine::playbackStatus()
{
    return m_data->m_player->state();
}

void PlayerEngine::setPlaybackMode(DmGlobal::PlaybackMode mode)
{
    m_data->m_playbackMode = mode;
}

DmGlobal::PlaybackMode PlayerEngine::getPlaybackMode()
{
    return m_data->m_playbackMode;
}

void PlayerEngine::setCurrentPlayList(const QString &playlistHash)
{
    m_data->m_playlistHash = playlistHash;
}

QString PlayerEngine::getCurrentPlayList()
{
    return m_data->m_playlistHash;
}

QList<MediaMeta> PlayerEngine::getCdaMetaInfo()
{
    return m_data->m_player->getCdaMetaInfo();
}

void PlayerEngine::setEqualizerEnabled(bool enabled)
{
    m_data->m_player->setEqualizerEnabled(enabled);
}

void PlayerEngine::loadFromPreset(uint index)
{
    m_data->m_player->loadFromPreset(index);
}

void PlayerEngine::setPreamplification(float value)
{
    m_data->m_player->setPreamplification(value);
}

void PlayerEngine::setAmplificationForBandAt(float amp, uint bandIndex)
{
    m_data->m_player->setAmplificationForBandAt(amp, bandIndex);
}

float PlayerEngine::amplificationForBandAt(uint bandIndex)
{
    return m_data->m_player->amplificationForBandAt(bandIndex);
}

float PlayerEngine::preamplification()
{
    return m_data->m_player->preamplification();
}
