// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QtCore/QDebug>
#include <QtCore/QDir>

#include <vlc/vlc.h>

#include "Error.h"
#include "Instance.h"
#include "Media.h"
#include "util/log.h"

#include "dynamiclibraries.h"

typedef libvlc_media_t *(*vlc_media_duplicate_function)(libvlc_media_t *);
typedef void (*vlc_media_release_function)(libvlc_media_t *);
typedef libvlc_media_t *(*vlc_media_new_path_function)(libvlc_instance_t *, const char *);
typedef libvlc_media_t *(*vlc_media_new_location_function)(libvlc_instance_t *, const char *);
typedef libvlc_event_manager_t *(*vlc_media_event_manager_function)(libvlc_media_t *);
typedef int (*vlc_event_attach_function)(libvlc_event_manager_t *,
                                         libvlc_event_type_t,
                                         libvlc_callback_t,
                                         void *);
typedef int (*vlc_event_detach_function)(libvlc_event_manager_t *,
                                         libvlc_event_type_t,
                                         libvlc_callback_t,
                                         void *);
typedef int (*vlc_media_is_parsed_function)(libvlc_media_t *);
typedef void (*vlc_media_parse_async_function)(libvlc_media_t *);
typedef libvlc_state_t (*vlc_media_get_state_function)(libvlc_media_t *);
typedef libvlc_time_t (*vlc_media_get_duration_function)(libvlc_media_t *);
typedef void (*vlc_media_add_option_function)(libvlc_media_t *, const char *);

VlcMedia::VlcMedia()
{
    qCDebug(dmMusic) << "Creating new VLC media instance";
    _vlcMedia = nullptr;
    _vlcEvents = nullptr;
}

VlcMedia::~VlcMedia()
{
    qCDebug(dmMusic) << "Destroying VLC media instance";
    releaseMedia();
}

void VlcMedia::releaseMedia()
{
    qCDebug(dmMusic) << "Releasing VLC media";
    //释放media
    if (_vlcMedia) {
        removeCoreConnections();
        vlc_media_release_function vlc_media_release = (vlc_media_release_function)DynamicLibraries::instance()->resolve("libvlc_media_release");
        vlc_media_release(_vlcMedia);
        _vlcMedia = nullptr;
        qCDebug(dmMusic) << "VLC media released successfully";
    }
}

libvlc_media_t *VlcMedia::core()
{
    return _vlcMedia;
}

void VlcMedia::initMedia(const QString &location,
                         bool localFile,
                         VlcInstance *instance, int track)
{
    qCDebug(dmMusic) << "Initializing media with location:" << location << "localFile:" << localFile << "track:" << track;
    _currentLocation = location;
    m_cdaTrackId = track;
    QString path = location;
    if (localFile)
        path = QDir::toNativeSeparators(path);

    releaseMedia();

    // Create a new libvlc media descriptor from location
    vlc_media_new_path_function vlc_media_new_path = (vlc_media_new_path_function)DynamicLibraries::instance()->resolve("libvlc_media_new_path");
    vlc_media_new_location_function vlc_media_new_location = (vlc_media_new_location_function)DynamicLibraries::instance()->resolve("libvlc_media_new_location");
    vlc_media_event_manager_function vlc_media_event_manager = (vlc_media_event_manager_function)DynamicLibraries::instance()->resolve("libvlc_media_event_manager");

    if (localFile) {
        qCDebug(dmMusic) << "Creating media from local path:" << path;
        _vlcMedia = vlc_media_new_path(instance->core(), path.toUtf8().data());
    } else {
        qCDebug(dmMusic) << "Creating media from location:" << path;
        _vlcMedia = vlc_media_new_location(instance->core(), path.toUtf8().data());
    }
    
    if (!_vlcMedia) {
        qCWarning(dmMusic) << "Failed to create VLC media for path:" << path;
        return;
    }
    
    _vlcEvents = vlc_media_event_manager(_vlcMedia);
    if (!_vlcEvents) {
        qCWarning(dmMusic) << "Failed to get media event manager";
        return;
    }

    createCoreConnections();
    qCDebug(dmMusic) << "Media initialization completed successfully";

    VlcError::showErrmsg();
}

int VlcMedia::getCdaTrack() const
{
    return m_cdaTrackId;
}

void VlcMedia::createCoreConnections()
{
    qCDebug(dmMusic) << "Creating core media connections";
    QList<libvlc_event_e> list;
    list << libvlc_MediaMetaChanged
         << libvlc_MediaSubItemAdded
         << libvlc_MediaDurationChanged
         << libvlc_MediaParsedChanged
         << libvlc_MediaFreed
         << libvlc_MediaStateChanged;

    vlc_event_attach_function vlc_event_attach = (vlc_event_attach_function)DynamicLibraries::instance()->resolve("libvlc_event_attach");
    foreach (const libvlc_event_e &event, list) {
        vlc_event_attach(_vlcEvents, event, libvlc_callback, this);
    }
    qCDebug(dmMusic) << "Core media connections created successfully";
}

void VlcMedia::removeCoreConnections()
{
    qCDebug(dmMusic) << "Removing core media connections";
    QList<libvlc_event_e> list;
    list << libvlc_MediaMetaChanged
         << libvlc_MediaSubItemAdded
         << libvlc_MediaDurationChanged
         << libvlc_MediaParsedChanged
         << libvlc_MediaFreed
         << libvlc_MediaStateChanged;

    vlc_event_detach_function vlc_event_detach = (vlc_event_detach_function)DynamicLibraries::instance()->resolve("libvlc_event_detach");
    foreach (const libvlc_event_e &event, list) {
        vlc_event_detach(_vlcEvents, event, libvlc_callback, this);
    }
    qCDebug(dmMusic) << "Core media connections removed successfully";
}

Vlc::State VlcMedia::state() const
{
    qCDebug(dmMusic) << "Getting media state";
    libvlc_state_t state;
    vlc_media_get_state_function vlc_media_get_state = (vlc_media_get_state_function)DynamicLibraries::instance()->resolve("libvlc_media_get_state");
    state = vlc_media_get_state(_vlcMedia);

    VlcError::showErrmsg();
    qCDebug(dmMusic) << "Current media state:" << state;
    return Vlc::State(state);
}

void VlcMedia::libvlc_callback(const libvlc_event_t *event,
                               void *data)
{
    VlcMedia *core = static_cast<VlcMedia *>(data);

    switch (event->type) {
    case libvlc_MediaMetaChanged:
        qCDebug(dmMusic) << "Media meta changed event - type:" << event->u.media_meta_changed.meta_type;
        emit core->metaChanged(Vlc::Meta(event->u.media_meta_changed.meta_type));
        break;
    case libvlc_MediaSubItemAdded:
        qCDebug(dmMusic) << "Media subitem added event";
        emit core->subitemAdded(event->u.media_subitem_added.new_child);
        break;
    case libvlc_MediaDurationChanged:
        qCDebug(dmMusic) << "Media duration changed event - new duration:" << event->u.media_duration_changed.new_duration;
        emit core->durationChanged(static_cast<int>(event->u.media_duration_changed.new_duration));
        break;
    case libvlc_MediaParsedChanged:
        qCDebug(dmMusic) << "Media parsed changed event - new status:" << event->u.media_parsed_changed.new_status;
        emit core->parsedChanged(static_cast<bool>(event->u.media_parsed_changed.new_status));
        break;
    case libvlc_MediaFreed:
        qCDebug(dmMusic) << "Media freed event";
        emit core->freed(event->u.media_freed.md);
        break;
    case libvlc_MediaStateChanged:
        qCDebug(dmMusic) << "Media state changed event - new state:" << event->u.media_state_changed.new_state;
        emit core->stateChanged(Vlc::State(event->u.media_state_changed.new_state));
        break;
    default:
        qCDebug(dmMusic) << "Unhandled media event type:" << event->type;
        break;
    }
}
