// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <vlc/vlc.h>
#include <vlc_common.h>
#include <vlc_variables.h>
#include <vlc_plugin.h>

#include "Error.h"
#include "Instance.h"
#include "Media.h"
#include "MediaPlayer.h"
#include "Equalizer.h"

#include "core/vlc/vlcdynamicinstance.h"

#include <QDebug>

extern int g_playbackStatus;

typedef libvlc_media_player_t *(*vlc_media_player_new_function)(libvlc_instance_t *);
typedef libvlc_event_manager_t *(*vlc_media_player_event_manager_function)(libvlc_media_player_t *);
typedef void (*vlc_media_player_release_function)(libvlc_media_player_t *);
typedef int (*vlc_event_attach_function)(libvlc_event_manager_t *,
                                         libvlc_event_type_t,
                                         libvlc_callback_t,
                                         void *);
typedef void (*vlc_event_detach_function)(libvlc_event_manager_t *,
                                          libvlc_event_type_t,
                                          libvlc_callback_t,
                                          void *);
typedef unsigned(*vlc_media_player_has_vout_function)(libvlc_media_player_t *);
typedef libvlc_time_t (*vlc_media_player_get_length_function)(libvlc_media_player_t *);
typedef libvlc_media_t *(*vlc_media_player_get_media_function)(libvlc_media_player_t *);
typedef void (*vlc_media_player_set_media_function)(libvlc_media_player_t *,
                                                    libvlc_media_t *);

typedef int (*vlc_media_player_play_function)(libvlc_media_player_t *);
typedef int (*vlc_media_player_can_pause_function)(libvlc_media_player_t *);
typedef void (*vlc_media_player_set_pause_function)(libvlc_media_player_t *,
                                                    int);
typedef void (*vlc_media_player_pause_function)(libvlc_media_player_t *);
typedef void (*vlc_media_player_set_time_function)(libvlc_media_player_t *, libvlc_time_t);
typedef int (*vlc_media_player_is_seekable_function)(libvlc_media_player_t *);
typedef libvlc_state_t (*vlc_media_player_get_state_function)(libvlc_media_player_t *);
typedef void (*vlc_media_player_stop_function)(libvlc_media_player_t *);
typedef libvlc_time_t (*vlc_media_player_get_time_function)(libvlc_media_player_t *);
typedef float (*vlc_media_player_get_position_function)(libvlc_media_player_t *);
typedef unsigned(*vlc_media_tracks_get_function)(libvlc_media_t *,
                                                 libvlc_media_track_t ***);

typedef void (*vlc_media_tracks_release_function)(libvlc_media_track_t **,
                                                  unsigned);
typedef void (*vlc_media_player_set_position_function)(libvlc_media_player_t *, float);

typedef int (*vlc_media_player_set_rate_function)(libvlc_media_player_t *, float);

typedef float (*vlc_media_player_get_rate_function)(libvlc_media_player_t *);

typedef void (*config_PutInt_func)(vlc_object_t *, const char *, int64_t);

typedef int (*var_SetChecked_func)(vlc_object_t *, const char *, int, vlc_value_t);

typedef int (*vlc_audio_set_volume_function)(libvlc_media_player_t *, int);
typedef int (*vlc_audio_get_volume_function)(libvlc_media_player_t *);
typedef int (*vlc_audio_set_mute_function)(libvlc_media_player_t *, int);
typedef int (*vlc_audio_get_mute_function)(libvlc_media_player_t *);

VlcMediaPlayer::VlcMediaPlayer(VlcInstance *instance)
    : QObject(instance)
{
    vlc_media_player_new_function vlc_media_player_new = (vlc_media_player_new_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("libvlc_media_player_new");
    vlc_media_player_event_manager_function vlc_media_player_event_manager = (vlc_media_player_event_manager_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("libvlc_media_player_event_manager");
    config_PutInt_func config_PutInt_fc = (config_PutInt_func)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("config_PutInt");
    var_SetChecked_func var_SetChecked_fc = (var_SetChecked_func)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("var_SetChecked");

    _vlcMediaPlayer = vlc_media_player_new(instance->core());
    _vlcEvents = vlc_media_player_event_manager(_vlcMediaPlayer);

    VlcError::showErrmsg();

    //unuseless
    _vlcEqualizer = new VlcEqualizer(this);

    //屏蔽视频功能,只支持音频
    config_PutInt_fc((vlc_object_t *)_vlcMediaPlayer, "video", 0); //0=disable
    //cdda plugin使能
    config_PutInt_fc((vlc_object_t *)_vlcMediaPlayer, "cd-audio", 1);
    //设置role
    vlc_value_t val;
    val.psz_string = const_cast<char *>("music");
    qDebug() << __FUNCTION__ << val.psz_string;
    var_SetChecked_fc((vlc_object_t *)_vlcMediaPlayer, "role", VLC_VAR_STRING, val);

    createCoreConnections();

    VlcError::showErrmsg();
}

VlcMediaPlayer::~VlcMediaPlayer()
{
    removeCoreConnections();
    //释放均衡器
    if (_vlcEqualizer) {
        delete _vlcEqualizer;
        _vlcEqualizer = nullptr;
    }
    vlc_media_player_release_function vlc_media_player_release = (vlc_media_player_release_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("libvlc_media_player_release");
    //释放播放器
    if (_vlcMediaPlayer) {
        vlc_media_player_release(_vlcMediaPlayer);
        _vlcEqualizer = nullptr;
    }
}

libvlc_media_player_t *VlcMediaPlayer::core() const
{
    return _vlcMediaPlayer;
}

VlcEqualizer *VlcMediaPlayer::equalizer() const
{
    return _vlcEqualizer;
}

void VlcMediaPlayer::createCoreConnections()
{
    QList<libvlc_event_e> list;
    list << libvlc_MediaPlayerMediaChanged
         << libvlc_MediaPlayerNothingSpecial
         << libvlc_MediaPlayerOpening
         << libvlc_MediaPlayerBuffering
         << libvlc_MediaPlayerPlaying
         << libvlc_MediaPlayerPaused
         << libvlc_MediaPlayerStopped
         << libvlc_MediaPlayerForward
         << libvlc_MediaPlayerBackward
         << libvlc_MediaPlayerEndReached
         << libvlc_MediaPlayerEncounteredError
         << libvlc_MediaPlayerTimeChanged
         << libvlc_MediaPlayerPositionChanged
         << libvlc_MediaPlayerSeekableChanged
         << libvlc_MediaPlayerPausableChanged
         << libvlc_MediaPlayerTitleChanged
         << libvlc_MediaPlayerSnapshotTaken
         << libvlc_MediaPlayerLengthChanged
         << libvlc_MediaPlayerVout;

    vlc_event_attach_function vlc_event_attach = (vlc_event_attach_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("libvlc_event_attach");
    foreach (const libvlc_event_e &event, list) {
        vlc_event_attach(_vlcEvents, event, libvlc_callback, this);
    }
}

void VlcMediaPlayer::removeCoreConnections()
{
    QList<libvlc_event_e> list;
    list << libvlc_MediaPlayerMediaChanged
         << libvlc_MediaPlayerNothingSpecial
         << libvlc_MediaPlayerOpening
         << libvlc_MediaPlayerBuffering
         << libvlc_MediaPlayerPlaying
         << libvlc_MediaPlayerPaused
         << libvlc_MediaPlayerStopped
         << libvlc_MediaPlayerForward
         << libvlc_MediaPlayerBackward
         << libvlc_MediaPlayerEndReached
         << libvlc_MediaPlayerEncounteredError
         << libvlc_MediaPlayerTimeChanged
         << libvlc_MediaPlayerPositionChanged
         << libvlc_MediaPlayerSeekableChanged
         << libvlc_MediaPlayerPausableChanged
         << libvlc_MediaPlayerTitleChanged
         << libvlc_MediaPlayerSnapshotTaken
         << libvlc_MediaPlayerLengthChanged
         << libvlc_MediaPlayerVout;

    vlc_event_detach_function vlc_event_detach = (vlc_event_detach_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("libvlc_event_detach");
    foreach (const libvlc_event_e &event, list) {
        vlc_event_detach(_vlcEvents, event, libvlc_callback, this);
    }
}

int VlcMediaPlayer::length() const
{
    vlc_media_player_get_length_function vlc_media_player_get_length = (vlc_media_player_get_length_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("libvlc_media_player_get_length");
    libvlc_time_t length = vlc_media_player_get_length(_vlcMediaPlayer);

    VlcError::showErrmsg();

    return length;
}

void VlcMediaPlayer::open(VlcMedia *media)
{
    //防止没打开文件
    if (media->core() == nullptr)
        return;
    vlc_media_player_set_media_function vlc_media_player_set_media = (vlc_media_player_set_media_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("libvlc_media_player_set_media");
    config_PutInt_func config_PutInt_fc = (config_PutInt_func)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("config_PutInt");
    int track = media->getCdaTrack();
    vlc_media_player_set_media(_vlcMediaPlayer, media->core());
    if (track >= 0) {
        config_PutInt_fc((vlc_object_t *)_vlcMediaPlayer, "cdda-track", track);
    }

    VlcError::showErrmsg();
}

void VlcMediaPlayer::initCddaTrack()
{
    /**
     * important:do not modify it,it will open stream failed if does not set cdda-track to zero!
     **/
    config_PutInt_func config_PutInt_fc = (config_PutInt_func)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("config_PutInt");
    config_PutInt_fc((vlc_object_t *)_vlcMediaPlayer, "cdda-track", 0);
}

void VlcMediaPlayer::play()
{
    if (!_vlcMediaPlayer)
        return;
    vlc_media_player_play_function vlc_media_player_play = (vlc_media_player_play_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("libvlc_media_player_play");
    vlc_media_player_play(_vlcMediaPlayer);

    VlcError::showErrmsg();
}

void VlcMediaPlayer::pause()
{
    if (!_vlcMediaPlayer)
        return;
    vlc_media_player_can_pause_function vlc_media_player_can_pause = (vlc_media_player_can_pause_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("libvlc_media_player_can_pause");
    vlc_media_player_set_pause_function vlc_media_player_set_pause = (vlc_media_player_set_pause_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("libvlc_media_player_set_pause");
    if (vlc_media_player_can_pause(_vlcMediaPlayer))
        vlc_media_player_set_pause(_vlcMediaPlayer, true);

    VlcError::showErrmsg();
}

void VlcMediaPlayer::resume()
{
    if (!_vlcMediaPlayer)
        return;
    vlc_media_player_can_pause_function vlc_media_player_can_pause = (vlc_media_player_can_pause_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("libvlc_media_player_can_pause");
    vlc_media_player_set_pause_function vlc_media_player_set_pause = (vlc_media_player_set_pause_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("libvlc_media_player_set_pause");
    if (vlc_media_player_can_pause(_vlcMediaPlayer))
        vlc_media_player_set_pause(_vlcMediaPlayer, false);

    VlcError::showErrmsg();
}

void VlcMediaPlayer::setTime(qint64 time)
{
    /*****************************************
     *  add enum Opening to set progress value
     * ***************************************/
#ifdef QT_DEBUG
    if (!(state() == Vlc::Buffering
          || state() == Vlc::Playing
          || state() == Vlc::Paused
          || state() == Vlc::Opening))
        return;
#else
    if (!(state() == Vlc::Buffering
            || state() == Vlc::Playing
            || state() == Vlc::Paused))
        return;
#endif
    vlc_media_player_set_time_function vlc_media_player_set_time = (vlc_media_player_set_time_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("libvlc_media_player_set_time");
    vlc_media_player_set_time(_vlcMediaPlayer, time);

    if (state() == Vlc::Paused)
        emit timeChanged(time);

    VlcError::showErrmsg();
}

void VlcMediaPlayer::setVolume(int volume)
{
    if (!_vlcMediaPlayer)
        return;
    vlc_audio_set_volume_function vlc_audio_set_volume = (vlc_audio_set_volume_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("libvlc_audio_set_volume");
    vlc_audio_set_volume(_vlcMediaPlayer, volume);

    VlcError::showErrmsg();
}

void VlcMediaPlayer::setMute(bool mute)
{
    if (!_vlcMediaPlayer)
        return;
    vlc_audio_set_mute_function vlc_audio_set_mute = (vlc_audio_set_mute_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("libvlc_audio_set_mute");
    vlc_audio_set_mute(_vlcMediaPlayer, mute ? 1 : 0);

    VlcError::showErrmsg();
}

Vlc::State VlcMediaPlayer::state() const
{
    // It's possible that the vlc doesn't play anything
    // so check before
    vlc_media_player_get_media_function vlc_media_player_get_media = (vlc_media_player_get_media_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("libvlc_media_player_get_media");
    if (!vlc_media_player_get_media(_vlcMediaPlayer))
        return Vlc::Idle;

    libvlc_state_t state;
    vlc_media_player_get_state_function vlc_media_player_get_state = (vlc_media_player_get_state_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("libvlc_media_player_get_state");
    state = vlc_media_player_get_state(_vlcMediaPlayer);

    return Vlc::State(state);
}
void VlcMediaPlayer::stop()
{
    if (!_vlcMediaPlayer)
        return;
    vlc_media_player_stop_function vlc_media_player_stop = (vlc_media_player_stop_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("libvlc_media_player_stop");
    vlc_media_player_stop(_vlcMediaPlayer);

    VlcError::showErrmsg();
}

int VlcMediaPlayer::time() const
{
    vlc_media_player_get_time_function vlc_media_player_get_time = (vlc_media_player_get_time_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("libvlc_media_player_get_time");
    libvlc_time_t time = vlc_media_player_get_time(_vlcMediaPlayer);

    VlcError::showErrmsg();

    return time;
}


void VlcMediaPlayer::libvlc_callback(const libvlc_event_t *event,
                                     void *data)
{
    VlcMediaPlayer *core = static_cast<VlcMediaPlayer *>(data);
    switch (event->type) {
    case libvlc_MediaPlayerMediaChanged:
        emit core->mediaChanged(event->u.media_player_media_changed.new_media);
        break;
    case libvlc_MediaPlayerNothingSpecial:
        emit core->nothingSpecial();
        break;
    case libvlc_MediaPlayerOpening:
        emit core->opening();
        break;
    case libvlc_MediaPlayerBuffering:
        emit core->buffering(event->u.media_player_buffering.new_cache);
        emit core->buffering(qRound(event->u.media_player_buffering.new_cache));
        break;
    case libvlc_MediaPlayerPlaying:
        emit core->playing();
        break;
    case libvlc_MediaPlayerPaused:
        emit core->paused();
        break;
    case libvlc_MediaPlayerStopped:
        emit core->stopped();
        break;
    case libvlc_MediaPlayerForward:
        emit core->forward();
        break;
    case libvlc_MediaPlayerBackward:
        emit core->backward();
        break;
    case libvlc_MediaPlayerEndReached:
        g_playbackStatus = 0;
        if (core->_data.isEmpty()) {
            emit core->end(); //play end
        } else {
            g_playbackStatus = 1;
            // 设置当前进度时间、步进、总时间
            emit core->endReached();
        }
        break;
    case libvlc_MediaPlayerEncounteredError:
        emit core->error();
        break;
//    case libvlc_MediaPlayerTimeChanged:
//        emit core->timeChanged(event->u.media_player_time_changed.new_time);
//        break;
    case libvlc_MediaPlayerPositionChanged:
        emit core->positionChanged(event->u.media_player_position_changed.new_position);
        break;
    case libvlc_MediaPlayerSeekableChanged:
        emit core->seekableChanged(event->u.media_player_seekable_changed.new_seekable);
        break;
    case libvlc_MediaPlayerPausableChanged:
        emit core->pausableChanged(event->u.media_player_pausable_changed.new_pausable);
        break;
    case libvlc_MediaPlayerTitleChanged:
        emit core->titleChanged(event->u.media_player_title_changed.new_title);
        break;
    case libvlc_MediaPlayerSnapshotTaken:
        emit core->snapshotTaken(event->u.media_player_snapshot_taken.psz_filename);
        break;
    case libvlc_MediaPlayerLengthChanged:
        emit core->lengthChanged(event->u.media_player_length_changed.new_length);
        break;
    case libvlc_MediaPlayerVout:
        emit core->vout(event->u.media_player_vout.new_count);
        break;
    default:
        break;
    }

    if (event->type >= libvlc_MediaPlayerNothingSpecial
            && event->type <= libvlc_MediaPlayerEncounteredError) {
        emit core->stateChanged();
    }
}

float VlcMediaPlayer::position()
{
    if (!_vlcMediaPlayer)
        return -1;
    vlc_media_player_get_position_function vlc_media_player_get_position = (vlc_media_player_get_position_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("libvlc_media_player_get_position");
    return vlc_media_player_get_position(_vlcMediaPlayer);
}

int VlcMediaPlayer::getVolume()
{
    if (!_vlcMediaPlayer)
        return -1;
    vlc_audio_get_volume_function vlc_audio_get_volume = (vlc_audio_get_volume_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("libvlc_audio_get_volume");
    return vlc_audio_get_volume(_vlcMediaPlayer);
}

bool VlcMediaPlayer::getMute()
{
    if (!_vlcMediaPlayer)
        return -1;
    vlc_audio_get_mute_function vlc_audio_get_mute = (vlc_audio_get_mute_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("libvlc_audio_get_mute");
    return vlc_audio_get_mute(_vlcMediaPlayer) > 0 ? true : false;
}

void VlcMediaPlayer::setPosition(float pos)
{
    vlc_media_player_set_position_function vlc_media_player_set_position = (vlc_media_player_set_position_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("libvlc_media_player_set_position");
    vlc_media_player_set_position(_vlcMediaPlayer, pos);

    VlcError::showErrmsg();
}
