// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vlcplayer.h"
#include "util/log.h"

#include <vlc/vlc.h>

#include <QDBusObjectPath>
#include <QDBusInterface>
#include <malloc.h>

#include "vlc/Error.h"
#include "vlc/Common.h"
#include "vlc/Enums.h"
#include "vlc/Instance.h"
#include "vlc/Media.h"
#include "vlc/MediaPlayer.h"
#include "vlc/Equalizer.h"

VlcPlayer::VlcPlayer(QObject *parent)
    : PlayerBase(parent)
{
    qCDebug(dmMusic) << "VlcPlayer constructor";
    m_supportedSuffix  << "aac"
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
                       << "aiff"
                       << "opus";
}

VlcPlayer::~VlcPlayer()
{
    releasePlayer();
}

void VlcPlayer::init()
{
    //防止多次创建
    if (m_qvinstance == nullptr) {
        qCDebug(dmMusic) << "Creating VLC instance and initializing components";
        m_qvinstance = new VlcInstance(VlcCommon::args(), nullptr);
        if (!m_qvinstance->core()) {
            qCCritical(dmMusic) << "VLC instance core is null, cannot create media player";
            delete m_qvinstance;
            m_qvinstance = nullptr;
            return;
        }
        m_qvinstance->version();
        m_qvplayer = new SdlPlayer(m_qvinstance);
        m_qvplayer->equalizer()->setPreamplification(12);
        m_qvmedia = new VlcMedia();

        connect(m_qvplayer, &VlcMediaPlayer::timeChanged, this, &VlcPlayer::timeChanged);
        connect(m_qvplayer, &VlcMediaPlayer::positionChanged, this, &VlcPlayer::positionChanged);
        connect(m_qvmedia, &VlcMedia::stateChanged,
        this, [ = ](Vlc::State state) {
            switch (state) {
            case Vlc::Playing: {
                qCInfo(dmMusic) << "VLC state changed to Playing";
                emit stateChanged(DmGlobal::Playing);
                break;
            }
            case Vlc::Paused: {
                qCInfo(dmMusic) << "VLC state changed to Paused";
                emit stateChanged(DmGlobal::Paused);
                break;
            }
            case Vlc::Stopped: {
                qCInfo(dmMusic) << "VLC state changed to Stopped";
                emit stateChanged(DmGlobal::Stopped);
                break;
            }
            default:
                break;
            }
        });

        connect(m_qvplayer, &VlcMediaPlayer::end, this, [ = ]() {
            qCDebug(dmMusic) << "Current track playback ended";
            emit end();
        });
        connect(m_qvplayer, &SdlPlayer::audioDataReady, this, &PlayerBase::audioDataReady);
        initCdaThread();
    }
}

void VlcPlayer::releasePlayer()
{
    qCDebug(dmMusic) << "Releasing VLC player resources";
    //释放cd线程
    if (m_pCdaThread) {
        qCDebug(dmMusic) << "Closing CDA thread";
        m_pCdaThread->closeThread();
        while (m_pCdaThread->isRunning()) {}
    }

    //删除媒体资源
    if (m_qvmedia) {
        qCDebug(dmMusic) << "Releasing VLC media";
        delete m_qvmedia;
        m_qvmedia = nullptr;
    }
    //删除媒体播放器
    if (m_qvplayer) {
        qCDebug(dmMusic) << "Releasing VLC media player";
        delete m_qvplayer;
        m_qvplayer = nullptr;
    }
    if (m_qvinstance) {
        qCDebug(dmMusic) << "Releasing VLC instance";
        delete m_qvinstance;
        m_qvinstance = nullptr;
    }
}

void VlcPlayer::release()
{
    releasePlayer();
}

void VlcPlayer::initCdaThread()
{
    if (nullptr == m_pCdaThread) {
        qCDebug(dmMusic) << "Initializing CDA thread";
        m_pCdaThread = new CdaThread(this);
        connect(m_pCdaThread, &CdaThread::sigSendCdaStatus, this,
                &PlayerBase::sigSendCdaStatus, Qt::QueuedConnection);
    }
    startCdaThread();
}

void VlcPlayer::startCdaThread()
{
    //为了不影响主线程加载，1s后再加载CD
    QTimer::singleShot(1000, this, [ = ]() {
        qCDebug(dmMusic) << __func__ << "timer timeout.";
        init();
        if (!m_qvplayer) return;
        qCDebug(dmMusic) << "init cda thread.";
        //初始化mediaplayer
        m_pCdaThread->setMediaPlayerPointer(m_qvplayer->core());
        qCDebug(dmMusic) << "setMediaPlayerPointer.";
        //查询cd信息
        m_pCdaThread->doQuery();
        qCDebug(dmMusic) << "start cda thread.";
    });
}

void VlcPlayer::play()
{
    qCDebug(dmMusic) << "Starting playback";
    init();
    if (m_qvplayer) {
        m_qvplayer->play();
    }
}

void VlcPlayer::pause()
{
    if (m_qvplayer) {
        qCDebug(dmMusic) << "Pausing playback";
        m_qvplayer->pause();
    }
}

DmGlobal::PlaybackStatus VlcPlayer::state()
{
    if (m_qvplayer) {
        Vlc::State  state = m_qvplayer->state();
        switch (state) {
        case Vlc::Playing: {
            qCDebug(dmMusic) << "Current state: Playing";
            return DmGlobal::Playing;
        }
        case Vlc::Paused: {
            qCDebug(dmMusic) << "Current state: Paused";
            return DmGlobal::Paused;
        }
        case Vlc::Stopped: {
            qCDebug(dmMusic) << "Current state: Stopped";
            return DmGlobal::Stopped;
        }
        default:
            qCDebug(dmMusic) << "Current state: Unknown, returning Stopped";
            return DmGlobal::Stopped;
        }
    } else {
        qCDebug(dmMusic) << "No player instance, state: Stopped";
        return DmGlobal::Stopped;
    }
}

void VlcPlayer::stop()
{
    if (m_qvplayer) {
        qCDebug(dmMusic) << "Stopping playback";
        m_qvplayer->stop();
    }
}

int VlcPlayer::length()
{
    init();
    if (!m_qvplayer) return 0;
    int len = m_qvplayer->length();
    qCDebug(dmMusic) << "Track length:" << len << "ms";
    return len;
}

void VlcPlayer::setTime(qint64 time)
{
    init();
    if (!m_qvplayer) return;
    qCDebug(dmMusic) << "Setting playback position to:" << time << "ms";
    m_qvplayer->setTime(time);
}

qint64 VlcPlayer::time()
{
    init();
    if (!m_qvplayer) return 0;
    qint64 pos = m_qvplayer->time();
    qCDebug(dmMusic) << "Current playback position:" << pos << "ms";
    return pos;
}

void VlcPlayer::setMediaMeta(MediaMeta meta)
{
    qCInfo(dmMusic) << "Setting media meta - Title:" << meta.title << "Path:" << meta.localPath;
    init();
    if (!m_qvplayer) return;
    m_activeMeta = meta;
    m_qvmedia->initMedia(meta.localPath, meta.mmType == DmGlobal::MimeTypeCDA ? false : true, m_qvinstance, meta.track);
    m_qvplayer->open(m_qvmedia);
    m_qvplayer->setCurMeta(meta);
    emit metaChanged();
#ifdef Q_OS_LINUX
    malloc_trim(0);
#endif
}

void VlcPlayer::setFadeInOutFactor(double fadeInOutFactor)
{
    init();
    if (!m_qvplayer) return;
    m_qvplayer->equalizer()->blockSignals(true);
    setPreamplification(static_cast<float>(12 * fadeInOutFactor));
    m_qvplayer->equalizer()->blockSignals(false);
}

void VlcPlayer::setEqualizerEnabled(bool enabled)
{
    init();
    if (!m_qvplayer) return;
    qCInfo(dmMusic) << "Setting equalizer enabled:" << enabled;
    m_qvplayer->equalizer()->setEnabled(enabled);
}

void VlcPlayer::loadFromPreset(uint index)
{
    init();
    if (!m_qvplayer) return;
    qCDebug(dmMusic) << "Loading equalizer preset:" << index;
    m_qvplayer->equalizer()->loadFromPreset(index);
}

void VlcPlayer::setPreamplification(float value)
{
    init();
    if (!m_qvplayer) return;
    m_qvplayer->equalizer()->setPreamplification(value);
}

void VlcPlayer::setAmplificationForBandAt(float amp, uint bandIndex)
{
    init();
    if (!m_qvplayer) return;
    m_qvplayer->equalizer()->setAmplificationForBandAt(amp, bandIndex);
}

float VlcPlayer::amplificationForBandAt(uint bandIndex)
{
    init();
    if (!m_qvplayer) return 0.0f;
    return m_qvplayer->equalizer()->amplificationForBandAt(bandIndex);
}

float VlcPlayer::preamplification()
{
    init();
    if (!m_qvplayer) return 0.0f;
    return m_qvplayer->equalizer()->preamplification();
}

void VlcPlayer::setVolume(int volume)
{
    init();
    qCDebug(dmMusic) << "Setting volume to:" << volume;
    m_volume = volume;
    if (m_qvplayer) {
        m_qvplayer->setVolume(volume);
    }
}

int VlcPlayer::getVolume()
{
    init();
    return m_volume;
}

void VlcPlayer::setMute(bool value)
{
    init();
    qCDebug(dmMusic) << "Setting mute state to:" << value;
    if (m_qvplayer) {
        m_qvplayer->setMute(value);
    }
}

void VlcPlayer::initCddaTrack()
{
    init();
    if (!m_qvplayer) return;
    qCDebug(dmMusic) << "Initializing CDDA track";
    m_qvplayer->initCddaTrack();
}

void VlcPlayer::setEqualizer(bool enabled, int curIndex, QList<int> indexbaud)
{
    init();
    if (!m_qvplayer) return;
    qCInfo(dmMusic) << "Configuring equalizer - Enabled:" << enabled << "Index:" << curIndex;
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
                qCWarning(dmMusic) << "Empty equalizer band values provided";
                return;
            } else {
                qCDebug(dmMusic) << "Setting custom equalizer values";
                m_qvplayer->equalizer()->setPreamplification(indexbaud.at(0));
                for (int i = 1; i < 11; i++) {
                    m_qvplayer->equalizer()->setAmplificationForBandAt(indexbaud.at(i), uint(i - 1));
                }
            }
        }
    }
}

QList<MediaMeta> VlcPlayer::getCdaMetaInfo()
{
    if (m_pCdaThread != nullptr) {
        qCDebug(dmMusic) << "Retrieving CDA meta information";
        return m_pCdaThread->getCdaMetaInfo();
    }
    qCWarning(dmMusic) << "CDA thread not initialized, cannot get meta info";
    return QList<MediaMeta>();
}

bool VlcPlayer::getMute()
{
    if (!m_qvplayer) {
        qCWarning(dmMusic) << "VLC player not initialized, returning default mute state";
        return false;
    }
    bool muted = m_qvplayer->getMute();
    qCDebug(dmMusic) << "Current mute state:" << muted;
    return muted;
}



