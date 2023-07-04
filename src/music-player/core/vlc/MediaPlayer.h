// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VLCQT_MEDIAPLAYER_H_
#define VLCQT_MEDIAPLAYER_H_

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtGui/qwindowdefs.h>

#include "Enums.h"
//#include "SharedExportCore.h"

#define LIBVLC_VERSION 0x020200

class VlcAudio;
class VlcEqualizer;
class VlcInstance;
class VlcMedia;
class VlcVideo;
class VlcVideoDelegate;

struct libvlc_event_t;
struct libvlc_event_manager_t;
struct libvlc_media_t;
struct libvlc_media_player_t;


class VLCQT_CORE_EXPORT VlcMediaPlayer : public QObject
{
    Q_OBJECT
public:
    /*!
        \brief VlcMediaPlayer constructor.

        This is mandatory to use libvlc playback functions.

        \param instance instance object (VlcInstance *)
    */
    explicit VlcMediaPlayer(VlcInstance *instance);

    /*!
        VlcMediaPlayer destructor
    */
    ~VlcMediaPlayer();

    /*!
        \brief Returns libvlc media player object.
        \return libvlc media player (libvlc_media_player_t *)
    */
    libvlc_media_player_t *core() const;

#if LIBVLC_VERSION >= 0x020200
    /*!
        \brief Returns equalizer object.
        \return equalizer (VlcEqualizer *)
    */
    VlcEqualizer *equalizer() const;
#endif

    /*!
        \brief Get the current movie length (in ms).
        \return the movie length (in ms), or -1 if there is no media (const int)
    */
    int length() const;

    /*!
        \brief Open media file or stream. Any media should be playable and opened.
        \param media object (VlcMedia *)
    */
    virtual void open(VlcMedia *media);

    /**
     * @brief initCddaTrack 初始化cdda track索引
     */
    void initCddaTrack();

public slots:
    /*! \brief Set the movie time (in ms).

        This has no effect if no media is being played. Not all formats and protocols support this.

        \param time the movie time (in ms) (int)
    */
    virtual void setTime(qint64 time);

    /**
     * Set current software audio volume.
     *
     * \param i_volume the volume in percents (0 = mute, 100 = 0dB)
     * \return 0 if the volume was set, -1 if it was out of range
     */
    virtual void setVolume(int volume);

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
    virtual void setMute(bool mute);

public:
    /*!
        \brief Get the current movie time (in ms).
        \return the movie time (in ms), or -1 if there is no media (const int)
    */
    int time() const;

    /*!
        \brief Get current player state.
        \return current player state (const Vlc::State)
    */
    Vlc::State state() const;

    /*!
        \brief Get media content position.
        \return current media position (float)
    */
    float position();

    /**
     * Get current software audio volume.
     *
     * \return the software volume in percents
     * (0 = mute, 100 = nominal / 0dB)
     */
    virtual int getVolume();

    /**
     * Get current mute status.
     *
     * \return the mute status (boolean) if defined, -1 if undefined/unapplicable
     */
    virtual bool getMute();

public slots:
    /*! \brief Set the media position.

        This has no effect if no media is being played. Not all formats and protocols support this.

        \param pos the media position (float)
    */
    void setPosition(float pos);

    /*!
        \brief Starts playing current media if possible
    */
    virtual void play();

    /*!
        \brief Pauses the playback of current media if possible
    */
    virtual void pause();

    /*!
        \brief Resumes the playback of current media if possible
    */
    virtual void resume();

    /*!
        \brief Stops playing current media
    */
    virtual void stop();

signals:
    /*!
        \brief Signal sent on backward
    */
    void backward();

    /*!
        \brief Signal sent on buffering
        \param buffer buffer status in percent
    */
    void buffering(float buffer);

    /*!
        \brief Signal sent on buffering
        \param buffer buffer status in percent
    */
    void buffering(int buffer);

    /*!
        \brief Signal sent when end reached
    */
    void end();

    /*!
        \brief Signal sent on error
    */
    void error();

    /*!
        \brief Signal sent on forward
    */
    void forward();

    /*!
        \brief Signal sent on length change
        \param length new length
    */
    void lengthChanged(int length);

    /*!
        \brief Signal sent on media change
        \param media new media object
    */
    void mediaChanged(libvlc_media_t *media);

    /*!
        \brief Signal sent nothing speciall happened
    */
    void nothingSpecial();

    /*!
        \brief Signal sent when opening
    */
    void opening();

    /*!
        \brief Signal sent on pausable change
        \param pausable pausable status
    */
    void pausableChanged(bool pausable);

    /*!
        \brief Signal sent when paused
    */
    void paused();

    /*!
        \brief Signal sent when playing
    */
    void playing();

    /*!
        \brief Signal sent on position change
        \param position new position
    */
    void positionChanged(float position);

    /*!
        \brief Signal sent on seekable change
        \param seekable seekable status
    */
    void seekableChanged(bool seekable);

    /*!
        \brief Signal sent on snapshot taken
        \param filename filename of the snapshot
    */
    void snapshotTaken(const QString &filename);

    /*!
        \brief Signal sent when stopped
    */
    void stopped();

    /*!
        \brief Signal sent on time change
        \param time new time
    */
    void timeChanged(qint64 time);

    /*!
        \brief Signal sent on title change
        \param title new title
    */
    void titleChanged(int title);

    /*!
        \brief Signal sent when video output is available
        \param count number of video outputs available
    */
    void vout(int count);

    /*!
        \brief Signal sent when state of the player changed
    */
    void stateChanged();

    void endReached();

protected:
    static void libvlc_callback(const libvlc_event_t *event,
                                void *data);

    void createCoreConnections();
    void removeCoreConnections();

    libvlc_media_player_t *_vlcMediaPlayer;
    libvlc_event_manager_t *_vlcEvents;

    VlcEqualizer *_vlcEqualizer;

    QByteArray _data;
};

#endif // VLCQT_MEDIAPLAYER_H_
