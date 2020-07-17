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
#include <QProcess>
#include <DRecentManager>
#include <QMutex>
#include <QMediaContent>

#include "metasearchservice.h"
#include "util/dbusutils.h"
#include <unistd.h>


#include <vlc/vlc.h>
#include "vlc/Audio.h"
#include "vlc/Error.h"
#include "vlc/Common.h"
#include "vlc/Enums.h"
#include "vlc/Instance.h"
#include "vlc/Media.h"
#include "vlc/MediaPlayer.h"
#include "util/global.h"

DCORE_USE_NAMESPACE

static QMap<QString, bool>  sSupportedSuffix;
static QStringList          sSupportedSuffixList;
static QStringList          sSupportedFiterList;
static QStringList          sSupportedMimeTypes;

static const int sFadeInOutAnimationDuration = 900; //ms

extern void apeToMp3(QString path, QString hash);
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

        qvinstance = new VlcInstance(VlcCommon::args(), nullptr);
        qvplayer = new VlcMediaPlayer(qvinstance);
        qvmedia = new VlcMedia();
//        qvplayer->audio()->setVolume(100);

    }

    void initConnection();
    void selectPrev(const MetaPtr info, Player::PlaybackMode mode);
    void selectNext(const MetaPtr info, Player::PlaybackMode mode);
    //void apeToMp3(QString path, QString hash);
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


    VlcInstance *qvinstance;
    VlcMedia *qvmedia;
    VlcMediaPlayer *qvplayer;
    bool isamr = false;
    bool ischangeMusic = false;
    QMutex mutex;

    PlaylistPtr     activePlaylist;
    PlaylistPtr     curPlaylist;
    MetaPtr         activeMeta;

    int             startSameMusic   = 1; //双击启动是否同一首歌
    int             volume      = 50.0;
    bool            playOnLoad  = true;
    bool            firstPlayOnLoad  = true; //外部双击打开处理一次
    bool            fadeInOut   = true;
    double          fadeInOutFactor     = 1.0;
    qlonglong m_position = -1; //用于音乐暂停时的播放位置

    QPropertyAnimation  *fadeInAnimation    = nullptr;
    QPropertyAnimation  *fadeOutAnimation   = nullptr;

//    QFileSystemWatcher  fileSystemWatcher;

    Player *q_ptr;
    Q_DECLARE_PUBLIC(Player)
};

void apeToMp3(QString path, QString hash)
{
    QFileInfo fileInfo(path);
    if (fileInfo.suffix().toLower() == "ape") {
        QString curPath = Global::cacheDir();
        QString toPath = QString("%1/images/%2.mp3").arg(curPath).arg(hash + "tmp");
        if (QFile::exists(toPath)) {
            QFile::remove(toPath);
        }
        QString fromPath = QString("%1/.tmp1.ape").arg(curPath);
        QFile::remove(fromPath);
        QFile file(path);
        file.link(fromPath);
        QString program = QString("ffmpeg -i %1  -ac 1 -ab 32 -ar 24000 %2").arg(fromPath).arg(toPath);
        QProcess::execute(program);
    } else if (fileInfo.suffix().toLower() == "amr") {
        QString curPath = Global::cacheDir();
        QString toPath = QString("%1/images/%2.mp3").arg(curPath).arg(hash);
        if (QFile::exists(toPath)) {
            QFile::remove(toPath);
        }
        QString fromPath = QString("%1/.tmp1.amr").arg(curPath);
        QFile::remove(fromPath);
        QFile file(path);
        file.link(fromPath);
        QString program = QString("ffmpeg -i %1  -ac 1 -ab 32 -ar 24000 %2").arg(fromPath).arg(toPath);
        QProcess::execute(program);
    } else {
        //do nothing
    }
}
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

//        ioPlayer->reset();
//        selectNext(activeMeta, mode);
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

    //vlc timeChanged to show
    q->connect(qvplayer, &VlcMediaPlayer::timeChanged,
    q, [ = ](qint64 position) {
        if (activeMeta.isNull()) {
            return;
        }

        auto duration = qvplayer->time();

        if (position > 1 && activeMeta->invalid) {
            Q_EMIT q->mediaError(activePlaylist, activeMeta, Player::NoError);
        }

        if (activeMeta->length == 0 && duration != 0 && duration > 0) {
            activeMeta->length = duration;
            Q_EMIT q->mediaUpdate(activePlaylist, activeMeta);
        }

        if (position >= activeMeta->offset + activeMeta->length + 1800 && qvplayer->state() == Vlc::Playing) {
            qDebug() << "WARN!!! change to next by position change";
            QTimer::singleShot(10, [ = ]() {
                selectNext(activeMeta, mode);
            });
            return;
        }

        Q_EMIT q->positionChanged(position - activeMeta->offset,  activeMeta->length, 1);
    });

    //        Idle,
    //        Opening,
    //        Buffering,
    //        Playing,
    //        Paused,
    //        Stopped,
    //        Ended,
    //        Error
    //vlc stateChanged
    q->connect(qvmedia, &VlcMedia::stateChanged,
    q, [ = ](Vlc::State status) {
        if (isamr) {
            switch (status) {
            case Vlc::Idle: {
                break;
            }
            case Vlc::Opening: {
                break;
            }
            case Vlc::Buffering: {
                break;
            }
            case Vlc::Playing: {
                Q_EMIT q->playbackStatusChanged(Player::Playing);
                break;
            }
            case Vlc::Paused: {
                Q_EMIT q->playbackStatusChanged(Player::Paused);
                break;
            }
            case Vlc::Stopped: {
                Q_EMIT q->playbackStatusChanged(Player::Stopped);
                break;
            }
            case Vlc::Ended: {
                //Processed short audio files
                if (qvplayer->time() != 0) {
//                    qDebug() << qvplayer->time() << qvplayer->length();
                    selectNext(activeMeta, mode);
                }
                break;
            }
            case Vlc::Error: {
                if (!activeMeta.isNull() && !QFile::exists(activeMeta->localPath)) {
                    MetaPtrList removeMusicList;
                    removeMusicList.append(activeMeta);
                    curPlaylist->removeMusicList(removeMusicList);
                    Q_EMIT q->mediaError(activePlaylist, activeMeta, Player::ResourceError);
                }
                break;
            }

            }
        }

    });



    q->connect(qplayer, &QMediaPlayer::stateChanged,
    q, [ = ](QMediaPlayer::State newState) {
        Q_UNUSED(newState)
//        ioPlayer->reset();
#if 0
        switch (newState) {
            if (!isamr) {   //warning: code will never be exetuted
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
        }
#endif

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
            if (!isamr)
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
            /*else {//pause Audio报错，m4a文件没有问题
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
//            int curNum = 0;
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
//            int curNum = 0;
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


    d->fadeOutAnimation = new QPropertyAnimation(this, "fadeInOutFactor");
    d->fadeInAnimation = new QPropertyAnimation(this, "fadeInOutFactor");
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
    Q_D(Player);
    d->qplayer->stop();
    d->qplayer->deleteLater();

    delete d->qvmedia;
    delete d->qvplayer;
    delete d->qvinstance;
    delete d->fadeOutAnimation;
    delete d->fadeInAnimation;
}


void Player::loadMedia(PlaylistPtr playlist, const MetaPtr meta, int position)
{
    qDebug() << "loadMedia"
             << meta->title
             << DMusic::lengthString(meta->offset) << "/"
             << DMusic::lengthString(meta->length) << "|"
             << position;
    Q_D(Player);
    d->activeMeta = meta;
    if (playlist->id() != PlayMusicListID)
        d->activePlaylist = playlist;

    int volume = -1;
    d->qplayer->blockSignals(true);
    d->isamr = false;

    QString curPath = Global::cacheDir();
    QString toPath = QString("%1/images/%2.mp3").arg(curPath).arg(meta->hash);
    if (!QFile::exists(toPath)) {
        Q_EMIT addApeTask(meta->localPath, meta->hash);
        if (!QFile::exists(toPath)) {
            toPath = meta->localPath;
        }
    }
    d->qplayer->setMedia(QMediaContent(QUrl::fromLocalFile(toPath)));
    volume = d->qplayer->volume();
    d->qplayer->setVolume(0);
    d->qplayer->play();

    if (!d->activePlaylist.isNull())
        d->activePlaylist->play(meta);

    if (position == 0) { //do not care process
        QTimer::singleShot(100, this, [ = ]() {//为了记录进度条生效，在加载的时候让音乐播放100ms
            if (d->isamr) {
                d->qvplayer->pause();
            } else {
                d->qplayer->pause();
                if (volume == 0) {
                    d->qplayer->setVolume(100);
                } else {
                    d->qplayer->setVolume(volume);
                }
            }
            d->qplayer->blockSignals(false);
            d->qplayer->setPosition(position); //set position
            /*************************
             * it does't matter whether it resumes
             * **********************/
            emit readyToResume();
            if (!d->activePlaylist.isNull())
                d->activePlaylist->play(meta);
        });
    } else {
        QTimer *pt = new QTimer;
        pt->start(150);
        pt->setInterval(1000);

        connect(pt, &QTimer::timeout, this, [ = ]() {
            if (d->qplayer->isSeekable()) {
                if (d->isamr) {
                    d->qvplayer->pause();
                } else {
                    d->qplayer->pause();
                    if (volume == 0) {
                        d->qplayer->setVolume(100);
                    } else {
                        d->qplayer->setVolume(volume);
                    }
                }
                d->qplayer->blockSignals(false);
                if (!d->activePlaylist.isNull())
                    d->activePlaylist->play(meta);

                d->canPlay = true;
                switch (d->startSameMusic) {
                case 1:
                    d->qplayer->setPosition(position); //set position
                    emit readyToResume();
                    break;
                case 2:
                    emit playerReady();
                    break;
                case 3:
                    d->qplayer->setPosition(position); //set position
                    emit playerReady(); //the same music
                    break;
                default:
                    d->qplayer->setPosition(position); //set position
                    emit readyToResume();
                    break;
                }

                pt->stop();
                pt->deleteLater();
            }

            if (pt->interval() >= 1000) {
                if (d->isamr) {
                    d->qvplayer->pause();
                } else {
                    d->qplayer->pause();
                    if (volume == 0) {
                        d->qplayer->setVolume(100);
                    } else {
                        d->qplayer->setVolume(volume);
                    }
                }
                d->qplayer->blockSignals(false);
                if (!d->activePlaylist.isNull())
                    d->activePlaylist->play(meta);

                d->canPlay = true;
                switch (d->startSameMusic) {
                case 1:
                    emit readyToResume();
                    break;
                case 2:
                case 3:
                    emit playerReady();
                    break;
                default:
                    emit readyToResume();
                    break;
                }
                pt->stop();
                pt->deleteLater();
            }
        });
    }
}


void Player::playMeta(PlaylistPtr playlist, const MetaPtr pmeta)
{
    Q_D(Player);
    MetaPtr meta = pmeta;
    if (meta == nullptr) {
        if (playlist == nullptr || playlist->isEmpty())
            return;
        meta = playlist->first();
    }
    d->mutex.lock();
    MetaPtr curMeta = meta;
    if (curMeta == nullptr)
        curMeta = d->curPlaylist->first();
    qDebug() << "playMeta"
             << curMeta->title
             << DMusic::lengthString(curMeta->offset) << "/"
             << DMusic::lengthString(curMeta->length);

    if (curMeta.isNull()) {
        d->mutex.unlock();
        return;
    }

    if (playlist->id() != PlayMusicListID)
        d->activePlaylist = playlist;

    d->activeMeta = curMeta;

    d->ischangeMusic = true;
    QFileInfo fileInfo(curMeta->localPath);
    if (QFile::exists(curMeta->localPath)) {
        if (d->qvplayer->state() != Vlc::Stopped && d->qvplayer->state() != Vlc::Idle) {
            d->qvplayer->stop();
        }
        d->isamr = false;
        QString curPath = Global::cacheDir();
        QString toPath = QString("%1/images/%2.mp3").arg(curPath).arg(curMeta->hash);
        if (!QFile::exists(toPath)) {
            //apeToMp3(curMeta->localPath, curMeta->hash);
            Q_EMIT Player::instance()->addApeTask(meta->localPath, meta->hash);
            if (!QFile::exists(toPath)) {
                toPath = curMeta->localPath;
            }
        }
        d->qplayer->setMedia(QMediaContent(QUrl::fromLocalFile(toPath)));
        d->qplayer->setPosition(curMeta->offset);
        d->qplayer->setVolume(100);
        d->qplayer->play();
    } else {
        if (d->qvplayer->state() != Vlc::Stopped && d->qvplayer->state() != Vlc::Idle) {
            d->qvplayer->stop();
        }
        d->isamr = false;
        d->qplayer->setMedia(QMediaContent(QUrl::fromLocalFile(meta->localPath)));
        d->qplayer->setPosition(curMeta->offset);
        d->qplayer->setVolume(100);
        d->qplayer->play();
    }

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

    //vlc & qplayer 声音同步
    QTimer::singleShot(200, this, [ = ]() {
        setVolume(d->volume);
        d->ischangeMusic = false;
    });

    if (d->firstPlayOnLoad == true) {
        d->firstPlayOnLoad = false;
        QTimer::singleShot(150, this, [ = ]() {
            if (d->isamr) {
                d->qvplayer->play();
            } else {
                d->qplayer->play();
            }
        });
    }

    d->fadeOutAnimation->stop();

    if (d->fadeInOut && d->fadeInAnimation->state() != QPropertyAnimation::Running) {
        d->fadeInAnimation = new QPropertyAnimation(this, "fadeInOutFactor");
        d->fadeInAnimation->setEasingCurve(QEasingCurve::InCubic);
        d->fadeInAnimation->setStartValue(0.10000);
        d->fadeInAnimation->setEndValue(1.0000);
        d->fadeInAnimation->setDuration(sFadeInOutAnimationDuration);
        d->fadeInAnimation->start();
    }
    d->mutex.unlock();
}

void Player::resume(PlaylistPtr playlist, const MetaPtr pmeta)
{
    Q_D(Player);
    MetaPtr meta = pmeta;
    if (meta == nullptr) {
        if (playlist == nullptr || playlist->isEmpty())
            return;
        meta = playlist->first();
    }

    if (d->fadeOutAnimation) {
        setFadeInOutFactor(1.0);
        d->fadeOutAnimation->stop();
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
        QString curPath = Global::cacheDir();
        QString toPath = QString("%1/images/%2.mp3").arg(curPath).arg(meta->hash);
        if (QFileInfo(toPath).exists() && d->m_position != -1) { //fisrt start
            stop();

            if (playlist->id() != PlayMusicListID)
                d->activePlaylist = playlist;
            d->activeMeta = meta;

            d->ischangeMusic = true;
            playMeta(playlist, meta);

            if (d->qvplayer->state() != Vlc::Stopped && d->qvplayer->state() != Vlc::Idle) {
                d->qvplayer->stop();
            }
            d->isamr = false;
            QString curPath = Global::cacheDir();
            QString toPath = QString("%1/images/%2.mp3").arg(curPath).arg(meta->hash);
            if (!QFile::exists(toPath)) {
                //apeToMp3(curMeta->localPath, curMeta->hash);
                Q_EMIT Player::instance()->addApeTask(meta->localPath, meta->hash);
                if (!QFile::exists(toPath)) {
                    toPath = meta->localPath;
                }
            }
            d->qplayer->setMedia(QMediaContent(QUrl::fromLocalFile(toPath)));
            d->qplayer->setPosition(d->m_position);
            d->qplayer->setVolume(100);
            d->qplayer->play();

            if (!d->activePlaylist.isNull())
                d->activePlaylist->play(meta);
            d->curPlaylist->play(meta);

            DRecentData data;
            data.appName = "Music";
            data.appExec = "deepin-music";
            DRecentManager::addItem(meta->localPath, data);

            //vlc & qplayer 声音同步
            QTimer::singleShot(200, this, [ = ]() {
                setVolume(d->volume);
                d->ischangeMusic = false;
            });

            if (d->firstPlayOnLoad == true) {
                d->firstPlayOnLoad = false;
                QTimer::singleShot(150, this, [ = ]() {
                    if (d->isamr) {
                        d->qvplayer->play();
                    } else {
                        d->qplayer->play();
                    }
                });
            }

            return;
        }

        if (d->isamr) {
            d->qvplayer->play();
        } else {
            d->qplayer->play();
        }
    });

    if (d->fadeInOut && d->fadeInAnimation->state() != QPropertyAnimation::Running) {
        d->fadeInAnimation->setEasingCurve(QEasingCurve::InCubic);
        d->fadeInAnimation->setStartValue(0.1000);
        d->fadeInAnimation->setEndValue(1.0000);
        d->fadeInAnimation->setDuration(sFadeInOutAnimationDuration);
//        connect(d->fadeInAnimation, &QPropertyAnimation::finished,
//        this, [ = ]() {
//            d->fadeInAnimation = nullptr;
//        });
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
    Q_UNUSED(meta)
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
//    d->ioPlayer->suspend();

    d->fadeInAnimation->stop();

    if (d->fadeInOut && d->fadeOutAnimation->state() != QPropertyAnimation::Running) {
        d->fadeOutAnimation->setEasingCurve(QEasingCurve::OutCubic);
        d->fadeOutAnimation->setStartValue(1.0000);
        d->fadeOutAnimation->setEndValue(0.1000);
        d->fadeOutAnimation->setDuration(sFadeInOutAnimationDuration);
        connect(d->fadeOutAnimation, &QPropertyAnimation::finished,
        this, [ = ]() {
            if (d->isamr) {
                d->qvplayer->pause();
            } else {
                d->qplayer->pause();
            }
            setFadeInOutFactor(1.0);
        });
        d->fadeOutAnimation->start();
    } else {
        if (d->isamr) {
            d->qvplayer->pause();
        } else {
            d->qplayer->pause();
        }
        setFadeInOutFactor(1.0);
    }

    if (d->isamr) {
        d->m_position = static_cast<qlonglong>(d->qvplayer->position());
    } else {
        d->m_position = d->qplayer->position();
    }
}

void Player::pauseNow()
{
    Q_D(Player);
    if (d->isamr) {
        d->qvplayer->pause();
        d->m_position = static_cast<qlonglong>(d->qvplayer->position());
    } else {
        d->qplayer->pause();
        d->m_position = d->qplayer->position();
    }
}

void Player::stop()
{
    Q_D(Player);

    if (d->isamr) {
        d->qvplayer->pause();
        d->activeMeta.clear(); //清除当前播放音乐；
        d->qvplayer->stop();
    } else {
        d->qplayer->pause();
        d->qplayer->setMedia(QMediaContent());
        d->activeMeta.clear(); //清除当前播放音乐；
        d->qplayer->stop();
    }

}

Player::PlaybackStatus Player::status()
{
    Q_D(const Player);
    if (d->isamr) {
        Vlc::State  status = d->qvplayer->state();

        if (status == Vlc::Playing) {
            return PlaybackStatus::Playing;
        } else if (status == Vlc::Paused) {
            return PlaybackStatus::Paused;
        } else if (status == Vlc::Stopped) {
            return PlaybackStatus::Stopped;
        } else {
            return PlaybackStatus::InvalidPlaybackStatus;
        }
    } else {
        return static_cast<PlaybackStatus>(d->qplayer->state());
    }

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
    if (d->isamr) {
        return  d->qvplayer->length();
    } else {
        if (d->qplayer->duration() == d->activeMeta->length) {
            return d->qplayer->duration();
        } else {
            return  d->activeMeta->length;
        }
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

    if (d->isamr) {
        if (d->qvplayer->length() == d->activeMeta->length) {
            return d->qvplayer->setTime(position);
        } else {
            d->qvplayer->setTime(position + d->activeMeta->offset);
        }
    } else {
        if (d->qplayer->duration() == d->activeMeta->length) {
            return d->qplayer->setPosition(position);
        } else {
            d->qplayer->setPosition(position + d->activeMeta->offset);
        }
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

void Player::updateVolume(int volume)
{
    Q_D(Player);
    if (volume > 100) {
        volume = 100;
    }
    if (volume < 0) {
        volume = 0;
    }
    if (!d->ischangeMusic) {
        d->volume = volume;
    }
}

void Player::setMuted(bool mute)
{
//    Q_D(Player);
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

        Q_EMIT mediaError(d->activePlaylist, d->activeMeta, Player::ResourceError);

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

bool Player::isReady()
{
    Q_D(Player);
    return d->canPlay;
}

void Player::setDoubleClickStartType(int start)
{
    Q_D(Player);
    d->startSameMusic = start;
}
