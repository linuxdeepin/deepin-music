// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <cmath>
#include <vlc/vlc.h>

#include "Equalizer.h"
#include "error.h"
#include "MediaPlayer.h"
#include "dynamiclibraries.h"
#include "util/log.h"

typedef libvlc_equalizer_t *(*vlc_audio_equalizer_new_function)(void);
typedef void (*vlc_audio_equalizer_release_function)(libvlc_equalizer_t *);
typedef float (*vlc_audio_equalizer_get_amp_at_index_function)(libvlc_equalizer_t *, unsigned);
typedef unsigned(*vlc_audio_equalizer_get_band_count_function)(void);
typedef float (*vlc_audio_equalizer_get_band_frequency_function)(unsigned);
typedef float (*vlc_audio_equalizer_get_preamp_function)(libvlc_equalizer_t *);
typedef unsigned(*vlc_audio_equalizer_get_preset_count_function)(void);
typedef const char *(*vlc_audio_equalizer_get_preset_name_function)(unsigned);
typedef libvlc_equalizer_t *(*vlc_audio_equalizer_new_from_preset_function)(unsigned);
typedef int (*vlc_audio_equalizer_set_amp_at_index_function)(libvlc_equalizer_t *, float, unsigned);
typedef int (*vlc_media_player_set_equalizer_function)(libvlc_media_player_t *, libvlc_equalizer_t *);
typedef int (*vlc_audio_equalizer_set_preamp_function)(libvlc_equalizer_t *, float);

VlcEqualizer::VlcEqualizer(VlcMediaPlayer *vlcMediaPlayer)
    : QObject(vlcMediaPlayer),
      _vlcMediaPlayer(vlcMediaPlayer)
{
    qCDebug(dmMusic) << "Creating new VLC equalizer for media player";
    vlc_audio_equalizer_new_function vlc_audio_equalizer_new = (vlc_audio_equalizer_new_function)DynamicLibraries::instance()->resolve("libvlc_audio_equalizer_new");
    _vlcEqualizer = vlc_audio_equalizer_new();
    if (_vlcEqualizer) {
        qCDebug(dmMusic) << "VLC equalizer created successfully";
    } else {
        qCWarning(dmMusic) << "Failed to create VLC equalizer";
    }
}

VlcEqualizer::~VlcEqualizer()
{
    qCDebug(dmMusic) << "Destroying VLC equalizer";
    //释放均衡器
    if (_vlcEqualizer) {
        vlc_audio_equalizer_release_function vlc_audio_equalizer_release = (vlc_audio_equalizer_release_function)DynamicLibraries::instance()->resolve("libvlc_audio_equalizer_release");
        vlc_audio_equalizer_release(_vlcEqualizer);
        qCDebug(dmMusic) << "VLC equalizer released";
    }
}

float VlcEqualizer::amplificationForBandAt(uint bandIndex) const
{
    qCDebug(dmMusic) << "Getting amplification for band index:" << bandIndex;
    if (_vlcEqualizer) {
        vlc_audio_equalizer_get_amp_at_index_function vlc_audio_equalizer_get_amp_at_index = (vlc_audio_equalizer_get_amp_at_index_function)DynamicLibraries::instance()->resolve("libvlc_audio_equalizer_get_amp_at_index");
        float ret = vlc_audio_equalizer_get_amp_at_index(_vlcEqualizer, bandIndex);
        if (!std::isnan(ret)) {
            qCDebug(dmMusic) << "Amplification value for band" << bandIndex << "is" << ret;
            return ret;
        }
    }
    qCWarning(dmMusic) << "Failed to get amplification for band" << bandIndex;
    return -1.0;
}

//uint VlcEqualizer::bandCount() const
//{
//    if (_vlcEqualizer) {
//        vlc_audio_equalizer_get_band_count_function vlc_audio_equalizer_get_band_count = (vlc_audio_equalizer_get_band_count_function)DynamicLibraries::instance()->resolve("libvlc_audio_equalizer_get_band_count");
//        return vlc_audio_equalizer_get_band_count();
//    } else {
//        return 0;
//    }
//}

//float VlcEqualizer::bandFrequency(uint bandIndex) const
//{
//    if (_vlcEqualizer) {
//        vlc_audio_equalizer_get_band_frequency_function vlc_audio_equalizer_get_band_frequency = (vlc_audio_equalizer_get_band_frequency_function)DynamicLibraries::instance()->resolve("libvlc_audio_equalizer_get_band_frequency");
//        return vlc_audio_equalizer_get_band_frequency(bandIndex);
//    } else {
//        return -1.0;
//    }
//}

float VlcEqualizer::preamplification() const
{
    qCDebug(dmMusic) << "Getting preamplification value";
    if (_vlcEqualizer) {
        vlc_audio_equalizer_get_preamp_function vlc_audio_equalizer_get_preamp = (vlc_audio_equalizer_get_preamp_function)DynamicLibraries::instance()->resolve("libvlc_audio_equalizer_get_preamp");
        float value = vlc_audio_equalizer_get_preamp(_vlcEqualizer);
        qCDebug(dmMusic) << "Preamplification value is" << value;
        return value;
    } else {
        qCWarning(dmMusic) << "No equalizer available for preamplification";
        return 0.0;
    }
}

//uint VlcEqualizer::presetCount() const
//{
//    vlc_audio_equalizer_get_preset_count_function vlc_audio_equalizer_get_preset_count = (vlc_audio_equalizer_get_preset_count_function)DynamicLibraries::instance()->resolve("libvlc_audio_equalizer_get_preset_count");
//    return vlc_audio_equalizer_get_preset_count();
//}

//QString VlcEqualizer::presetNameAt(uint index) const
//{
//    vlc_audio_equalizer_get_preset_name_function vlc_audio_equalizer_get_preset_name = (vlc_audio_equalizer_get_preset_name_function)DynamicLibraries::instance()->resolve("libvlc_audio_equalizer_get_preset_name");
//    const char *name = vlc_audio_equalizer_get_preset_name(index);
//    if (name == NULL) {
//        return QString();
//    } else {
//        return QString(name);
//    }
//}

void VlcEqualizer::loadFromPreset(uint index)
{
    qCDebug(dmMusic) << "Loading equalizer preset from index:" << index;
    if (_vlcEqualizer) {
        vlc_audio_equalizer_release_function vlc_audio_equalizer_release = (vlc_audio_equalizer_release_function)DynamicLibraries::instance()->resolve("libvlc_audio_equalizer_release");
        vlc_audio_equalizer_release(_vlcEqualizer);
        qCDebug(dmMusic) << "Released previous equalizer settings";
    }
    //18：The custom mode
    if (index < 18) {
        vlc_audio_equalizer_new_from_preset_function vlc_audio_equalizer_new_from_preset = (vlc_audio_equalizer_new_from_preset_function)DynamicLibraries::instance()->resolve("libvlc_audio_equalizer_new_from_preset");
        _vlcEqualizer = vlc_audio_equalizer_new_from_preset(index);
        if (_vlcEqualizer) {
            qCDebug(dmMusic) << "Successfully loaded preset" << index;
            emit presetLoaded();
        } else {
            qCWarning(dmMusic) << "Failed to load preset" << index;
        }
    } else {
        vlc_audio_equalizer_new_function vlc_audio_equalizer_new = (vlc_audio_equalizer_new_function)DynamicLibraries::instance()->resolve("libvlc_audio_equalizer_new");
        _vlcEqualizer = vlc_audio_equalizer_new();
        qCDebug(dmMusic) << "Created new custom equalizer settings";
    }
}

void VlcEqualizer::setAmplificationForBandAt(float amp, uint bandIndex)
{
    qCDebug(dmMusic) << "Setting amplification for band" << bandIndex << "to" << amp;
    if (!_vlcEqualizer) {
        qCWarning(dmMusic) << "No equalizer available to set amplification";
        return;
    }
    vlc_audio_equalizer_set_amp_at_index_function vlc_audio_equalizer_set_amp_at_index = (vlc_audio_equalizer_set_amp_at_index_function)DynamicLibraries::instance()->resolve("libvlc_audio_equalizer_set_amp_at_index");
    vlc_media_player_set_equalizer_function vlc_media_player_set_equalizer = (vlc_media_player_set_equalizer_function)DynamicLibraries::instance()->resolve("libvlc_media_player_set_equalizer");
    int result = vlc_audio_equalizer_set_amp_at_index(_vlcEqualizer, amp, bandIndex);
    if (result == 0) {
        qCDebug(dmMusic) << "Successfully set amplification for band" << bandIndex;
    } else {
        qCWarning(dmMusic) << "Failed to set amplification for band" << bandIndex;
    }
    vlc_media_player_set_equalizer(_vlcMediaPlayer->core(), _vlcEqualizer);
}

void VlcEqualizer::setEnabled(bool enabled)
{
    vlc_media_player_set_equalizer_function vlc_media_player_set_equalizer = (vlc_media_player_set_equalizer_function)DynamicLibraries::instance()->resolve("libvlc_media_player_set_equalizer");
    if (enabled && _vlcEqualizer != nullptr) {
        vlc_media_player_set_equalizer(_vlcMediaPlayer->core(), _vlcEqualizer);
    } else {
        vlc_media_player_set_equalizer(_vlcMediaPlayer->core(), nullptr);
    }
}

void VlcEqualizer::setPreamplification(float value)
{
    qCDebug(dmMusic) << "Setting preamplification to:" << value;
    if (!_vlcEqualizer) {
        qCWarning(dmMusic) << "No equalizer available to set preamplification";
        return;
    }
    vlc_media_player_set_equalizer_function vlc_media_player_set_equalizer = (vlc_media_player_set_equalizer_function)DynamicLibraries::instance()->resolve("libvlc_media_player_set_equalizer");
    vlc_audio_equalizer_set_preamp_function vlc_audio_equalizer_set_preamp = (vlc_audio_equalizer_set_preamp_function)DynamicLibraries::instance()->resolve("libvlc_audio_equalizer_set_preamp");
    int result = vlc_audio_equalizer_set_preamp(_vlcEqualizer, value);
    if (result == 0) {
        qCDebug(dmMusic) << "Successfully set preamplification value";
    } else {
        qCWarning(dmMusic) << "Failed to set preamplification value";
    }
    vlc_media_player_set_equalizer(_vlcMediaPlayer->core(), _vlcEqualizer);
}
