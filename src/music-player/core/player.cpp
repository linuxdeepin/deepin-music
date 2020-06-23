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
#include "AudioBufferDevice.h"
#include "AudioPlayer.h"

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

#include <DRecentManager>

#include "metasearchservice.h"
#include "util/dbusutils.h"
#include <unistd.h>

DCORE_USE_NAMESPACE

static QMap<QString, bool>  sSupportedSuffix;
static QStringList          sSupportedSuffixList;
static QStringList          sSupportedFiterList;
static QStringList          sSupportedMimeTypes;

static const int sFadeInOutAnimationDuration = 900; //ms

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
        qplayer = new QMediaPlayer();
        qplayer->setVolume(100);
        qProbe = new QAudioProbe();
        /*-------AudioPlayer-------*/
        ioPlayer  =  new AudioPlayer();
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
    QString sinkInputPath;

    Player::PlaybackMode    mode    = Player::RepeatAll;
    Player::PlaybackStatus  status  = Player::InvalidPlaybackStatus;


    QMediaPlayer    *qplayer;
    QAudioProbe     *qProbe;
    /*-------ioPlayer----------*/
    AudioPlayer  *ioPlayer;
    qint64 ioDuration = 0;

    PlaylistPtr     activePlaylist;
    PlaylistPtr     curPlaylist;
    MetaPtr         activeMeta;

    int             volume      = 50.0;
    bool            playOnLoad  = true;
    bool            firstPlayOnLoad  = true; //外部双击打开处理一次
    bool            fadeInOut   = true;
    double          fadeInOutFactor     = 1.0;
    qlonglong       m_position          = 0.0;//只能用于判断音乐是否正常结束

    QPropertyAnimation  *fadeInAnimation    = nullptr;
    QPropertyAnimation  *fadeOutAnimation   = nullptr;

//    QFileSystemWatcher  fileSystemWatcher;

    Player *q_ptr;
    Q_DECLARE_PUBLIC(Player)
};

void PlayerPrivate::initConnection()
{
    Q_Q(Player);

    /*----------ioPlayer connect-----------*/
    q->connect(ioPlayer->_buffer, &AudioBufferDevice::positionChanged, q,
    [ = ](qint64 position) {

        //qDebug() << position << "-" << ioDuration;
        Q_EMIT q->positionChanged(position, ioDuration, 20);
    });

    q->connect(ioPlayer->_buffer, &AudioBufferDevice::durationChanged, q,
    [ = ](qint64 position) {
        Q_UNUSED(position)
        ioDuration++;
    });

    q->connect(ioPlayer->_buffer, &AudioBufferDevice::endOfMedia, q,
    [ = ]() {
        qDebug() << "AudioBufferDevice::endOfMedia";

        ioPlayer->reset();
        selectNext(activeMeta, mode);
    });


    q->connect(ioPlayer->_buffer, &AudioBufferDevice::againMedia, q,
    [ = ]() {
        //! 重新加载资源
        if (playOnLoad && (!activeMeta.isNull()) && QFile::exists(activeMeta->localPath)) {

            ioDuration = 0;

            QString temp = activeMeta->localPath;
            if (temp.endsWith(".amr1")) {
                qplayer->stop();
                ioPlayer->play();
                ioPlayer->setSourceFilename(activeMeta->localPath);
            }
        }
    });


    q->connect(q, &Player::sliderReleased, ioPlayer->_buffer, &AudioBufferDevice::sliderReleased);

    /*--------------END ioPlayer------------*/

    qplayer->setAudioRole(QAudio::MusicRole);


    q->connect(qplayer, &QMediaPlayer::positionChanged,
    q, [ = ](qint64 position) {
        if (activeMeta.isNull()) {
            return;
        }

        auto duration = qplayer->duration();

        if (position > 1 && activeMeta->invalid) {
            qDebug() << " ======> position > 1 && activeMeta->invalid";
            Q_EMIT q->mediaError(activePlaylist, activeMeta, Player::NoError);
        }

        /*----fix len----*/
        if (activeMeta->length == 0 && duration != 0 && duration > 0) {
            activeMeta->length = duration;
            qDebug() << "update" << activeMeta->length;
            Q_EMIT q->mediaUpdate(activePlaylist, activeMeta);
        }

        if (position >= activeMeta->offset + activeMeta->length + 1800 && qplayer->state() == QMediaPlayer::PlayingState) {
            qDebug() << "WARN!!! change to next by position change";
            QTimer::singleShot(10, [ = ]() {
                selectNext(activeMeta, mode);
            });
            return;
        }

        Q_EMIT q->positionChanged(position - activeMeta->offset,  activeMeta->length, 1);
    });

    q->connect(qplayer, &QMediaPlayer::stateChanged,
    q, [ = ](QMediaPlayer::State newState) {

        ioPlayer->reset();

        switch (newState) {
        case QMediaPlayer::StoppedState:
            Q_EMIT q->playbackStatusChanged(Player::Stopped);
            break;
        case QMediaPlayer::PlayingState:
            Q_EMIT q->playbackStatusChanged(Player::Playing);
            break;
        case QMediaPlayer::PausedState:
            Q_EMIT q->playbackStatusChanged(Player::Paused);
            break;
        }
    });

    q->connect(qplayer, &QMediaPlayer::volumeChanged,
    q, [ = ](int volume) {
        if (fadeInOutFactor < 1.0) {
            return;
        }
        Q_EMIT q->volumeChanged(volume);
    });
    q->connect(qplayer, &QMediaPlayer::mutedChanged,
               q, &Player::mutedChanged);
    q->connect(qplayer, &QMediaPlayer::durationChanged,
               q, &Player::durationChanged);

    q->connect(qplayer, &QMediaPlayer::mediaStatusChanged,
    q, [ = ](QMediaPlayer::MediaStatus status) {
        switch (status) {
        case QMediaPlayer::LoadedMedia: {
            //wtf the QMediaPlayer can play image format, 233333333
            if (activeMeta.isNull()) {
                qplayer->pause();
                qplayer->stop();
                return;
            }
            QMimeDatabase db;
            QMimeType type = db.mimeTypeForFile(activeMeta->localPath, QMimeDatabase::MatchContent);
            if (!sSupportedMimeTypes.contains(type.name())) {
                qDebug() << "unsupported mime type" << type << activePlaylist << activeMeta;
                qplayer->pause();
                qplayer->stop();
                qDebug() << "===========> QMediaPlayer::LoadedMedia";
                Q_EMIT q->mediaError(activePlaylist, activeMeta, Player::FormatError);
                return;
            }

            if (playOnLoad && QFile::exists(activeMeta->localPath)) {

                ioDuration = 0;

                QString temp = activeMeta->localPath;
                if (temp.endsWith(".amr1")) {
                    qplayer->stop();

                    ioPlayer->play();

                    ioPlayer->setSourceFilename(activeMeta->localPath);

                } else {
                    qplayer->play();
                }
            }
            break;
        }
        case QMediaPlayer::EndOfMedia: {

            selectNext(activeMeta, mode);
            break;
        }

        case QMediaPlayer::LoadingMedia: {
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
        if (error == QMediaPlayer::ResourceError) {
            if (!activeMeta.isNull() && !QFile::exists(activeMeta->localPath)) {
                MetaPtrList removeMusicList;
                removeMusicList.append(activeMeta);
                curPlaylist->removeMusicList(removeMusicList);
                Q_EMIT q->mediaError(activePlaylist, activeMeta, static_cast<Player::Error>(error));
            } else {
                qDebug() << "#####QMediaPlayer::error";
                qplayer->pause();
                qlonglong postion = qplayer->position();
                sleep(1);
                qplayer->setPosition(postion);
                qplayer->play();
            }

            /*else {
                QFileInfo fi("activeMeta->localPath");
                if (!fi.isReadable()) {
                    MetaPtrList removeMusicList;
                    removeMusicList.append(activeMeta);
                    curPlaylist->removeMusicList(removeMusicList);
                    Q_EMIT q->mediaError(activePlaylist, activeMeta, static_cast<Player::Error>(error));
                }
            }*/
        }
    });

    /*
        q->connect(&fileSystemWatcher, &QFileSystemWatcher::fileChanged,
        q, [ = ](const QString & path) {
            if (!QFile::exists(activeMeta->localPath) && !activePlaylist->allmusic().isEmpty()) {
                 qDebug() << "change " << path;
                qplayer->pause();
                qplayer->stop();
                Q_EMIT q->mediaError(activePlaylist, activeMeta, Player::ResourceError);
            }
        });
      */
}

void PlayerPrivate::selectNext(const MetaPtr info, Player::PlaybackMode mode)
{
    Q_Q(Player);
    if (!curPlaylist || curPlaylist->isEmpty()) {
        return;
    }

    bool invalidFlag = info->invalid;
    if (invalidFlag) {
        for (auto curMeta : curPlaylist->allmusic()) {
            if (!curMeta->invalid) {
                invalidFlag = false;
                break;
            }
        }
    }


    switch (mode) {
    case Player::RepeatAll: {
        auto curMeta = curPlaylist->next(info);
        if (QFile::exists(curMeta->localPath)) {
            curMeta->invalid = false;
        }
        if (curMeta->invalid && !invalidFlag) {
            int curNum = 0;
            while (curNum < curPlaylist->allmusic().size()) {
                curMeta = curPlaylist->next(curMeta);
                if (!curMeta->invalid)
                    break;
            }
        }
        q->playMeta(activePlaylist, curMeta);
        break;
    }
    case Player::RepeatSingle: {
        q->playMeta(activePlaylist, info);
        break;
    }
    case Player::Shuffle: {
        auto curMeta = curPlaylist->shuffleNext(info);
        if (QFile::exists(curMeta->localPath)) {
            curMeta->invalid = false;
        }
        if (curMeta->invalid && !invalidFlag) {
            while (true) {
                curMeta = curPlaylist->shuffleNext(curMeta);
                if (!curMeta->invalid || QFile::exists(curMeta->localPath))
                    break;
            }
        }
        q->playMeta(activePlaylist, curMeta);
        break;
    }
    }
}

void PlayerPrivate::selectPrev(const MetaPtr info, Player::PlaybackMode mode)
{
    Q_Q(Player);
    if (!curPlaylist || curPlaylist->isEmpty()) {
        return;
    }

    bool invalidFlag = info->invalid;
    if (invalidFlag) {
        for (auto curMeta : curPlaylist->allmusic()) {
            if (!curMeta->invalid) {
                invalidFlag = false;
                break;
            }
        }
    }

    switch (mode) {
    case Player::RepeatAll: {
        auto curMeta = curPlaylist->prev(info);
        if (QFile::exists(curMeta->localPath)) {
            curMeta->invalid = false;
        }
        if (curMeta->invalid && !invalidFlag) {
            int curNum = 0;
            while (curNum < curPlaylist->allmusic().size()) {
                curMeta = curPlaylist->prev(curMeta);
                if (!curMeta->invalid || QFile::exists(curMeta->localPath))
                    break;
            }
        }
        q->playMeta(activePlaylist, curMeta);
        break;
    }
    case Player::RepeatSingle: {
        q->playMeta(activePlaylist, info);
        break;
    }
    case Player::Shuffle: {
        auto curMeta = curPlaylist->shufflePrev(info);
        if (QFile::exists(curMeta->localPath)) {
            curMeta->invalid = false;
        }
        if (curMeta->invalid && !invalidFlag) {
            while (true) {
                curMeta = curPlaylist->shufflePrev(curMeta);
                if (!curMeta->invalid || QFile::exists(curMeta->localPath))
                    break;
            }
        }
        q->playMeta(activePlaylist, curMeta);
        break;
    }
    }
}

Player::Player(QObject *parent) : QObject(parent), d_ptr(new PlayerPrivate(this))
{
    initMiniTypes();
}

void Player::init()
{
    Q_D(Player);
    qRegisterMetaType<Player::Error>();
    qRegisterMetaType<Player::PlaybackStatus>();

    d->initConnection();
}

void Player::setActivePlaylist(PlaylistPtr playlist)
{
    Q_D(Player);
    d->activePlaylist = playlist;
}

void Player::setCurPlaylist(PlaylistPtr curPlaylist)
{
    Q_D(Player);
    d->curPlaylist = curPlaylist;
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
    if (playlist->id() != PlayMusicListID)
        d->activePlaylist = playlist;

    d->qplayer->blockSignals(true);
    d->qplayer->setMedia(QMediaContent(QUrl::fromLocalFile(meta->localPath)));
    int volume = d->qplayer->volume();
    d->qplayer->setVolume(0);
    if (!d->activePlaylist.isNull())
        d->activePlaylist->play(meta);
    d->qplayer->play();
    QTimer::singleShot(100, this, [ = ]() {//为了记录进度条生效，在加载的时候让音乐播放100ms
        d->qplayer->pause();
        d->qplayer->setVolume(volume);
        d->qplayer->blockSignals(false);
        if (!d->activePlaylist.isNull())
            d->activePlaylist->play(meta);
    });
}

void Player::playMeta(PlaylistPtr playlist, const MetaPtr meta)
{
    Q_D(Player);
    if (QFileInfo(meta->localPath).dir().isEmpty()) {
        Q_EMIT mediaError(playlist, meta, Player::ResourceError);
        return ;
    }

    MetaPtr curMeta = meta;
    if (curMeta == nullptr)
        curMeta = d->curPlaylist->first();
    qDebug() << "playMeta"
             << curMeta->title
             << DMusic::lengthString(curMeta->offset) << "/"
             << DMusic::lengthString(curMeta->length);

    if (curMeta.isNull())
        return;

    if (playlist->id() != PlayMusicListID)
        d->activePlaylist = playlist;

//    if (d->activePlaylist.isNull())
//        return;

    d->activeMeta = curMeta;
    d->qplayer->setMedia(QMediaContent(QUrl::fromLocalFile(curMeta->localPath)));
    d->qplayer->setPosition(curMeta->offset);
    if (!d->activePlaylist.isNull())
        d->activePlaylist->play(curMeta);
    d->curPlaylist->play(curMeta);

    DRecentData data;
    data.appName = "Music";
    data.appExec = "deepin-music";
    DRecentManager::addItem(curMeta->localPath, data);

    if (!d->activePlaylist.isNull()) {
        Q_EMIT mediaPlayed(d->activePlaylist, d->activeMeta);
    } else {
        Q_EMIT mediaPlayed(d->curPlaylist, d->activeMeta);
    }

//    if (d->qplayer->mediaStatus() == QMediaPlayer::BufferedMedia) {
//        QTimer::singleShot(100, this, [ = ]() {

//            d->qplayer->play();
//        });
//    }

    if (d->firstPlayOnLoad == true) {
        d->firstPlayOnLoad = false;
        QTimer::singleShot(150, this, [ = ]() {
            d->qplayer->play();
        });
    }



    if (d->fadeOutAnimation) {
        d->fadeOutAnimation->stop();
        d->fadeOutAnimation->deleteLater();
        d->fadeOutAnimation = nullptr;
    }
    if (d->fadeInOut && !d->fadeInAnimation) {
        qDebug() << "start fade in";
        d->fadeInAnimation = new QPropertyAnimation(this, "fadeInOutFactor");
        d->fadeInAnimation->setEasingCurve(QEasingCurve::InCubic);
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

void Player::resume(PlaylistPtr playlist, const MetaPtr meta)
{
    Q_D(Player);
    if (meta == nullptr) {
        return;
    }

    if (QFileInfo(meta->localPath).dir().isEmpty()) {
        Q_EMIT mediaError(playlist, meta, Player::ResourceError);
        return ;
    }
    if (d->fadeOutAnimation) {
        setFadeInOutFactor(1.0);
        d->fadeOutAnimation->stop();
        d->fadeOutAnimation->deleteLater();
        d->fadeOutAnimation = nullptr;
    }


    qDebug() << "resume top";
    if (playlist == d->activePlaylist && d->qplayer->state() == QMediaPlayer::PlayingState && meta->hash == d->activeMeta->hash)
        return;

    d->activeMeta = meta;
    if (d->curPlaylist != nullptr)
        d->curPlaylist->play(meta);
    setPlayOnLoaded(true);
    //增大音乐自动开始播放时间，给setposition留足空间
    QTimer::singleShot(100, this, [ = ]() {
        QString temp = meta->localPath;
        if (temp.endsWith(".amr1")) {
            d->ioPlayer->play();
        } else {
            d->qplayer->play();
        }
    });

    if (d->fadeInOut && !d->fadeInAnimation) {
        d->fadeInAnimation = new QPropertyAnimation(this, "fadeInOutFactor");
        d->fadeInAnimation->setEasingCurve(QEasingCurve::InCubic);
        d->fadeInAnimation->setStartValue(0.1000);
        d->fadeInAnimation->setEndValue(1.0000);
        d->fadeInAnimation->setDuration(sFadeInOutAnimationDuration);
        connect(d->fadeInAnimation, &QPropertyAnimation::finished,
        this, [ = ]() {
            d->fadeInAnimation->deleteLater();
            d->fadeInAnimation = nullptr;
        });
        d->fadeInAnimation->start();
    }

    if (!d->activePlaylist.isNull() && d->activePlaylist->contains(d->activeMeta)) {
        Q_EMIT mediaPlayed(d->activePlaylist, d->activeMeta);
    } else {
        Q_EMIT mediaPlayed(d->curPlaylist, d->activeMeta);
    }
}

void Player::playNextMeta(PlaylistPtr playlist, const MetaPtr meta)
{
    Q_UNUSED(playlist)
    Q_D(Player);
//    Q_ASSERT(playlist == d->activePlaylist);

    setPlayOnLoaded(true);
    if (d->mode == RepeatSingle) {
        d->selectNext(meta, RepeatAll);
    } else {
        d->selectNext(meta, d->mode);
    }
}

void Player::playPrevMusic(PlaylistPtr playlist, const MetaPtr meta)
{
    Q_UNUSED(playlist)
    Q_D(Player);
//    Q_ASSERT(playlist == d->activePlaylist);

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

    /*--------suspend--------*/
    d->ioPlayer->suspend();

    if (d->fadeInAnimation) {

        d->fadeInAnimation->stop();
        d->fadeInAnimation->deleteLater();
        d->fadeInAnimation = nullptr;
    }

    if (d->fadeInOut && !d->fadeOutAnimation) {

        d->fadeOutAnimation = new QPropertyAnimation(this, "fadeInOutFactor");
        d->fadeOutAnimation->setEasingCurve(QEasingCurve::OutCubic);
        d->fadeOutAnimation->setStartValue(1.0000);
        d->fadeOutAnimation->setEndValue(0.1000);
        d->fadeOutAnimation->setDuration(sFadeInOutAnimationDuration);
        connect(d->fadeOutAnimation, &QPropertyAnimation::finished,
        this, [ = ]() {
            d->fadeOutAnimation->deleteLater();
            d->fadeOutAnimation = nullptr;
            d->qplayer->pause();
            setFadeInOutFactor(1.0);
        });
        d->fadeOutAnimation->start();
    } else {
        d->qplayer->pause();
        setFadeInOutFactor(1.0);
    }
}

void Player::pauseNow()
{
    Q_D(Player);
    d->qplayer->pause();
}

void Player::stop()
{
    Q_D(Player);

    d->qplayer->pause();
    d->qplayer->setMedia(QMediaContent());
    d->activeMeta.clear(); //清除当前播放音乐；
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

PlaylistPtr Player::curPlaylist() const
{
    Q_D(const Player);
    return d->curPlaylist;
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

bool Player::muted()
{
    //Q_D(const Player);
    //return d->qplayer->isMuted();
    return this->isMusicMuted();
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


void Player::setIOPosition(qint64 value, qint64 range)
{
    Q_UNUSED(range)
    Q_D(Player);

    if (d->playOnLoad && d->activeMeta && QFile::exists(d->activeMeta->localPath)) {

        QString temp = d->activeMeta->localPath;

        if (temp.endsWith(".amr1")) {

            if (value != 0 && d->ioDuration != 0) {
                // qint64 position =  (value * d->ioDuration) / range;
                qint64 position = (value * d->ioDuration) / 1000;
                Q_EMIT this->sliderReleased(position);
            }
        }
    }
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

void Player::setVolume(int volume)
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
    //d->qplayer->setVolume(d->volume * d->fadeInOutFactor);
    d->qplayer->blockSignals(false);

    setMusicVolume((volume + 0.1) / 100.0);//设置到dbus的音量必须大1，设置才会生效
}

void Player::setMuted(bool mute)
{
    //Q_D(Player);
    //d->qplayer->setMuted(mute);
    setMusicMuted(mute);
}

void Player::setFadeInOutFactor(double fadeInOutFactor)
{
    Q_D(Player);
    d->fadeInOutFactor = fadeInOutFactor;
//    qDebug() << "setFadeInOutFactor" << fadeInOutFactor
//             << d->volume *d->fadeInOutFactor << d->volume;
    d->qplayer->blockSignals(true);
    d->qplayer->setVolume(/*d->volume*/static_cast<int>(100 * d->fadeInOutFactor));
    d->qplayer->blockSignals(false);

    //setMusicVolume(d->volume * d->fadeInOutFactor / 100.0);
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

void Player::musicFileMiss()
{
    Q_D(Player);

    /*--------Remove the usb flash drive, the music is invalid-------*/
    if (d->activeMeta != nullptr && access(d->activeMeta->localPath.toStdString().c_str(), F_OK) != 0 && (!d->activePlaylist->allmusic().isEmpty())) {
        stop();

        //Q_EMIT mediaError(d->activePlaylist, d->activeMeta, Player::ResourceError);

        d->activeMeta = nullptr;
        d->activePlaylist->play(MetaPtr());
    }
}

void Player::readSinkInputPath()
{
    Q_D(Player);
//    if (!d->sinkInputPath.isEmpty())
//        return;
    QVariant v = DBusUtils::redDBusProperty("com.deepin.daemon.Audio", "/com/deepin/daemon/Audio",
                                            "com.deepin.daemon.Audio", "SinkInputs");

    if (!v.isValid())
        return;

    QList<QDBusObjectPath> allSinkInputsList = v.value<QList<QDBusObjectPath> >();
//    qDebug() << "allSinkInputsListSize: " << allSinkInputsList.size();

    for (auto curPath : allSinkInputsList) {
//        qDebug() << "path: " << curPath.path();

        QVariant nameV = DBusUtils::redDBusProperty("com.deepin.daemon.Audio", curPath.path(),
                                                    "com.deepin.daemon.Audio.SinkInput", "Name");

        if (!nameV.isValid() || nameV.toString() != "Music")
            continue;

        d->sinkInputPath = curPath.path();
        break;
    }
}

bool Player::setMusicVolume(double volume)
{
    if (volume > 1.0) {
        volume = 1.000;
    }
    Q_D(Player);
    readSinkInputPath();

    if (!d->sinkInputPath.isEmpty()) {
        QDBusInterface ainterface("com.deepin.daemon.Audio", d->sinkInputPath,
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
    Q_D(Player);
    readSinkInputPath();
    if (!d->sinkInputPath.isEmpty()) {
        QDBusInterface ainterface("com.deepin.daemon.Audio", d->sinkInputPath,
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
    Q_D(Player);
    readSinkInputPath();

    if (!d->sinkInputPath.isEmpty()) {
        QVariant MuteV = DBusUtils::redDBusProperty("com.deepin.daemon.Audio", d->sinkInputPath,
                                                    "com.deepin.daemon.Audio.SinkInput", "Mute");

        if (!MuteV.isValid()) {
            return false;
        }

        return MuteV.toBool();
    }

    return false;
}
