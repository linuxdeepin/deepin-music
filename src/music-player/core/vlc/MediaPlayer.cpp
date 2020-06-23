

#include <vlc/vlc.h>

#include "Audio.h"
#include "Error.h"
#include "Instance.h"
#include "Media.h"
#include "MediaPlayer.h"
#include <QDebug>


#include "Equalizer.h"


VlcMediaPlayer::VlcMediaPlayer(VlcInstance *instance)
    : QObject(instance)
{
    _vlcMediaPlayer = libvlc_media_player_new(instance->core());
    _vlcEvents = libvlc_media_player_event_manager(_vlcMediaPlayer);

    VlcError::showErrmsg();

    _vlcAudio = new VlcAudio(this);
    _vlcEqualizer = new VlcEqualizer(this);

    _media = 0;

//    connect(&timer, SIGNAL(timeout()), this, SLOT(timeoutSlot()));
//    timer.start(1000);

    createCoreConnections();

    VlcError::showErrmsg();
}

VlcMediaPlayer::~VlcMediaPlayer()
{
    removeCoreConnections();

    delete _vlcAudio;

    libvlc_media_player_release(_vlcMediaPlayer);

    VlcError::showErrmsg();
}

libvlc_media_player_t *VlcMediaPlayer::core() const
{
    return _vlcMediaPlayer;
}

VlcAudio *VlcMediaPlayer::audio() const
{
    return _vlcAudio;
}

//VlcVideo *VlcMediaPlayer::video() const
//{
////    return _vlcVideo;
//}

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

    foreach (const libvlc_event_e &event, list) {
        libvlc_event_attach(_vlcEvents, event, libvlc_callback, this);
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

    foreach (const libvlc_event_e &event, list) {
        libvlc_event_detach(_vlcEvents, event, libvlc_callback, this);
    }
}

bool VlcMediaPlayer::hasVout() const
{
    bool status = false;
    if (_vlcMediaPlayer) {
        status = libvlc_media_player_has_vout(_vlcMediaPlayer);
    }

    return status;
}

int VlcMediaPlayer::length() const
{
    libvlc_time_t length = libvlc_media_player_get_length(_vlcMediaPlayer);

    VlcError::showErrmsg();

    return length;
}

VlcMedia *VlcMediaPlayer::currentMedia() const
{
    return _media;
}

libvlc_media_t *VlcMediaPlayer::currentMediaCore()
{
    libvlc_media_t *media = libvlc_media_player_get_media(_vlcMediaPlayer);

    VlcError::showErrmsg();

    return media;
}

void VlcMediaPlayer::open(VlcMedia *media)
{
    _media = media;
    libvlc_media_player_set_media(_vlcMediaPlayer, media->core());

    VlcError::showErrmsg();
}

void VlcMediaPlayer::openOnly(VlcMedia *media)
{
    _media = media;
    libvlc_media_player_set_media(_vlcMediaPlayer, media->core());

    VlcError::showErrmsg();
}

void VlcMediaPlayer::play()
{
    if (!_vlcMediaPlayer)
        return;

    libvlc_media_player_play(_vlcMediaPlayer);

    VlcError::showErrmsg();
}

void VlcMediaPlayer::pause()
{
    if (!_vlcMediaPlayer)
        return;

    if (libvlc_media_player_can_pause(_vlcMediaPlayer))
        libvlc_media_player_set_pause(_vlcMediaPlayer, true);

    VlcError::showErrmsg();
}

void VlcMediaPlayer::togglePause()
{
    if (!_vlcMediaPlayer)
        return;

    if (libvlc_media_player_can_pause(_vlcMediaPlayer))
        libvlc_media_player_pause(_vlcMediaPlayer);

    VlcError::showErrmsg();
}

void VlcMediaPlayer::resume()
{
    if (!_vlcMediaPlayer)
        return;

    if (libvlc_media_player_can_pause(_vlcMediaPlayer))
        libvlc_media_player_set_pause(_vlcMediaPlayer, false);

    VlcError::showErrmsg();
}

void VlcMediaPlayer::setTime(qint64 time)
{
    /*****************************************
     *  add enum Opening to set progress value
     * ***************************************/
#ifdef QT_NO_DEBUG
    if (!(state() == Vlc::Buffering
            || state() == Vlc::Playing
            || state() == Vlc::Paused
            ||  state() == Vlc::Opening))
        return;
#else
    if (!(state() == Vlc::Buffering
            || state() == Vlc::Playing
            || state() == Vlc::Paused
            || state() == Vlc::Opening)) //because debug modle may be Opening state
        return;
#endif

    libvlc_media_player_set_time(_vlcMediaPlayer, time);

    if (state() == Vlc::Paused)
        emit timeChanged(time);

    VlcError::showErrmsg();
}

//void VlcMediaPlayer::setVideoWidget(VlcVideoDelegate *widget)
//{
//    _videoWidget = widget;
//}

bool VlcMediaPlayer::seekable() const
{
    if (!libvlc_media_player_get_media(_vlcMediaPlayer))
        return false;

    bool seekable = libvlc_media_player_is_seekable(_vlcMediaPlayer);

    VlcError::showErrmsg();

    return seekable;
}

Vlc::State VlcMediaPlayer::state() const
{
    // It's possible that the vlc doesn't play anything
    // so check before
    if (!libvlc_media_player_get_media(_vlcMediaPlayer))
        return Vlc::Idle;

    libvlc_state_t state;
    state = libvlc_media_player_get_state(_vlcMediaPlayer);

//    VlcError::showErrmsg();

    return Vlc::State(state);
}
//void VlcMediaPlayer::timeoutSlot()
//{
//    if (!_vlcMediaPlayer)
//        return;

//    emit timeChanged(libvlc_media_player_get_time(_vlcMediaPlayer));
//}

void VlcMediaPlayer::stop()
{
    if (!_vlcMediaPlayer)
        return;

    libvlc_media_player_stop(_vlcMediaPlayer);

    VlcError::showErrmsg();
}

int VlcMediaPlayer::time() const
{
    libvlc_time_t time = libvlc_media_player_get_time(_vlcMediaPlayer);

    VlcError::showErrmsg();

    return time;
}

//VlcVideoDelegate *VlcMediaPlayer::videoWidget() const
//{
//    return _videoWidget;
//}

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
        emit core->end();
        break;
    case libvlc_MediaPlayerEncounteredError:
        emit core->error();
        break;
    case libvlc_MediaPlayerTimeChanged:
        emit core->timeChanged(event->u.media_player_time_changed.new_time);
        break;
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

    return libvlc_media_player_get_position(_vlcMediaPlayer);
}

float VlcMediaPlayer::sampleAspectRatio()
{
    if (!_vlcMediaPlayer)
        return 0.0;
#if LIBVLC_VERSION >= 0x020100
    float sar = 0.0;

    libvlc_media_track_t **tracks;
    unsigned tracksCount;
    tracksCount = libvlc_media_tracks_get(_media->core(), &tracks);
    if (tracksCount > 0) {
        for (unsigned i = 0; i < tracksCount; i++) {
            libvlc_media_track_t *track = tracks[i];
            if (track->i_type == libvlc_track_video && track->i_id == 0) {
                libvlc_video_track_t *videoTrack = track->video;
                if (videoTrack->i_sar_num > 0)
                    sar = (float)videoTrack->i_sar_den / (float)videoTrack->i_sar_num;
            }
        }
        libvlc_media_tracks_release(tracks, tracksCount);
    }

    return sar;
#else
    return 1.0;
#endif // LIBVLC_VERSION >= 0x020100
}

void VlcMediaPlayer::setPosition(float pos)
{
    libvlc_media_player_set_position(_vlcMediaPlayer, pos);

    VlcError::showErrmsg();
}

void VlcMediaPlayer::setPlaybackRate(float rate)
{
    libvlc_media_player_set_rate(_vlcMediaPlayer, rate);

    VlcError::showErrmsg();
}

float VlcMediaPlayer::playbackRate()
{
    if (!_vlcMediaPlayer)
        return -1;

    return libvlc_media_player_get_rate(_vlcMediaPlayer);
}
