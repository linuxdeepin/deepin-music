// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
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
#include "util/log.h"

#include "dynamiclibraries.h"

#include <QDebug>

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
    vlc_media_player_new_function vlc_media_player_new = (vlc_media_player_new_function)DynamicLibraries::instance()->resolve("libvlc_media_player_new");
    vlc_media_player_event_manager_function vlc_media_player_event_manager = (vlc_media_player_event_manager_function)DynamicLibraries::instance()->resolve("libvlc_media_player_event_manager");
    config_PutInt_func config_PutInt_fc = (config_PutInt_func)DynamicLibraries::instance()->resolve("config_PutInt");
    var_SetChecked_func var_SetChecked_fc = (var_SetChecked_func)DynamicLibraries::instance()->resolve("var_SetChecked");

    _vlcMediaPlayer = vlc_media_player_new(instance->core());
    _vlcEvents = vlc_media_player_event_manager(_vlcMediaPlayer);

    VlcError::showErrmsg();

    //unuseless
    _vlcEqualizer = new VlcEqualizer(this);

    //屏蔽视频功能,只支持音频
    qCDebug(dmMusic) << "Configuring media player - disabling video, enabling CD audio";
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
    qCDebug(dmMusic) << "Destroying VLC media player instance";
    removeCoreConnections();
    //释放均衡器
    if (_vlcEqualizer) {
        delete _vlcEqualizer;
        _vlcEqualizer = nullptr;
        qCDebug(dmMusic) << "Equalizer released";
    }
    vlc_media_player_release_function vlc_media_player_release = (vlc_media_player_release_function)DynamicLibraries::instance()->resolve("libvlc_media_player_release");
    //释放播放器
    if (_vlcMediaPlayer) {
        vlc_media_player_release(_vlcMediaPlayer);
        _vlcEqualizer = nullptr;
        qCDebug(dmMusic) << "Media player released";
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
    qCDebug(dmMusic) << "Creating core media player connections";
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

    vlc_event_attach_function vlc_event_attach = (vlc_event_attach_function)DynamicLibraries::instance()->resolve("libvlc_event_attach");
    foreach (const libvlc_event_e &event, list) {
        vlc_event_attach(_vlcEvents, event, libvlc_callback, this);
    }
}

void VlcMediaPlayer::removeCoreConnections()
{
    qCDebug(dmMusic) << "Removing core media player connections";
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

    vlc_event_detach_function vlc_event_detach = (vlc_event_detach_function)DynamicLibraries::instance()->resolve("libvlc_event_detach");
    foreach (const libvlc_event_e &event, list) {
        vlc_event_detach(_vlcEvents, event, libvlc_callback, this);
    }
    qCDebug(dmMusic) << "Core media player connections removed successfully";
}

int VlcMediaPlayer::length() const
{
    vlc_media_player_get_length_function vlc_media_player_get_length = (vlc_media_player_get_length_function)DynamicLibraries::instance()->resolve("libvlc_media_player_get_length");
    libvlc_time_t length = vlc_media_player_get_length(_vlcMediaPlayer);

    VlcError::showErrmsg();

    return length;
}

void VlcMediaPlayer::open(VlcMedia *media)
{
    qCDebug(dmMusic) << "Opening media";
    //防止没打开文件
    if (media->core() == nullptr) {
        qCWarning(dmMusic) << "Cannot open media: media core is null";
        return;
    }
    vlc_media_player_set_media_function vlc_media_player_set_media = (vlc_media_player_set_media_function)DynamicLibraries::instance()->resolve("libvlc_media_player_set_media");
    config_PutInt_func config_PutInt_fc = (config_PutInt_func)DynamicLibraries::instance()->resolve("config_PutInt");
    int track = media->getCdaTrack();
    vlc_media_player_set_media(_vlcMediaPlayer, media->core());
    if (track >= 0) {
        qCDebug(dmMusic) << "Setting CDA track to:" << track;
        config_PutInt_fc((vlc_object_t *)_vlcMediaPlayer, "cdda-track", track);
    }

    VlcError::showErrmsg();
    qCDebug(dmMusic) << "Media opened successfully";
}

void VlcMediaPlayer::initCddaTrack()
{
    qCDebug(dmMusic) << "Initializing CDDA track";
    /**
     * important:do not modify it,it will open stream failed if does not set cdda-track to zero!
     **/
    config_PutInt_func config_PutInt_fc = (config_PutInt_func)DynamicLibraries::instance()->resolve("config_PutInt");
    config_PutInt_fc((vlc_object_t *)_vlcMediaPlayer, "cdda-track", 0);
    qCDebug(dmMusic) << "CDDA track initialized to 0";
}

void VlcMediaPlayer::play()
{
    if (!_vlcMediaPlayer) {
        qCWarning(dmMusic) << "Cannot play: media player is null";
        return;
    }
    qCDebug(dmMusic) << "Starting playback";
    vlc_media_player_play_function vlc_media_player_play = (vlc_media_player_play_function)DynamicLibraries::instance()->resolve("libvlc_media_player_play");
    vlc_media_player_play(_vlcMediaPlayer);

    VlcError::showErrmsg();
}

void VlcMediaPlayer::pause()
{
    if (!_vlcMediaPlayer) {
        qCWarning(dmMusic) << "Cannot pause: media player is null";
        return;
    }
    qCDebug(dmMusic) << "Attempting to pause playback";
    vlc_media_player_can_pause_function vlc_media_player_can_pause = (vlc_media_player_can_pause_function)DynamicLibraries::instance()->resolve("libvlc_media_player_can_pause");
    vlc_media_player_set_pause_function vlc_media_player_set_pause = (vlc_media_player_set_pause_function)DynamicLibraries::instance()->resolve("libvlc_media_player_set_pause");
    if (vlc_media_player_can_pause(_vlcMediaPlayer)) {
        vlc_media_player_set_pause(_vlcMediaPlayer, true);
        qCDebug(dmMusic) << "Playback paused";
    } else {
        qCWarning(dmMusic) << "Media cannot be paused";
    }

    VlcError::showErrmsg();
}

void VlcMediaPlayer::resume()
{
    if (!_vlcMediaPlayer) {
        qCWarning(dmMusic) << "Cannot resume: media player is null";
        return;
    }
    qCDebug(dmMusic) << "Attempting to resume playback";
    vlc_media_player_can_pause_function vlc_media_player_can_pause = (vlc_media_player_can_pause_function)DynamicLibraries::instance()->resolve("libvlc_media_player_can_pause");
    vlc_media_player_set_pause_function vlc_media_player_set_pause = (vlc_media_player_set_pause_function)DynamicLibraries::instance()->resolve("libvlc_media_player_set_pause");
    if (vlc_media_player_can_pause(_vlcMediaPlayer)) {
        vlc_media_player_set_pause(_vlcMediaPlayer, false);
        qCDebug(dmMusic) << "Playback resumed";
    } else {
        qCWarning(dmMusic) << "Media cannot be resumed";
    }

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
            ||  state() == Vlc::Opening)) {
        qCWarning(dmMusic) << "Cannot set time: invalid state" << state();
        return;
    }
#else
    if (!(state() == Vlc::Buffering
            || state() == Vlc::Playing
            || state() == Vlc::Paused)) {
        qCWarning(dmMusic) << "Cannot set time: invalid state" << state();
        return;
    }
#endif
    qCDebug(dmMusic) << "Setting time to:" << time;
    vlc_media_player_set_time_function vlc_media_player_set_time = (vlc_media_player_set_time_function)DynamicLibraries::instance()->resolve("libvlc_media_player_set_time");
    vlc_media_player_set_time(_vlcMediaPlayer, time);

    if (state() == Vlc::Paused)
        emit timeChanged(time);

    VlcError::showErrmsg();
}

void VlcMediaPlayer::setVolume(int volume)
{
    if (!_vlcMediaPlayer) {
        qCWarning(dmMusic) << "Cannot set volume: media player is null";
        return;
    }
    qCDebug(dmMusic) << "Setting volume to:" << volume;
    vlc_audio_set_volume_function vlc_audio_set_volume = (vlc_audio_set_volume_function)DynamicLibraries::instance()->resolve("libvlc_audio_set_volume");
    vlc_audio_set_volume(_vlcMediaPlayer, volume);

    VlcError::showErrmsg();
}

void VlcMediaPlayer::setMute(bool mute)
{
    if (!_vlcMediaPlayer) {
        qCWarning(dmMusic) << "Cannot set mute: media player is null";
        return;
    }
    qCDebug(dmMusic) << "Setting mute to:" << mute;
    vlc_audio_set_mute_function vlc_audio_set_mute = (vlc_audio_set_mute_function)DynamicLibraries::instance()->resolve("libvlc_audio_set_mute");
    vlc_audio_set_mute(_vlcMediaPlayer, mute ? 1 : 0);

    VlcError::showErrmsg();
}

Vlc::State VlcMediaPlayer::state() const
{
    qCDebug(dmMusic) << "Getting player state";
    // It's possible that the vlc doesn't play anything
    // so check before
    vlc_media_player_get_media_function vlc_media_player_get_media = (vlc_media_player_get_media_function)DynamicLibraries::instance()->resolve("libvlc_media_player_get_media");
    if (!vlc_media_player_get_media(_vlcMediaPlayer)) {
        qCDebug(dmMusic) << "No media loaded, returning Idle state";
        return Vlc::Idle;
    }

    libvlc_state_t state;
    vlc_media_player_get_state_function vlc_media_player_get_state = (vlc_media_player_get_state_function)DynamicLibraries::instance()->resolve("libvlc_media_player_get_state");
    state = vlc_media_player_get_state(_vlcMediaPlayer);
    qCDebug(dmMusic) << "Current player state:" << state;

    return Vlc::State(state);
}

void VlcMediaPlayer::stop()
{
    if (!_vlcMediaPlayer) {
        qCWarning(dmMusic) << "Cannot stop: media player is null";
        return;
    }
    qCDebug(dmMusic) << "Stopping playback";
    vlc_media_player_stop_function vlc_media_player_stop = (vlc_media_player_stop_function)DynamicLibraries::instance()->resolve("libvlc_media_player_stop");
    vlc_media_player_stop(_vlcMediaPlayer);

    VlcError::showErrmsg();
}

int VlcMediaPlayer::time() const
{
    qCDebug(dmMusic) << "Getting current time";
    vlc_media_player_get_time_function vlc_media_player_get_time = (vlc_media_player_get_time_function)DynamicLibraries::instance()->resolve("libvlc_media_player_get_time");
    libvlc_time_t time = vlc_media_player_get_time(_vlcMediaPlayer);

    VlcError::showErrmsg();
    qCDebug(dmMusic) << "Current time:" << time;
    return time;
}


void VlcMediaPlayer::libvlc_callback(const libvlc_event_t *event,
                                     void *data)
{
    VlcMediaPlayer *core = static_cast<VlcMediaPlayer *>(data);
    switch (event->type) {
    case libvlc_MediaPlayerMediaChanged:
        qCDebug(dmMusic) << "Media player event: Media changed";
        emit core->mediaChanged(event->u.media_player_media_changed.new_media);
        break;
    case libvlc_MediaPlayerNothingSpecial:
        qCDebug(dmMusic) << "Media player event: Nothing special";
        emit core->nothingSpecial();
        break;
    case libvlc_MediaPlayerOpening:
        qCDebug(dmMusic) << "Media player event: Opening";
        emit core->opening();
        break;
    case libvlc_MediaPlayerBuffering:
        qCDebug(dmMusic) << "Media player event: Buffering" << event->u.media_player_buffering.new_cache << "%";
        emit core->buffering(event->u.media_player_buffering.new_cache);
        emit core->buffering(qRound(event->u.media_player_buffering.new_cache));
        break;
    case libvlc_MediaPlayerPlaying:
        qCDebug(dmMusic) << "Media player event: Playing";
        emit core->playing();
        break;
    case libvlc_MediaPlayerPaused:
        qCDebug(dmMusic) << "Media player event: Paused";
        emit core->paused();
        break;
    case libvlc_MediaPlayerStopped:
        qCDebug(dmMusic) << "Media player event: Stopped";
        emit core->stopped();
        break;
    case libvlc_MediaPlayerForward:
        qCDebug(dmMusic) << "Media player event: Forward";
        emit core->forward();
        break;
    case libvlc_MediaPlayerBackward:
        qCDebug(dmMusic) << "Media player event: Backward";
        emit core->backward();
        break;
    case libvlc_MediaPlayerEndReached:
        qCDebug(dmMusic) << "Media player event: End reached";
        emit core->end(); //play end
        break;
    case libvlc_MediaPlayerEncounteredError:
        qCCritical(dmMusic) << "Media player event: Error encountered";
        emit core->error();
        break;
//    case libvlc_MediaPlayerTimeChanged:
//        emit core->timeChanged(event->u.media_player_time_changed.new_time);
//        break;
    case libvlc_MediaPlayerPositionChanged:
        // qCDebug(dmMusic) << "Media player event: Position changed to" << event->u.media_player_position_changed.new_position;
        emit core->positionChanged(event->u.media_player_position_changed.new_position);
        break;
    case libvlc_MediaPlayerSeekableChanged:
        qCDebug(dmMusic) << "Media player event: Seekable changed to" << event->u.media_player_seekable_changed.new_seekable;
        emit core->seekableChanged(event->u.media_player_seekable_changed.new_seekable);
        break;
    case libvlc_MediaPlayerPausableChanged:
        qCDebug(dmMusic) << "Media player event: Pausable changed to" << event->u.media_player_pausable_changed.new_pausable;
        emit core->pausableChanged(event->u.media_player_pausable_changed.new_pausable);
        break;
    case libvlc_MediaPlayerTitleChanged:
        qCDebug(dmMusic) << "Media player event: Title changed to" << event->u.media_player_title_changed.new_title;
        emit core->titleChanged(event->u.media_player_title_changed.new_title);
        break;
    case libvlc_MediaPlayerSnapshotTaken:
        qCDebug(dmMusic) << "Media player event: Snapshot taken" << event->u.media_player_snapshot_taken.psz_filename;
        emit core->snapshotTaken(event->u.media_player_snapshot_taken.psz_filename);
        break;
    case libvlc_MediaPlayerLengthChanged:
        qCDebug(dmMusic) << "Media player event: Length changed to" << event->u.media_player_length_changed.new_length;
        emit core->lengthChanged(event->u.media_player_length_changed.new_length);
        break;
    case libvlc_MediaPlayerVout:
        qCDebug(dmMusic) << "Media player event: Video output count changed to" << event->u.media_player_vout.new_count;
        emit core->vout(event->u.media_player_vout.new_count);
        break;
    default:
        // qCDebug(dmMusic) << "Media player event: Unknown event type" << event->type;
        break;
    }

    if (event->type >= libvlc_MediaPlayerNothingSpecial
            && event->type <= libvlc_MediaPlayerEncounteredError) {
        qCDebug(dmMusic) << "Media player state changed";
        emit core->stateChanged();
    }
}

float VlcMediaPlayer::position()
{
    if (!_vlcMediaPlayer) {
        qCWarning(dmMusic) << "Cannot get position: media player is null";
        return -1;
    }
    qCDebug(dmMusic) << "Getting playback position";
    vlc_media_player_get_position_function vlc_media_player_get_position = (vlc_media_player_get_position_function)DynamicLibraries::instance()->resolve("libvlc_media_player_get_position");
    float pos = vlc_media_player_get_position(_vlcMediaPlayer);
    qCDebug(dmMusic) << "Current position:" << pos;
    return pos;
}

int VlcMediaPlayer::getVolume()
{
    if (!_vlcMediaPlayer) {
        qCWarning(dmMusic) << "Cannot get volume: media player is null";
        return -1;
    }
    qCDebug(dmMusic) << "Getting volume";
    vlc_audio_get_volume_function vlc_audio_get_volume = (vlc_audio_get_volume_function)DynamicLibraries::instance()->resolve("libvlc_audio_get_volume");
    int vol = vlc_audio_get_volume(_vlcMediaPlayer);
    qCDebug(dmMusic) << "Current volume:" << vol;
    return vol;
}

bool VlcMediaPlayer::getMute()
{
    if (!_vlcMediaPlayer) {
        qCWarning(dmMusic) << "Cannot get mute state: media player is null";
        return -1;
    }
    qCDebug(dmMusic) << "Getting mute state";
    vlc_audio_get_mute_function vlc_audio_get_mute = (vlc_audio_get_mute_function)DynamicLibraries::instance()->resolve("libvlc_audio_get_mute");
    bool muted = vlc_audio_get_mute(_vlcMediaPlayer) > 0;
    qCDebug(dmMusic) << "Current mute state:" << muted;
    return muted;
}

void VlcMediaPlayer::setPosition(float pos)
{
    qCDebug(dmMusic) << "Setting position to:" << pos;
    vlc_media_player_set_position_function vlc_media_player_set_position = (vlc_media_player_set_position_function)DynamicLibraries::instance()->resolve("libvlc_media_player_set_position");
    vlc_media_player_set_position(_vlcMediaPlayer, pos);

    VlcError::showErrmsg();
}
