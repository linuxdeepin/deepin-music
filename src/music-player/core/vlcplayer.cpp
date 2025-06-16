// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vlcplayer.h"
#include "qtplayer.h"

#include "util/dbusutils.h"
#include "util/global.h"

#include <vlc/vlc.h>
#include "vlc/Error.h"
#include "vlc/Common.h"
#include "vlc/Enums.h"
#include "vlc/Instance.h"
#include "vlc/Media.h"
#include "vlc/MediaPlayer.h"
#include "vlc/Equalizer.h"
#include "vlc/sdlplayer.h"
#include "core/musicsettings.h"

#include <QDBusObjectPath>
#include <QDBusInterface>
#include <QProcess>
#include <QApplication>
#include <QFile>
#include <metadetector.h>

VlcPlayer::VlcPlayer(QObject *parent)
    : PlayerBase(parent)
{

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
#ifdef __sw_64__
        m_qvplayer = new VlcMediaPlayer(m_qvinstance);
#else
        m_qvplayer = new SdlPlayer(m_qvinstance);
#endif
        m_qvplayer->equalizer()->setPreamplification(12);
        m_qvmedia = new VlcMedia();

        connect(m_qvplayer, &VlcMediaPlayer::timeChanged, this, &VlcPlayer::timeChanged);
        connect(m_qvplayer, &VlcMediaPlayer::positionChanged, this, &VlcPlayer::positionChanged);
        connect(m_qvmedia, &VlcMedia::stateChanged,
        this, [ = ](Vlc::State state) {
            switch (state) {
            case Vlc::Playing: {
                emit stateChanged(PlayerBase::PlayState::Playing);
                break;
            }
            case Vlc::Paused: {
                emit stateChanged(PlayerBase::PlayState::Paused);
                break;
            }
            case Vlc::Stopped: {
                emit stateChanged(PlayerBase::PlayState::Stopped);
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
    m_qtPlayer->release();
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
        init();
        //初始化mediaplayer
        m_pCdaThread->setMediaPlayerPointer(m_qvplayer->core());
        //查询cd信息
        m_pCdaThread->doQuery();
    });
}

void VlcPlayer::play()
{
    if(m_bApe) {
        return m_qtPlayer->play();
    }
    init();
    m_qvplayer->play();
}

void VlcPlayer::pause()
{
    if(m_bApe) {
        return m_qtPlayer->pause();
    }
    if (m_qvplayer) {
#ifndef __sw_64__
        static_cast<SdlPlayer*>(m_qvplayer)->setCachingThreadPause(true);
#endif
        m_qvplayer->pause();
    }
}

void VlcPlayer::pauseNew()
{
    if(m_bApe) {
        return m_qtPlayer->pauseNew();
    }
    if (m_qvplayer) {
        m_qvplayer->pauseNew();
    }
}

void VlcPlayer::resume()
{
    if(m_bApe) {
        return m_qtPlayer->resume();
    }
    if (m_qvplayer) {
        m_qvplayer->resume();
#ifndef __sw_64__
        static_cast<SdlPlayer*>(m_qvplayer)->setCachingThreadPause(false);
#endif
    }
}
PlayerBase::PlayState VlcPlayer::state()
{
    if(m_bApe) {
        return m_qtPlayer->state();
    }
    init();
    Vlc::State  state = m_qvplayer->state();
    switch (state) {
    case Vlc::Playing: {
        return PlayerBase::Playing;
    }
    case Vlc::Paused: {
        return PlayerBase::Paused;
    }
    case Vlc::Stopped: {
        return PlayerBase::Stopped;
    }
    default:
        return static_cast<PlayerBase::PlayState>(state);
    }
}

void VlcPlayer::stop()
{
    if(m_bApe) {
        return m_qtPlayer->stop();
    }
    if (m_qvplayer) {
        m_qvplayer->stop();
    }
}

int VlcPlayer::length()
{
    if(m_bApe) {
        return m_qtPlayer->length();
    }
    init();
    qDebug() << "VlcPlayer: m_qvplayer->length()" << m_qvplayer->length();
    return  m_qvplayer->length();
}

void VlcPlayer::setTime(qint64 time)
{
    if(m_bApe) {
        return m_qtPlayer->setTime(time);
    }
    init();
    m_qvplayer->setTime(time);
}

qint64 VlcPlayer::time()
{
    if(m_bApe) {
        return m_qtPlayer->time();
    }
    init();
    return m_qvplayer->time();
}

void VlcPlayer::setMediaMeta(MediaMeta meta)
{
    init();
    m_activeMeta = meta;
    bool newIsApe = (MetaDetector::getInstance()->getAudioType(meta).toLower() == "ape");
    bool engineChanged = (m_bApe != newIsApe);

    m_bApe = newIsApe;
    if(m_bApe) {
        connect(m_qtPlayer, &PlayerBase::timeChanged, this, &PlayerBase::timeChanged);
        connect(m_qtPlayer, &PlayerBase::positionChanged, this, &PlayerBase::positionChanged);
        connect(m_qtPlayer, &PlayerBase::stateChanged, this, &PlayerBase::stateChanged);
        connect(m_qtPlayer, &PlayerBase::end, this, &PlayerBase::end);
        connect(m_qtPlayer, &PlayerBase::sigSendCdaStatus, this, &PlayerBase::sigSendCdaStatus);
        m_qtPlayer->setMediaMeta(meta);
    } else {
        m_qtPlayer->stop();
        disconnect(m_qtPlayer, &PlayerBase::timeChanged, this, &PlayerBase::timeChanged);
        disconnect(m_qtPlayer, &PlayerBase::positionChanged, this, &PlayerBase::positionChanged);
        disconnect(m_qtPlayer, &PlayerBase::stateChanged, this, &PlayerBase::stateChanged);
        disconnect(m_qtPlayer, &PlayerBase::end, this, &PlayerBase::end);
        disconnect(m_qtPlayer, &PlayerBase::sigSendCdaStatus, this, &PlayerBase::sigSendCdaStatus);
        m_qvmedia->initMedia(meta.localPath, meta.mmType == MIMETYPE_CDA ? false : true, m_qvinstance, meta.track);
    }
    m_qvplayer->open(m_qvmedia);

    if (engineChanged) {
        if (m_bApe) {
            m_qtPlayer->setMute(m_qvplayer->getMute());
            // m_qtPlayer->setVolume(m_qvplayer->getVolume()); //Qt自带播放器播放APE格式视频时调节音量可能会导致音量异常
        } else {
            m_qvplayer->setMute(m_qtPlayer->getMute());
            m_qtPlayer->setMute(false);
            auto qtPlayer = dynamic_cast<QtPlayer *>(m_qtPlayer);
            if (qtPlayer)
                m_qvplayer->setVolume(qtPlayer->getVolume());
        }
    }
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

void VlcPlayer::setVolume(int volume)
{
    if(m_bApe) {
        return m_qtPlayer->setVolume(volume);
    }
    init();
    m_qvplayer->setVolume(volume);
}

void VlcPlayer::setMute(bool value)
{
    if(m_bApe) {
        return m_qtPlayer->setMute(value);
    }
    init();
    m_qvplayer->setMute(value);
}

void VlcPlayer::initCddaTrack()
{
    if(m_bApe) {
        return m_qtPlayer->initCddaTrack();
    }
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

void VlcPlayer::setQtPlayer(PlayerBase *qtPlayer)
{
    m_qtPlayer = qtPlayer;
}

bool VlcPlayer::getMute()
{
    if(m_bApe) {
        return m_qtPlayer->getMute();
    }
    return m_qvplayer->getMute();
}



