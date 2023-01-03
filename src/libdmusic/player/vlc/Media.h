// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VLCQT_MEDIA_H_
#define VLCQT_MEDIA_H_

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QUrl>

#include "Enums.h"
//#include "SharedExportCore.h"

class VlcInstance;
struct VlcStats;

struct libvlc_event_t;
struct libvlc_event_manager_t;
struct libvlc_media_t;

/*!

    An abstract representation of a playable media.
    It consists of a media location and various optional meta data.
*/
class VLCQT_CORE_EXPORT VlcMedia : public QObject
{
    Q_OBJECT
public:
    /*!
        \brief VlcMedia constructor.

        This constructor creates a new media instance from a media location.
    */
    VlcMedia();

    /*!
        \brief VlcMedia destructor
    */
    ~VlcMedia();

    /*!
        \brief release Media
    */
    void releaseMedia();

    /*!
        \brief libvlc media item
        \return libvlc media item (libvlc_media_t *)
    */
    libvlc_media_t *core();

    /*!
        \brief Get media state
        \return current media state
    */
    Vlc::State state() const;

    void initMedia(const QString &location,
                   bool localFile,
                   VlcInstance *instance, int track = -1);

    /**
     * @brief getCdaTrack 获取cd track id
     * @return
     */
    int getCdaTrack() const;

signals:
    /*!
        \brief Signal sent on meta change
        \param meta new meta
    */
    void metaChanged(const Vlc::Meta &meta);

    /*!
        \brief Signal sent on subitem added
        \param subitem subitem that was added
    */
    void subitemAdded(libvlc_media_t *subitem);

    /*!
        \brief Signal sent on duration change
        \param duration new duration
    */
    void durationChanged(int duration);

    /*!
        \brief Signal sent on parsed change
        \param status new parsed status
    */
    Q_DECL_DEPRECATED void parsedChanged(int status);

    /*!
        \brief Signal sent on parsed change
        \param status new parsed status
    */
    void parsedChanged(bool status);

    /*!
        \brief Signal sent on freed
        \param media freed libvlc_media_t object
    */
    void freed(libvlc_media_t *media);

    /*!
        \brief Signal sent on state change
        \param state new state
    */
    void stateChanged(const Vlc::State &state);

private:
    static void libvlc_callback(const libvlc_event_t *event,
                                void *data);

    void createCoreConnections();
    void removeCoreConnections();

    libvlc_media_t *_vlcMedia;
    libvlc_event_manager_t *_vlcEvents;

    QString _currentLocation;
    int m_cdaTrackId = -1;
};

#endif // VLCQT_MEDIA_H_
