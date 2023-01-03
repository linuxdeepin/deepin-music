// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vlcplayer.h"

#include <vlc/vlc.h>

#include <QDBusObjectPath>
#include <QDBusInterface>

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
                       << "aiff";
}

VlcPlayer::~VlcPlayer()
{
    releasePlayer();
}

void VlcPlayer::init()
{
    //防止多次创建
    if (m_qvinstance == nullptr) {
        m_qvinstance = new VlcInstance(VlcCommon::args(), nullptr);
        m_qvinstance->version();
        m_qvplayer = new VlcMediaPlayer(m_qvinstance);
        m_qvplayer->equalizer()->setPreamplification(12);
        m_qvmedia = new VlcMedia();

        connect(m_qvplayer, &VlcMediaPlayer::timeChanged, this, &VlcPlayer::timeChanged);
        connect(m_qvplayer, &VlcMediaPlayer::positionChanged, this, &VlcPlayer::positionChanged);
        connect(m_qvmedia, &VlcMedia::stateChanged,
        this, [ = ](Vlc::State state) {
            switch (state) {
            case Vlc::Playing: {
                emit stateChanged(DmGlobal::Playing);
                break;
            }
            case Vlc::Paused: {
                emit stateChanged(DmGlobal::Paused);
                break;
            }
            case Vlc::Stopped: {
                emit stateChanged(DmGlobal::Stopped);
                break;
            }
            default:
                break;
            }
        });

        //当一首歌播放结束时，发送此信息，让player继续播放下一首
        connect(m_qvplayer, &VlcMediaPlayer::end, this, &VlcPlayer::end);
        initCdaThread();
    }
}

void VlcPlayer::releasePlayer()
{
    //释放cd线程
    if (m_pCdaThread) {
        m_pCdaThread->closeThread();
        while (m_pCdaThread->isRunning()) {}
    }

    //删除媒体资源
    if (m_qvmedia) {
        delete m_qvmedia;
        m_qvmedia = nullptr;
    }
    //删除媒体播放器
    if (m_qvplayer) {
        delete m_qvplayer;
        m_qvplayer = nullptr;
    }
    if (m_qvinstance) {
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
        qDebug() << __func__ << "timer timeout.";
        init();
        qDebug() << "init cda thread.";
        //初始化mediaplayer
        m_pCdaThread->setMediaPlayerPointer(m_qvplayer->core());
        qDebug() << "setMediaPlayerPointer.";
        //查询cd信息
        m_pCdaThread->doQuery();
        qDebug() << "start cda thread.";
    });
}

void VlcPlayer::play()
{
    init();
    m_qvplayer->play();
}

void VlcPlayer::pause()
{
    if (m_qvplayer) {
        m_qvplayer->pause();
    }
}

DmGlobal::PlaybackStatus VlcPlayer::state()
{
    if (m_qvplayer) {
        Vlc::State  state = m_qvplayer->state();
        switch (state) {
        case Vlc::Playing: {
            return DmGlobal::Playing;
        }
        case Vlc::Paused: {
            return DmGlobal::Paused;
        }
        case Vlc::Stopped: {
            return DmGlobal::Stopped;
        }
        default:
            return DmGlobal::Stopped;
        }
    } else {
        return DmGlobal::Stopped;
    }

}

void VlcPlayer::stop()
{
    if (m_qvplayer) {
        m_qvplayer->stop();
    }
}

int VlcPlayer::length()
{
    init();
    qDebug() << "VlcPlayer: m_qvplayer->length()" << m_qvplayer->length();
    return  m_qvplayer->length();
}

void VlcPlayer::setTime(qint64 time)
{
    init();
    m_qvplayer->setTime(time);
}

qint64 VlcPlayer::time()
{
    init();
    return m_qvplayer->time();
}

void VlcPlayer::setMediaMeta(MediaMeta meta)
{
    init();
    m_activeMeta = meta;
    m_qvmedia->initMedia(meta.localPath, meta.mmType == DmGlobal::MimeTypeCDA ? false : true, m_qvinstance, meta.track);
    m_qvplayer->open(m_qvmedia);
    emit metaChanged();
}

void VlcPlayer::setFadeInOutFactor(double fadeInOutFactor)
{
    init();
    m_qvplayer->equalizer()->blockSignals(true);
    setPreamplification(static_cast<float>(12 * fadeInOutFactor));
    m_qvplayer->equalizer()->blockSignals(false);
}

void VlcPlayer::setEqualizerEnabled(bool enabled)
{
    init();
    m_qvplayer->equalizer()->setEnabled(enabled);
}

void VlcPlayer::loadFromPreset(uint index)
{
    init();
    m_qvplayer->equalizer()->loadFromPreset(index);
}

void VlcPlayer::setPreamplification(float value)
{
    init();
    m_qvplayer->equalizer()->setPreamplification(value);
}

void VlcPlayer::setAmplificationForBandAt(float amp, uint bandIndex)
{
    init();
    m_qvplayer->equalizer()->setAmplificationForBandAt(amp, bandIndex);
}

float VlcPlayer::amplificationForBandAt(uint bandIndex)
{
    init();
    return m_qvplayer->equalizer()->amplificationForBandAt(bandIndex);
}

float VlcPlayer::preamplification()
{
    init();
    return m_qvplayer->equalizer()->preamplification();
}

void VlcPlayer::setVolume(int volume)
{
    init();
    m_volume = volume;
    m_qvplayer->setVolume(volume);
}

int VlcPlayer::getVolume()
{
    init();
    return m_volume;
}

void VlcPlayer::setMute(bool value)
{
    init();
    m_qvplayer->setMute(value);
}

void VlcPlayer::initCddaTrack()
{
    init();
    m_qvplayer->initCddaTrack();
}

void VlcPlayer::setEqualizer(bool enabled, int curIndex, QList<int> indexbaud)
{
    init();
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

QList<MediaMeta> VlcPlayer::getCdaMetaInfo()
{
    if (m_pCdaThread != nullptr)
        return m_pCdaThread->getCdaMetaInfo();
    return QList<MediaMeta>();
}

bool VlcPlayer::getMute()
{
    return m_qvplayer->getMute();
}



