/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     ZouYa <zouya@uniontech.com>
 *
 * Maintainer: WangYu <wangyu@uniontech.com>
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

#include <cmath>
#include <vlc/vlc.h>

#include "Equalizer.h"
#include "error.h"
#include "MediaPlayer.h"
#include "core/vlc/vlcdynamicinstance.h"

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
    vlc_audio_equalizer_new_function vlc_audio_equalizer_new = (vlc_audio_equalizer_new_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("libvlc_audio_equalizer_new");
    _vlcEqualizer = vlc_audio_equalizer_new();
}

VlcEqualizer::~VlcEqualizer()
{
//    if (_vlcEqualizer) {
//        vlc_audio_equalizer_release_function vlc_audio_equalizer_release = (vlc_audio_equalizer_release_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("libvlc_audio_equalizer_release");
//        vlc_audio_equalizer_release(_vlcEqualizer);
//    }
}

float VlcEqualizer::amplificationForBandAt(uint bandIndex) const
{
    if (_vlcEqualizer) {
        vlc_audio_equalizer_get_amp_at_index_function vlc_audio_equalizer_get_amp_at_index = (vlc_audio_equalizer_get_amp_at_index_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("libvlc_audio_equalizer_get_amp_at_index");
        float ret = vlc_audio_equalizer_get_amp_at_index(_vlcEqualizer, bandIndex);
        if (!std::isnan(ret)) {
            return ret;
        }
    }
    return -1.0;
}

//uint VlcEqualizer::bandCount() const
//{
//    if (_vlcEqualizer) {
//        vlc_audio_equalizer_get_band_count_function vlc_audio_equalizer_get_band_count = (vlc_audio_equalizer_get_band_count_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("libvlc_audio_equalizer_get_band_count");
//        return vlc_audio_equalizer_get_band_count();
//    } else {
//        return 0;
//    }
//}

//float VlcEqualizer::bandFrequency(uint bandIndex) const
//{
//    if (_vlcEqualizer) {
//        vlc_audio_equalizer_get_band_frequency_function vlc_audio_equalizer_get_band_frequency = (vlc_audio_equalizer_get_band_frequency_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("libvlc_audio_equalizer_get_band_frequency");
//        return vlc_audio_equalizer_get_band_frequency(bandIndex);
//    } else {
//        return -1.0;
//    }
//}

float VlcEqualizer::preamplification() const
{
    if (_vlcEqualizer) {
        vlc_audio_equalizer_get_preamp_function vlc_audio_equalizer_get_preamp = (vlc_audio_equalizer_get_preamp_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("libvlc_audio_equalizer_get_preamp");
        return vlc_audio_equalizer_get_preamp(_vlcEqualizer);
    } else {
        return 0.0;
    }
}

//uint VlcEqualizer::presetCount() const
//{
//    vlc_audio_equalizer_get_preset_count_function vlc_audio_equalizer_get_preset_count = (vlc_audio_equalizer_get_preset_count_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("libvlc_audio_equalizer_get_preset_count");
//    return vlc_audio_equalizer_get_preset_count();
//}

//QString VlcEqualizer::presetNameAt(uint index) const
//{
//    vlc_audio_equalizer_get_preset_name_function vlc_audio_equalizer_get_preset_name = (vlc_audio_equalizer_get_preset_name_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("libvlc_audio_equalizer_get_preset_name");
//    const char *name = vlc_audio_equalizer_get_preset_name(index);
//    if (name == NULL) {
//        return QString();
//    } else {
//        return QString(name);
//    }
//}

void VlcEqualizer::loadFromPreset(uint index)
{
    if (_vlcEqualizer) {
        vlc_audio_equalizer_release_function vlc_audio_equalizer_release = (vlc_audio_equalizer_release_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("libvlc_audio_equalizer_release");
        vlc_audio_equalizer_release(_vlcEqualizer);
    }
    //18：The custom mode
    if (index < 18) {
        vlc_audio_equalizer_new_from_preset_function vlc_audio_equalizer_new_from_preset = (vlc_audio_equalizer_new_from_preset_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("libvlc_audio_equalizer_new_from_preset");
        _vlcEqualizer = vlc_audio_equalizer_new_from_preset(index);
        if (_vlcEqualizer) {
            emit presetLoaded();
        }
    } else {
        vlc_audio_equalizer_new_function vlc_audio_equalizer_new = (vlc_audio_equalizer_new_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("libvlc_audio_equalizer_new");
        _vlcEqualizer = vlc_audio_equalizer_new();
    }
}

void VlcEqualizer::setAmplificationForBandAt(float amp, uint bandIndex)
{
    if (!_vlcEqualizer) {
        return;
    }
    vlc_audio_equalizer_set_amp_at_index_function vlc_audio_equalizer_set_amp_at_index = (vlc_audio_equalizer_set_amp_at_index_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("libvlc_audio_equalizer_set_amp_at_index");
    vlc_media_player_set_equalizer_function vlc_media_player_set_equalizer = (vlc_media_player_set_equalizer_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("libvlc_media_player_set_equalizer");
    vlc_audio_equalizer_set_amp_at_index(_vlcEqualizer, amp, bandIndex);
    vlc_media_player_set_equalizer(_vlcMediaPlayer->core(), _vlcEqualizer);
}

void VlcEqualizer::setEnabled(bool enabled)
{
    vlc_media_player_set_equalizer_function vlc_media_player_set_equalizer = (vlc_media_player_set_equalizer_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("libvlc_media_player_set_equalizer");
    if (enabled && _vlcEqualizer != nullptr) {
        vlc_media_player_set_equalizer(_vlcMediaPlayer->core(), _vlcEqualizer);
    } else {
        vlc_media_player_set_equalizer(_vlcMediaPlayer->core(), nullptr);
    }
}

void VlcEqualizer::setPreamplification(float value)
{
    if (!_vlcEqualizer) {
        return;
    }
    vlc_media_player_set_equalizer_function vlc_media_player_set_equalizer = (vlc_media_player_set_equalizer_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("libvlc_media_player_set_equalizer");
    vlc_audio_equalizer_set_preamp_function vlc_audio_equalizer_set_preamp = (vlc_audio_equalizer_set_preamp_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("libvlc_audio_equalizer_set_preamp");
    vlc_audio_equalizer_set_preamp(_vlcEqualizer, value);
    vlc_media_player_set_equalizer(_vlcMediaPlayer->core(), _vlcEqualizer);
}
