// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sdlplayer.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
#include <libavutil/common.h>
#include <SDL2/SDL_log.h>
#include <SDL2/SDL_stdinc.h>
#ifdef __cplusplus
}
#endif // __cplusplus

#include <vlc/vlc.h>

#include <QDebug>
#include <QMutex>
#include <QTimer>
#include <QDBusObjectPath>
#include <QDBusInterface>

#include "global.h"
#include "vlcdynamicinstance.h"
#include "checkdatazerothread.h"
#include "Media.h"
#include "util/utils.h"
#include "util/log.h"

#define PLAYBACK_STATUS_INIT        0
#define PLAYBACK_STATUS_CHANGING    1
#define PLAYBACK_STATUS_RESTORE     2

#ifndef AUDIO_MIN_BUFFER_SIZE
#define AUDIO_MIN_BUFFER_SIZE 512
#endif
#ifndef AUDIO_MAX_CALLBACKS_PER_SEC
#define AUDIO_MAX_CALLBACKS_PER_SEC 30
#endif

#define SDL_AUDIO_ERR_MSG "Error writing to datastream"
int g_playbackStatus = 0;
static QMutex vlc_mutex;

typedef SDL_AudioStatus(*SDL_GetAudioStatus_function)();
typedef int (*SDL_Init_function)(Uint32 flags);
typedef void (*SDL_PauseAudio_function)(int pause_on);
typedef void (*SDL_Delay_function)(Uint32 ms);
typedef int (*SDL_OpenAudio_function)(SDL_AudioSpec *desired,
                                      SDL_AudioSpec *obtained);
typedef void *(*SDL_memset_function)(SDL_OUT_BYTECAP(len) void *dst, int c, size_t len);
typedef void (*SDL_MixAudio_function)(Uint8 *dst, const Uint8 *src,
                                      Uint32 len, int volume);
typedef Uint32(*SDL_GetQueuedAudioSize_function)(SDL_AudioDeviceID dev);
typedef void (*SDL_ClearQueuedAudio_function)(SDL_AudioDeviceID dev);
typedef void (*SDL_LockAudio_function)(void);
typedef void (*SDL_UnlockAudio_function)(void);
typedef void (*SDL_CloseAudio_function)(void);
typedef void (*SDL_LogSetPriority_function)(int category,
                                            SDL_LogPriority priority);
typedef void (*SDL_LogSetOutputFunction_function)(SDL_LogOutputFunction callback, void *userdata);
typedef void (*SDL_Quit_function)(void);

typedef int (*vlc_audio_set_volume_function)(libvlc_media_player_t *, int);
typedef int (*vlc_audio_get_volume_function)(libvlc_media_player_t *);
typedef void (*vlc_audio_set_callbacks_function)(libvlc_media_player_t *mp,
                                                 libvlc_audio_play_cb play,
                                                 libvlc_audio_pause_cb pause,
                                                 libvlc_audio_resume_cb resume,
                                                 libvlc_audio_flush_cb flush,
                                                 libvlc_audio_drain_cb drain,
                                                 void *opaque);
typedef void (*vlc_audio_set_format_callbacks_function)(libvlc_media_player_t *mp,
                                                        libvlc_audio_setup_cb setup,
                                                        libvlc_audio_cleanup_cb cleanup);

typedef av_const int (*av_log2_function)(unsigned v);

/** SDL已提交patch打印写入错误信息，通过SDL_LogOutputFunction_Err_Write回调函数可接收到错误信息
    音乐可关闭SDL设备，重新播放歌曲，避免写入错误时，继续写入数据导致的崩溃
**/
void SDL_LogOutputFunction_Err_Write(void *userdata, int category, SDL_LogPriority priority, const char *message)
{
    qCDebug(dmMusic) << "SDL log message - Category:" << category << "Priority:" << priority << "Message:" << message;
    SDL_GetAudioStatus_function GetAudioStatus = (SDL_GetAudioStatus_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSdlSymbol("SDL_GetAudioStatus");
    QString strmsg = message;
    if (strmsg == SDL_AUDIO_ERR_MSG && category == SDL_LOG_CATEGORY_AUDIO && priority == SDL_LOG_PRIORITY_ERROR) {
        if (GetAudioStatus() == SDL_AudioStatus::SDL_AUDIO_PLAYING) {
            /** checkDataZeroThread线程监控g_playbackStatus的值，如果为PLAYBACK_STATUS_RESTORE
                会自动切换下一首歌曲。该线程最初用于监控歌曲是否播放完成，也适用于目前的场景
            **/
            //线程监控g_playbackStatus的值由于有时间间隔无法及时执行下一首歌曲的操作,还是会因为sdl向pulseaudio写入数据错误而导致音乐崩溃，
            //这里直接调用VlcMediaPlayer中checkDataZero来播放下一首歌曲
            //g_playbackStatus = PLAYBACK_STATUS_RESTORE;
            qCWarning(dmMusic) << "SDL audio error detected during playback, switching to next song";
            SdlPlayer *sdlPlayer = static_cast<SdlPlayer *>(userdata);
            sdlPlayer->checkDataZero();
        }
    }
}

SdlPlayer::SdlPlayer(VlcInstance *instance)
    : VlcMediaPlayer(instance), m_loadSdlLibrary(false)
{
    qCDebug(dmMusic) << "Initializing SDL player";
    m_loadSdlLibrary = VlcDynamicInstance::VlcFunctionInstance()->loadSdlLibrary();
    if (m_loadSdlLibrary) {
        qCDebug(dmMusic) << "SDL library loaded successfully, initializing SDL audio";
        SDL_Init_function Init = (SDL_Init_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSdlSymbol("SDL_GetAudioStatus");
        vlc_audio_set_callbacks_function vlc_audio_set_callbacks = (vlc_audio_set_callbacks_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("libvlc_audio_set_callbacks");
        vlc_audio_set_format_callbacks_function vlc_audio_set_format_callbacks = (vlc_audio_set_format_callbacks_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("libvlc_audio_set_format_callbacks");
        SDL_LogSetPriority_function LogSetPriority = (SDL_LogSetPriority_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSdlSymbol("SDL_LogSetPriority");
        SDL_LogSetOutputFunction_function LogSetOutputFunction = (SDL_LogSetOutputFunction_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSdlSymbol("SDL_LogSetOutputFunction");
        Init(SDL_INIT_AUDIO);
        vlc_audio_set_callbacks(_vlcMediaPlayer, libvlc_audio_play_cb, libvlc_audio_pause_cb, libvlc_audio_resume_cb, libvlc_audio_flush_cb, nullptr, this);
        vlc_audio_set_format_callbacks(_vlcMediaPlayer, libvlc_audio_setup_cb, nullptr);

        //设置日志回调等级
        LogSetPriority(SDL_LOG_CATEGORY_AUDIO, SDL_LOG_PRIORITY_ERROR);
        //注册SDL日志回调，捕获SDL输出的错误日志
        LogSetOutputFunction(SDL_LogOutputFunction_Err_Write, this);

        g_playbackStatus = PLAYBACK_STATUS_INIT;
        m_pCheckDataThread = new CheckDataZeroThread(this, this);
        connect(m_pCheckDataThread, &CheckDataZeroThread::sigPlayNextSong, this, &SdlPlayer::checkDataZero, Qt::QueuedConnection);
        connect(m_pCheckDataThread, &CheckDataZeroThread::sigExtraTime, this, &VlcMediaPlayer::timeChanged, Qt::QueuedConnection);
        qCDebug(dmMusic) << "SDL player initialization completed";
    } else {
        qCWarning(dmMusic) << "Failed to load SDL library";
    }
}

SdlPlayer::~SdlPlayer()
{
    qCDebug(dmMusic) << "Cleaning up SDL player";
    if (m_loadSdlLibrary) {
        SDL_Quit_function Quit = (SDL_Quit_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSdlSymbol("SDL_Quit");
        Quit();
        qCDebug(dmMusic) << "SDL quit called";
        m_pCheckDataThread->quitThread();
        while (m_pCheckDataThread->isRunning()) {}
        qCDebug(dmMusic) << "Check data thread stopped";
    }
}

void SdlPlayer::open(VlcMedia *media)
{
    qCDebug(dmMusic) << "Opening media in SDL player";
    //防止没打开文件
    if (media->core() == nullptr) {
        qCWarning(dmMusic) << "Cannot open media: media core is null";
        return;
    }

    if (m_loadSdlLibrary) {
        qCDebug(dmMusic) << "Preparing SDL audio for new media";
        SDL_GetAudioStatus_function GetAudioStatus = (SDL_GetAudioStatus_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSdlSymbol("SDL_GetAudioStatus");
        SDL_PauseAudio_function PauseAudio = (SDL_PauseAudio_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSdlSymbol("SDL_PauseAudio");
        SDL_GetQueuedAudioSize_function GetQueuedAudioSize = (SDL_GetQueuedAudioSize_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSdlSymbol("SDL_GetQueuedAudioSize");
        SDL_ClearQueuedAudio_function ClearQueuedAudio = (SDL_ClearQueuedAudio_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSdlSymbol("SDL_ClearQueuedAudio");
        SDL_LockAudio_function LockAudio = (SDL_LockAudio_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSdlSymbol("SDL_LockAudio");
        SDL_UnlockAudio_function UnlockAudio = (SDL_UnlockAudio_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSdlSymbol("SDL_UnlockAudio");
        SDL_Delay_function Delay = (SDL_Delay_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSdlSymbol("SDL_Delay");
        SDL_CloseAudio_function CloseAudio = (SDL_CloseAudio_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSdlSymbol("SDL_CloseAudio");
        if (GetAudioStatus() != SDL_AUDIO_PLAYING) {
            PauseAudio(1);
            qCDebug(dmMusic) << "Audio paused for initialization";
        }
        cleanMemCache();
        /**
          根据SDL官方标注，必须清空SDL队列播放后才能调用SDL_LockAudio()，
          不然可能引起不必要的错误,参数为1是SDL默认设备，无需更改
        **/
        if (GetQueuedAudioSize(1) > 0) {
            ClearQueuedAudio(1);
            qCDebug(dmMusic) << "Cleared audio queue";
        }
        LockAudio();
        Delay(40);
        UnlockAudio();
        if (qEnvironmentVariable("DDE_CURRENT_COMPOSITOR") != "TreeLand") {
            CloseAudio();
            qCDebug(dmMusic) << "Closed audio device";
        }

        m_sinkInputPath.clear();
    }

    VlcMediaPlayer::open(media);
    g_playbackStatus = PLAYBACK_STATUS_INIT;
    qCDebug(dmMusic) << "Media opened successfully";
}

void SdlPlayer::play()
{
    if (!_vlcMediaPlayer) {
        qCWarning(dmMusic) << "Cannot play: media player is null";
        return;
    }
    qCDebug(dmMusic) << "Starting playback";
    VlcMediaPlayer::play();
    if (m_loadSdlLibrary) {
        if (!m_pCheckDataThread->isRunning()) {
            m_pCheckDataThread->start();
            qCDebug(dmMusic) << "Started check data thread";
        }
    }
}

void SdlPlayer::pause()
{
    if (!_vlcMediaPlayer) {
        qCWarning(dmMusic) << "Cannot pause: media player is null";
        return;
    }
    qCDebug(dmMusic) << "Pausing playback";
    setProgressTag(0); //first start

    if (m_loadSdlLibrary) {
        SDL_GetAudioStatus_function GetAudioStatus = (SDL_GetAudioStatus_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSdlSymbol("SDL_GetAudioStatus");
        SDL_PauseAudio_function PauseAudio = (SDL_PauseAudio_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSdlSymbol("SDL_PauseAudio");
        if (GetAudioStatus() != SDL_AUDIO_PAUSED && GetAudioStatus() != SDL_AUDIO_STOPPED) {
            PauseAudio(1);
            qCDebug(dmMusic) << "Audio paused";
        }
    }

    VlcMediaPlayer::pause();
}

void SdlPlayer::resume()
{
    if (!_vlcMediaPlayer) {
        qCWarning(dmMusic) << "Cannot resume: media player is null";
        return;
    }
    qCDebug(dmMusic) << "Resuming playback";
    VlcMediaPlayer::resume();
    if (m_loadSdlLibrary) {
        SDL_GetAudioStatus_function GetAudioStatus = (SDL_GetAudioStatus_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSdlSymbol("SDL_GetAudioStatus");
        SDL_PauseAudio_function PauseAudio = (SDL_PauseAudio_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSdlSymbol("SDL_PauseAudio");
        SDL_OpenAudio_function OpenAudio = (SDL_OpenAudio_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSdlSymbol("SDL_OpenAudio");
        SDL_Delay_function Delay = (SDL_Delay_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSdlSymbol("SDL_Delay");
        if (GetAudioStatus() == SDL_AUDIO_STOPPED) {
            OpenAudio(&obtainedAS, nullptr);
            qCDebug(dmMusic) << "Reopened audio device";
        }

        if ((GetAudioStatus() != SDL_AUDIO_STOPPED)) {
            Delay(40);
            PauseAudio(0);
            qCDebug(dmMusic) << "Audio resumed";
        }
    }
}

void SdlPlayer::stop()
{
    if (!_vlcMediaPlayer) {
        qCWarning(dmMusic) << "Cannot stop: media player is null";
        return;
    }
    qCDebug(dmMusic) << "Stopping playback";
    VlcMediaPlayer::stop();
    if (m_loadSdlLibrary) {
        cleanMemCache();
        SDL_PauseAudio_function PauseAudio = (SDL_PauseAudio_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSdlSymbol("SDL_PauseAudio");
        SDL_GetQueuedAudioSize_function GetQueuedAudioSize = (SDL_GetQueuedAudioSize_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSdlSymbol("SDL_GetQueuedAudioSize");
        SDL_ClearQueuedAudio_function ClearQueuedAudio = (SDL_ClearQueuedAudio_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSdlSymbol("SDL_ClearQueuedAudio");
        SDL_LockAudio_function LockAudio = (SDL_LockAudio_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSdlSymbol("SDL_LockAudio");
        SDL_UnlockAudio_function UnlockAudio = (SDL_UnlockAudio_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSdlSymbol("SDL_UnlockAudio");
        SDL_Delay_function Delay = (SDL_Delay_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSdlSymbol("SDL_Delay");
        SDL_CloseAudio_function CloseAudio = (SDL_CloseAudio_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSdlSymbol("SDL_CloseAudio");

        if (GetQueuedAudioSize(1) > 0) {
            ClearQueuedAudio(1);
            qCDebug(dmMusic) << "Cleared audio queue";
        }
        PauseAudio(1);
        LockAudio();
        Delay(40);
        UnlockAudio();
        if (qEnvironmentVariable("DDE_CURRENT_COMPOSITOR") != "TreeLand") {
            CloseAudio();
            qCDebug(dmMusic) << "Closed audio device";
        }
    }
}

int SdlPlayer::getVolume()
{
    return m_loadSdlLibrary ? m_volume : VlcMediaPlayer::getVolume();
}

bool SdlPlayer::getMute()
{
    return m_loadSdlLibrary ? m_mute : VlcMediaPlayer::getMute();
}

void SdlPlayer::setTime(qint64 time)
{
    VlcMediaPlayer::setTime(time);
    cleanMemCache(); //clear data when seek
}

void SdlPlayer::setVolume(int volume)
{
    if (m_loadSdlLibrary) {
        m_volume = volume;
    } else {
        VlcMediaPlayer::setVolume(volume);
    }
}

void SdlPlayer::setMute(bool mute)
{
    if (m_loadSdlLibrary)
        m_mute = mute;
    else {
        VlcMediaPlayer::setMute(mute);
    }
}

void SdlPlayer::checkDataZero()
{
    qCDebug(dmMusic) << "Checking for zero data condition, emitting end signal";
    emit end();
}

void SdlPlayer::setProgressTag(int prog)
{
    qCDebug(dmMusic) << "Setting progress tag to:" << prog;
    progressTag = prog;
}

void SdlPlayer::libvlc_audio_play_cb(void *data, const void *samples, unsigned count, int64_t pts)
{
    Q_UNUSED(pts)
    SdlPlayer *sdlMediaPlayer = static_cast<SdlPlayer *>(data);
    if (!sdlMediaPlayer) {
        return;
    }
    
    if (sdlMediaPlayer->progressTag) {
        return;
    }

    int size = count * sdlMediaPlayer->obtainedAS.channels * sdlMediaPlayer->_rate / 8;

    /** vlc解析的通道数超过设置到sdl的通道数时，声音会出现沙哑的问题，
        原因是SDL支持的最大channel数为6,当超过这个阈值时，SDL本身是不支持的，会按照正常的指针偏移去读取下一桢数据，
        导致解析出问题，声音沙哑。解决方案为舍弃掉samples中，超出SDL通道数的的数据，只拷贝SDL支持通道数内的数据。
    **/
    char curSamples[size];
    if (sdlMediaPlayer->_channels != sdlMediaPlayer->obtainedAS.channels) {
        for (int i = 0; i < count; ++i) {
            for (int j = 0; j < sdlMediaPlayer->obtainedAS.channels; ++j) {
                memcpy((curSamples + (i * sdlMediaPlayer->obtainedAS.channels + j) * (sdlMediaPlayer->_rate / 8)),
                       (char *)samples + (i * sdlMediaPlayer->_channels + j)*sdlMediaPlayer->_rate / 8,
                       sdlMediaPlayer->_rate / 8);
            }
        }
    } else {
        memcpy(curSamples, (char *)samples, size);
    }

    QByteArray ba((char *)curSamples, size);
    QMutexLocker locker(&vlc_mutex);
    sdlMediaPlayer->_data.append(ba);
}

void SdlPlayer::libvlc_audio_pause_cb(void *data, int64_t pts)
{
    Q_UNUSED(data)
    Q_UNUSED(pts)
    qCDebug(dmMusic) << "Audio pause callback triggered";
    SDL_GetAudioStatus_function GetAudioStatus = (SDL_GetAudioStatus_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSdlSymbol("SDL_GetAudioStatus");
    SDL_PauseAudio_function PauseAudio = (SDL_PauseAudio_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSdlSymbol("SDL_PauseAudio");
    if (GetAudioStatus() != SDL_AUDIO_PAUSED && GetAudioStatus() != SDL_AUDIO_STOPPED) {
        PauseAudio(1);
        qCDebug(dmMusic) << "Audio paused successfully";
    }
}

void SdlPlayer::libvlc_audio_resume_cb(void *data, int64_t pts)
{
    Q_UNUSED(data)
    Q_UNUSED(pts)
    qCDebug(dmMusic) << "Audio resume callback triggered";
    SDL_GetAudioStatus_function GetAudioStatus = (SDL_GetAudioStatus_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSdlSymbol("SDL_GetAudioStatus");
    SDL_PauseAudio_function PauseAudio = (SDL_PauseAudio_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSdlSymbol("SDL_PauseAudio");
    if (GetAudioStatus() != SDL_AUDIO_PLAYING) {
        PauseAudio(0);
        qCDebug(dmMusic) << "Audio resumed successfully";
    }
}

int SdlPlayer::libvlc_audio_setup_cb(void **data, char *format, unsigned *rate, unsigned *channels)
{
    qCDebug(dmMusic) << "Setting up audio - Format:" << format << "Rate:" << *rate << "Channels:" << *channels;
    
    SDL_PauseAudio_function PauseAudio = (SDL_PauseAudio_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSdlSymbol("SDL_PauseAudio");
    SDL_Delay_function Delay = (SDL_Delay_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSdlSymbol("SDL_Delay");
    SDL_OpenAudio_function OpenAudio = (SDL_OpenAudio_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSdlSymbol("SDL_OpenAudio");
    av_log2_function Log2 = (av_log2_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("av_log2", true);
    
    // 防御性编程：先检查data指针有效性，再解引用
    if (!data) {
        qCCritical(dmMusic) << "Null data pointer in audio setup";
        return -1;
    }
    SdlPlayer *sdlMediaPlayer = *(SdlPlayer **)data;
    if (!sdlMediaPlayer) {
        qCCritical(dmMusic) << "Invalid player instance in audio setup";
        return -1;
    }
    
    // 在确认指针有效后再暂停音频，避免早期返回时音频被永久暂停
    PauseAudio(1);
    
    sdlMediaPlayer->cleanMemCache();
    sdlMediaPlayer->_rate = libvlc_audio_format(format);
    sdlMediaPlayer->_channels = *channels;
    sdlMediaPlayer->_sampleRate = *rate;

    SDL_AudioSpec desiredAS;
    desiredAS.freq = sdlMediaPlayer->_sampleRate;
    desiredAS.format = format_from_vlc_to_SDL(format);
    desiredAS.channels = static_cast<uint8_t>(sdlMediaPlayer->_channels);
    desiredAS.silence = 0;
    desiredAS.samples = FFMAX(AUDIO_MIN_BUFFER_SIZE, 2 << Log2(desiredAS.freq / AUDIO_MAX_CALLBACKS_PER_SEC));
    desiredAS.callback = SDL_audio_cbk;
    desiredAS.userdata = sdlMediaPlayer;

    qCDebug(dmMusic) << "Opening audio device with spec - Freq:" << desiredAS.freq 
                     << "Format:" << desiredAS.format 
                     << "Channels:" << (int)desiredAS.channels
                     << "Samples:" << desiredAS.samples;

    if (OpenAudio(&desiredAS, &sdlMediaPlayer->obtainedAS) < 0) {
        qCCritical(dmMusic) << "Failed to open audio device";
        return -1;
    }

    Delay(40);
    PauseAudio(0);

    sdlMediaPlayer->resetVolume();
    sdlMediaPlayer->m_sinkInputPath.clear();
    qCDebug(dmMusic) << "Audio setup completed successfully";
    return 0;
}

unsigned int SdlPlayer::libvlc_audio_format(char *format)
{
    unsigned int formatNum = 16;
    QString formatStr(format);
    if (formatStr.contains("8")) {
        formatNum = 8;
    } else if (formatStr.contains("16")) {
        formatNum = 16;
    } else if (formatStr.contains("32")) {
        formatNum = 32;
    } else if (formatStr.contains("64")) {
        formatNum = 64;
    } else {
        formatNum = 16;
    }
    return formatNum;
}

unsigned int SdlPlayer::format_from_vlc_to_SDL(char *format)
{
    unsigned int formatNum = AUDIO_S16SYS;
    QString formatStr(format);
    if (formatStr.contains("8")) {
        if (formatStr.contains("u")) {
            formatNum = AUDIO_U8;
        } else {
            formatNum = AUDIO_S8;
        }
    } else if (formatStr.contains("16")) {
        if (formatStr.contains("u")) {
            formatNum = AUDIO_U16SYS;
        } else {
            formatNum = AUDIO_S16SYS;
        }
    } else if (formatStr.contains("32")) {
        if (formatStr.contains("f")) {
            formatNum = AUDIO_F32SYS;
        } else {
            formatNum = AUDIO_S32SYS;
        }
    } else {
        formatNum = 16;
    }
    return formatNum;
}

void SdlPlayer::libvlc_audio_flush_cb(void *data, int64_t pts)
{
    //we need it ,but do nothing
    Q_UNUSED(data)
    Q_UNUSED(pts)
}

void SdlPlayer::SDL_audio_cbk(void *userdata, uint8_t *stream, int len)
{
    SDL_memset_function Memset = (SDL_memset_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSdlSymbol("SDL_memset");
    SDL_MixAudio_function MixAudio = (SDL_MixAudio_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSdlSymbol("SDL_MixAudio");
    
    SdlPlayer *sdlMediaPlayer = static_cast<SdlPlayer *>(userdata);
    if (!sdlMediaPlayer) {
        qCWarning(dmMusic) << "Invalid player instance in audio callback";
        return;
    }

    Memset(stream, 0, size_t(len));
    
    if (sdlMediaPlayer->_data.isEmpty()) {
        if (g_playbackStatus == PLAYBACK_STATUS_CHANGING) {
            g_playbackStatus = PLAYBACK_STATUS_RESTORE;
            qCDebug(dmMusic) << "Playback status changed to restore due to empty buffer";
        }
        return;
    }

    if (sdlMediaPlayer->_data.size() >= len) {
        QMutexLocker locker(&vlc_mutex);
        QByteArray d = sdlMediaPlayer->_data.mid(0, len);
        sdlMediaPlayer->_data.remove(0, len);
        locker.unlock();
        MixAudio(stream, (uint8_t *)d.data(), size_t(d.size()), sdlMediaPlayer->m_mute ? 0 : sdlMediaPlayer->m_volume * SDL_MIX_MAXVOLUME / 100.0);
    } else {
        QMutexLocker locker(&vlc_mutex);
        QByteArray d = sdlMediaPlayer->_data;
        locker.unlock();
        MixAudio(stream, (uint8_t *)d.data(), size_t(d.size()), sdlMediaPlayer->m_mute ? 0 : sdlMediaPlayer->m_volume * SDL_MIX_MAXVOLUME / 100.0);
        sdlMediaPlayer->cleanMemCache();
    }
    sdlMediaPlayer->resetVolume();
}

void SdlPlayer::cleanMemCache()
{
    qCDebug(dmMusic) << "Cleaning memory cache";
    QMutexLocker locker(&vlc_mutex);
    _data.clear();
    qCDebug(dmMusic) << "Memory cache cleared";
}

void SdlPlayer::readSinkInputPath()
{
    // qCDebug(dmMusic) << "Reading sink input path";
    QVariant v = Utils::readDBusProperty("org.deepin.dde.Audio1", "/org/deepin/dde/Audio1",
                                             "org.deepin.dde.Audio1", "SinkInputs");

    if (!v.isValid()) {
        qCWarning(dmMusic) << "Failed to read sink inputs from DBus";
        return;
    }

    QList<QDBusObjectPath> allSinkInputsList = v.value<QList<QDBusObjectPath>>();
    for (auto curPath : allSinkInputsList) {
        QVariant nameV = Utils::readDBusProperty("org.deepin.dde.Audio1", curPath.path(),
                                                     "org.deepin.dde.Audio1.SinkInput", "Name");

        if (!nameV.isValid() || (nameV.toString() != "Deepin Music" && nameV.toString() != "deepin-music"))
            continue;

        m_sinkInputPath = curPath.path();
        // qCDebug(dmMusic) << "Found sink input path:" << m_sinkInputPath;
        break;
    }
}

void SdlPlayer::resetVolume()
{
    if (!m_sinkInputPath.isEmpty()) {
        qCDebug(dmMusic) << "Skipping volume reset - path already exists";
        return;
    }
    
    readSinkInputPath();
    if (m_sinkInputPath.isEmpty()) {
        qCDebug(dmMusic) << "No sink input path found for volume reset";
        return;
    }

    QVariant volumeV = Utils::readDBusProperty("org.deepin.dde.Audio1", m_sinkInputPath,
                                                   "org.deepin.dde.Audio1.SinkInput", "Volume");

    if (!volumeV.isValid()) {
        qCWarning(dmMusic) << "Failed to read volume from sink input";
        return;
    }

    QDBusInterface ainterface("org.deepin.dde.Audio1", m_sinkInputPath,
                              "org.deepin.dde.Audio1.SinkInput",
                              QDBusConnection::sessionBus());
    if (!ainterface.isValid()) {
        qCWarning(dmMusic) << "Invalid DBus interface for volume control";
        return;
    }

    if (!qFuzzyCompare(volumeV.toDouble(), 1.0)) {
        ainterface.call(QLatin1String("SetVolume"), 1.0, false);
        qCDebug(dmMusic) << "Volume reset to 1.0";
    }

    m_sinkInputPath.clear();
}
