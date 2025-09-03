// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SDLPLAYER_H_
#define SDLPLAYER_H_

#include "MediaPlayer.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_error.h>

class CheckDataZeroThread;
class CheckDataCachingThread;

class VLCQT_CORE_EXPORT SdlPlayer : public VlcMediaPlayer
{
    Q_OBJECT
public:
    /*!
        \brief SdlPlayer constructor.

        This is mandatory to use libvlc playback functions.

        \param instance instance object (VlcInstance *)
    */
    explicit SdlPlayer(VlcInstance *instance);

    /*!
        SdlPlayer destructor
    */
    ~SdlPlayer();

    /*!
        \brief Open media file or stream. Any media should be playable and opened.
        \param media object (VlcMedia *)
    */
    void open(VlcMedia *media);

    /*!
        \brief Starts playing current media if possible
    */
    void play();

    /*!
        \brief Pauses the playback of current media if possible
    */
    void pause();

    /*!
        \brief Pauses the playback of current media if possible
    */
    void pauseNew();

    /*!
        \brief Resumes the playback of current media if possible
    */
    void resume();

    /*!
        \brief Stops playing current media
    */
    void stop();

    /**
     * Get current software audio volume.
     *
     * \return the software volume in percents
     * (0 = mute, 100 = nominal / 0dB)
     */
    int getVolume();

    /**
     * Get current mute status.
     *
     * \return the mute status (boolean) if defined, -1 if undefined/unapplicable
     */
    bool getMute();

    void setCachingThreadPause(bool pause);

public slots:
    /*! \brief Set the movie time (in ms).

        This has no effect if no media is being played. Not all formats and protocols support this.

        \param time the movie time (in ms) (int)
    */
    void setTime(qint64 time);

    /**
     * Set current software audio volume.
     *
     * \param i_volume the volume in percents (0 = mute, 100 = 0dB)
     * \return 0 if the volume was set, -1 if it was out of range
     */
    void setVolume(int volume);

    /**
     * Set mute status.
     *
     * \param status If status is true then mute, otherwise unmute
     * \warning This function does not always work. If there are no active audio
     * playback stream, the mute status might not be available. If digital
     * pass-through (S/PDIF, HDMI...) is in use, muting may be unapplicable. Also
     * some audio output plugins do not support muting at all.
     * \note To force silent playback, disable all audio tracks. This is more
     * efficient and reliable than mute.
     */
    void setMute(bool mute);

    // 定时检测，vlc的写的内存数据是否被SDL消耗完成
    void checkDataZero();

    /*!
        \brief set progress flag when play
    */
    void setProgressTag(int prog = -1);

private:
    static void libvlc_audio_play_cb(void *data, const void *samples,
                                     unsigned count, int64_t pts);

    static void libvlc_audio_pause_cb(void *data, int64_t pts);

    static void libvlc_audio_resume_cb(void *data, int64_t pts);

    static int libvlc_audio_setup_cb(void **data, char *format, unsigned *rate,
                                     unsigned *channels);

    static unsigned int libvlc_audio_format(char *format);

    static unsigned int format_from_vlc_to_SDL(char *format);

    static void libvlc_audio_flush_cb(void *data, int64_t pts);

    static void SDL_audio_cbk(void *userdata, uint8_t *stream, int len);

    //清空缓存数据
    void cleanMemCache();

    void readSinkInputPath();

    void resetVolume();

    static void switchToDefaultSink();

    // 将状态码翻译成文字信息，人类可读
    static QString transPaContextState(int code);

public:
    unsigned int _rate, _channels, _sampleRate;

private:
    QString m_sinkInputPath;
    SDL_AudioSpec obtainedAS;
    bool m_loadSdlLibrary;

//    QByteArray _data;
    int progressTag = 0;
    int m_volume = 50.0;
    bool m_mute = false;

    CheckDataZeroThread *m_pCheckDataThread = nullptr;
    CheckDataCachingThread *m_pCheckDataChingThread = nullptr;

    static int  switchOnceFlag;
};

#endif // SDLPLAYER_H_
