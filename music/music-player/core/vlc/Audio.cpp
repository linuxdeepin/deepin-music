

#include <vlc/vlc.h>
#include <vlc_common.h>
#include <vlc_variables.h>
#include <QDebug>

#include "Audio.h"
#include "Error.h"
#include "MediaPlayer.h"
#include "core/vlc/vlcdynamicinstance.h"

/*!
    \private
*/
class VlcAudioCallbackHelper
{
public:
    static int volumeCallback(vlc_object_t *obj,
                              const char *name,
                              vlc_value_t oldVal,
                              vlc_value_t newVal,
                              void *data)
    {
        Q_UNUSED(obj)
        Q_UNUSED(name)
        Q_UNUSED(oldVal)

        VlcAudio *core = static_cast<VlcAudio *>(data);
        emit core->volumeChangedF(newVal.f_float);
        int vol = newVal.f_float < 0 ? -1 : qRound(newVal.f_float * 100.f);
        emit core->volumeChanged(vol);
        return VLC_SUCCESS;
    }

    static int muteCallback(vlc_object_t *obj,
                            const char *name,
                            vlc_value_t oldVal,
                            vlc_value_t newVal,
                            void *data)
    {
        Q_UNUSED(obj);
        Q_UNUSED(name);
        Q_UNUSED(oldVal);

        VlcAudio *core = static_cast<VlcAudio *>(data);
        if (oldVal.b_bool != newVal.b_bool)
            emit core->muteChanged(newVal.b_bool);
        return VLC_SUCCESS;
    }
};

typedef int (*vlc_get_mute_function)(libvlc_media_player_t *);
typedef int (*vlc_audio_set_volume_function)(libvlc_media_player_t *, int);
typedef int (*vlc_audio_set_track_function)(libvlc_media_player_t *, int);
typedef void (*vlc_audio_toggle_mute_function)(libvlc_media_player_t *);
typedef void (*vlc_audio_set_mute_function)(libvlc_media_player_t *, int);
typedef int (*vlc_audio_get_track_function)(libvlc_media_player_t *);
typedef int (*vlc_audio_get_track_count_function)(libvlc_media_player_t *);
typedef libvlc_track_description_t *(*vlc_audio_get_track_description_function)(libvlc_media_player_t *);
typedef void (*vlc_track_description_list_release_function)(libvlc_track_description_t *);
typedef int (*vlc_audio_get_volume_function)(libvlc_media_player_t *);
typedef int (*vlc_audio_get_channel_function)(libvlc_media_player_t *);
typedef int (*vlc_audio_set_channel_function)(libvlc_media_player_t *, int);

typedef void (*var_AddCallback_function)(vlc_object_t *, const char *, vlc_callback_t, void *);
typedef void (*var_DelCallback_function)(vlc_object_t *, const char *, vlc_callback_t, void *);

VlcAudio::VlcAudio(VlcMediaPlayer *player)
    : QObject(player),
      _vlcMediaPlayer(player->core())
{
    var_AddCallback_function AddCallback = (var_AddCallback_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("var_AddCallback");
    AddCallback((vlc_object_t *)_vlcMediaPlayer, "volume", VlcAudioCallbackHelper::volumeCallback, this);
    AddCallback((vlc_object_t *)_vlcMediaPlayer, "mute", VlcAudioCallbackHelper::muteCallback, this);
    getMute();
}

VlcAudio::~VlcAudio()
{
    var_DelCallback_function DelCallback = (var_DelCallback_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("var_DelCallback");
    DelCallback((vlc_object_t *)_vlcMediaPlayer, "volume", VlcAudioCallbackHelper::volumeCallback, this);
    DelCallback((vlc_object_t *)_vlcMediaPlayer, "mute", VlcAudioCallbackHelper::muteCallback, this);
}

bool VlcAudio::getMute() const
{
    bool mute = false;
    if (_vlcMediaPlayer) {
        vlc_get_mute_function vlc_get_mute = (vlc_get_mute_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("libvlc_audio_get_mute");
        mute = vlc_get_mute(_vlcMediaPlayer);
        VlcError::showErrmsg();
    }

    return mute;
}

void VlcAudio::setVolume(int volume)
{
    if (_vlcMediaPlayer) {
        // Don't change if volume is the same
        if (volume != VlcAudio::volume()) {
            vlc_audio_set_volume_function vlc_get_mute = (vlc_audio_set_volume_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("libvlc_audio_set_volume");
            vlc_get_mute(_vlcMediaPlayer, volume);
            VlcError::showErrmsg();
        }
    }
}

void VlcAudio::setTrack(int track)
{
    if (_vlcMediaPlayer) {
        vlc_audio_set_track_function vlc_audio_set_track = (vlc_audio_set_track_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("libvlc_audio_set_track");
        vlc_audio_set_track(_vlcMediaPlayer, track);
        VlcError::showErrmsg();
    }
}

bool VlcAudio::toggleMute() const
{
    if (_vlcMediaPlayer) {
        vlc_audio_toggle_mute_function vlc_audio_toggle_mute = (vlc_audio_toggle_mute_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("libvlc_audio_toggle_mute");
        vlc_audio_toggle_mute(_vlcMediaPlayer);
        VlcError::showErrmsg();
    }

    return getMute();
}

void VlcAudio::setMute(bool mute) const
{
    if (_vlcMediaPlayer && mute != getMute()) {
        vlc_audio_set_mute_function vlc_audio_set_mute = (vlc_audio_set_mute_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("libvlc_audio_set_mute");
        vlc_audio_set_mute(_vlcMediaPlayer, mute);
        VlcError::showErrmsg();
    }
}

int VlcAudio::track() const
{
    int track = -1;
    if (_vlcMediaPlayer) {
        vlc_audio_get_track_function vlc_audio_get_track = (vlc_audio_get_track_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("libvlc_audio_get_track");
        track = vlc_audio_get_track(_vlcMediaPlayer);
        VlcError::showErrmsg();
    }

    return track;
}

int VlcAudio::trackCount() const
{
    int count = -1;
    if (_vlcMediaPlayer) {
        vlc_audio_get_track_count_function vlc_audio_get_track_count = (vlc_audio_get_track_count_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("libvlc_audio_get_track_count");
        count = vlc_audio_get_track_count(_vlcMediaPlayer);
        VlcError::showErrmsg();
    }

    return count;
}

QStringList VlcAudio::trackDescription() const
{
    QStringList descriptions;

    if (_vlcMediaPlayer) {
        libvlc_track_description_t *desc;
        vlc_audio_get_track_description_function vlc_audio_get_track_description = (vlc_audio_get_track_description_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("libvlc_audio_get_track_description");
        desc = vlc_audio_get_track_description(_vlcMediaPlayer);
        VlcError::showErrmsg();

        descriptions << QString().fromUtf8(desc->psz_name);
        if (trackCount() > 1) {
            for (int i = 1; i < trackCount(); i++) {
                desc = desc->p_next;
                descriptions << QString().fromUtf8(desc->psz_name);
            }
        }
    }

    return descriptions;
}

QList<int> VlcAudio::trackIds() const
{
    QList<int> ids;

    if (_vlcMediaPlayer) {
        libvlc_track_description_t *desc;
        vlc_audio_get_track_description_function vlc_audio_get_track_description = (vlc_audio_get_track_description_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("libvlc_audio_get_track_description");
        desc = vlc_audio_get_track_description(_vlcMediaPlayer);
        VlcError::showErrmsg();

        ids << desc->i_id;
        if (trackCount() > 1) {
            for (int i = 1; i < trackCount(); i++) {
                desc = desc->p_next;
                ids << desc->i_id;
            }
        }
    }

    return ids;
}

QMap<int, QString> VlcAudio::tracks() const
{
    QMap<int, QString> tracks;

    if (_vlcMediaPlayer) {
        libvlc_track_description_t *desc, *first;
        vlc_audio_get_track_description_function vlc_audio_get_track_description = (vlc_audio_get_track_description_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("libvlc_audio_get_track_description");
        first = desc = vlc_audio_get_track_description(_vlcMediaPlayer);
        VlcError::showErrmsg();

        if (desc != nullptr) {
            tracks.insert(desc->i_id, QString().fromUtf8(desc->psz_name));
            if (trackCount() > 1) {
                for (int i = 1; i < trackCount(); i++) {
                    desc = desc->p_next;
                    tracks.insert(desc->i_id, QString().fromUtf8(desc->psz_name));
                }
            }
            vlc_track_description_list_release_function vlc_track_description_list_release = (vlc_track_description_list_release_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("libvlc_track_description_list_release");
            vlc_track_description_list_release(first);
        }
    }

    return tracks;
}

int VlcAudio::volume() const
{
    int volume = -1;
    if (_vlcMediaPlayer) {
        vlc_audio_get_volume_function  vlc_audio_get_volume = (vlc_audio_get_volume_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("libvlc_audio_get_volume");
        volume = vlc_audio_get_volume(_vlcMediaPlayer);
        VlcError::showErrmsg();
    }

    return volume;
}

Vlc::AudioChannel VlcAudio::channel() const
{
    Vlc::AudioChannel channel = Vlc::AudioChannelError;
    if (_vlcMediaPlayer) {
        vlc_audio_get_channel_function  vlc_audio_get_channel = (vlc_audio_get_channel_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("libvlc_audio_get_volume");
        channel = Vlc::AudioChannel(vlc_audio_get_channel(_vlcMediaPlayer));
        VlcError::showErrmsg();
    }

    return channel;
}

void VlcAudio::setChannel(Vlc::AudioChannel channel)
{
    if (_vlcMediaPlayer) {
        // Don't change if channel is the same
        if (channel != VlcAudio::channel()) {
            vlc_audio_set_channel_function vlc_audio_set_channel = (vlc_audio_set_channel_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("libvlc_audio_set_channel");
            vlc_audio_set_channel(_vlcMediaPlayer, channel);
            VlcError::showErrmsg();
        }
    }
}
