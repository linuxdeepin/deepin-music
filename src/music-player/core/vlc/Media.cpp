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

#include <QtCore/QDebug>
#include <QtCore/QDir>

#include <vlc/vlc.h>

#include "Error.h"
#include "Instance.h"
#include "Media.h"

#include "core/vlc/vlcdynamicinstance.h"

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

//VlcMedia::VlcMedia(const QString &location,
//                   bool localFile,
//                   VlcInstance *instance)
//    : QObject(instance)
//{
//    initMedia(location, localFile, instance);
//}

//VlcMedia::VlcMedia(const QString &location,
//                   VlcInstance *instance)
//    : QObject(instance)
//{
//    initMedia(location, false, instance);
//}

//VlcMedia::VlcMedia(libvlc_media_t *media):_vlcEvents(nullptr)
//{
//    // Create a new libvlc media descriptor from existing one
//    vlc_media_duplicate_function vlc_media_duplicate = (vlc_media_duplicate_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("libvlc_media_duplicate");
//    _vlcMedia = vlc_media_duplicate(media);

//    VlcError::showErrmsg();
//}

VlcMedia::VlcMedia()
{
    _vlcMedia = nullptr;
    _vlcEvents = nullptr;
}

VlcMedia::~VlcMedia()
{
//    removeCoreConnections();
//    vlc_media_release_function vlc_media_release = (vlc_media_release_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("libvlc_media_release");
//    vlc_media_release(_vlcMedia);
}

libvlc_media_t *VlcMedia::core()
{
    return _vlcMedia;
}

void VlcMedia::initMedia(const QString &location,
                         bool localFile,
                         VlcInstance *instance, int track)
{
    _currentLocation = location;
    m_cdaTrackId = track;
    QString path = location;
    if (localFile)
        path = QDir::toNativeSeparators(path);

    // Create a new libvlc media descriptor from location
    vlc_media_new_path_function vlc_media_new_path = (vlc_media_new_path_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("libvlc_media_new_path");
    vlc_media_new_location_function vlc_media_new_location = (vlc_media_new_location_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("libvlc_media_new_location");
    vlc_media_event_manager_function vlc_media_event_manager = (vlc_media_event_manager_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("libvlc_media_event_manager");

    if (localFile)
        _vlcMedia = vlc_media_new_path(instance->core(), path.toUtf8().data());
    else {
        _vlcMedia = vlc_media_new_location(instance->core(), path.toUtf8().data());
    }
    _vlcEvents = vlc_media_event_manager(_vlcMedia);

    createCoreConnections();

    VlcError::showErrmsg();
}

int VlcMedia::getCdaTrack() const
{
    return m_cdaTrackId;
}

void VlcMedia::createCoreConnections()
{
    QList<libvlc_event_e> list;
    list << libvlc_MediaMetaChanged
         << libvlc_MediaSubItemAdded
         << libvlc_MediaDurationChanged
         << libvlc_MediaParsedChanged
         << libvlc_MediaFreed
         << libvlc_MediaStateChanged;

    vlc_event_attach_function vlc_event_attach = (vlc_event_attach_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("libvlc_event_attach");
    foreach (const libvlc_event_e &event, list) {
        vlc_event_attach(_vlcEvents, event, libvlc_callback, this);
    }
}

//void VlcMedia::removeCoreConnections()
//{
//    QList<libvlc_event_e> list;
//    list << libvlc_MediaMetaChanged
//         << libvlc_MediaSubItemAdded
//         << libvlc_MediaDurationChanged
//         << libvlc_MediaParsedChanged
//         << libvlc_MediaFreed
//         << libvlc_MediaStateChanged;

//    vlc_event_detach_function vlc_event_detach = (vlc_event_detach_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("libvlc_event_detach");
//    foreach (const libvlc_event_e &event, list) {
//        vlc_event_detach(_vlcEvents, event, libvlc_callback, this);
//    }
//}

//bool VlcMedia::parsed() const
//{
//    vlc_media_is_parsed_function vlc_media_is_parsed = (vlc_media_is_parsed_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("libvlc_media_is_parsed");
//    int parsed = vlc_media_is_parsed(_vlcMedia);

//    VlcError::showErrmsg();

//    return parsed;
//}

//void VlcMedia::parse()
//{
//    vlc_media_parse_async_function  vlc_media_parse_async = (vlc_media_parse_async_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("libvlc_media_parse_async");
//    vlc_media_parse_async(_vlcMedia);

//    VlcError::showErrmsg();
//}

//QString VlcMedia::currentLocation() const
//{
//    return _currentLocation;
//}

Vlc::State VlcMedia::state() const
{
    libvlc_state_t state;
    vlc_media_get_state_function vlc_media_get_state = (vlc_media_get_state_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("libvlc_media_get_state");
    state = vlc_media_get_state(_vlcMedia);

    VlcError::showErrmsg();

    return Vlc::State(state);
}

//qint64 VlcMedia::duration() const
//{
//    vlc_media_get_duration_function vlc_media_get_duration = (vlc_media_get_duration_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("libvlc_media_get_duration");
//    libvlc_time_t duration = vlc_media_get_duration(_vlcMedia);

//    VlcError::showErrmsg();

//    return duration;
//}

//QString VlcMedia::duplicate(const QString &name,
//                            const QString &path,
//                            const Vlc::Mux &mux)
//{
//    return record(name, path, mux, true);
//}

//QString VlcMedia::duplicate(const QString &name,
//                            const QString &path,
//                            const Vlc::Mux &mux,
//                            const Vlc::AudioCodec &audioCodec,
//                            const Vlc::VideoCodec &videoCodec)
//{
//    return record(name, path, mux, audioCodec, videoCodec, true);
//}

//QString VlcMedia::duplicate(const QString &name,
//                            const QString &path,
//                            const Vlc::Mux &mux,
//                            const Vlc::AudioCodec &audioCodec,
//                            const Vlc::VideoCodec &videoCodec,
//                            int bitrate,
//                            int fps,
//                            int scale)
//{
//    return record(name, path, mux, audioCodec, videoCodec, bitrate, fps, scale, true);
//}

//QString VlcMedia::merge(const QString &name,
//                        const QString &path,
//                        const Vlc::Mux &mux)
//{
//    QString option1, option2, parameters;
//    QString l = QDir::toNativeSeparators(path + "/" + name);

//    parameters = "gather:std{access=file,mux=%1,dst='%2'}";
//    parameters = parameters.arg(Vlc::mux()[mux], l + "." + Vlc::mux()[mux]);

//    option1 = ":sout-keep";
//    option2 = ":sout=#%1";
//    option2 = option2.arg(parameters);

//    setOption(option1);
//    setOption(option2);

//    VlcError::showErrmsg();

//    return l + "." + Vlc::mux()[mux];
//}

//QString VlcMedia::record(const QString &name,
//                         const QString &path,
//                         const Vlc::Mux &mux,
//                         bool duplicate)
//{
//    QString option1, option2, parameters;
//    QString l = QDir::toNativeSeparators(path + "/" + name);

//    parameters = "std{access=file,mux=%1,dst='%2'}";
//    parameters = parameters.arg(Vlc::mux()[mux], l + "." + Vlc::mux()[mux]);

//    if (duplicate) {
//        option2 = ":sout=#duplicate{dst=display,dst=\"%1\"}";
//    } else {
//        option2 = ":sout=#%1";
//    }

//    option1 = ":sout-all";
//    option2 = option2.arg(parameters);

//    setOption(option1);
//    setOption(option2);

//    VlcError::showErrmsg();

//    return l + "." + Vlc::mux()[mux];
//}

//QString VlcMedia::record(const QString &name,
//                         const QString &path,
//                         const Vlc::Mux &mux,
//                         const Vlc::AudioCodec &audioCodec,
//                         const Vlc::VideoCodec &videoCodec,
//                         bool duplicate)
//{
//    QString option1, option2, parameters;
//    QString l = QDir::toNativeSeparators(path + "/" + name);

//    parameters = "transcode{vcodec=%1,acodec=%2}:std{access=file,mux=%3,dst='%4'}";
//    parameters = parameters.arg(Vlc::videoCodec()[videoCodec], Vlc::audioCodec()[audioCodec], Vlc::mux()[mux], l + "." + Vlc::mux()[mux]);

//    if (duplicate) {
//        option2 = ":sout=#duplicate{dst=display,dst=\"%1\"}";
//    } else {
//        option2 = ":sout=#%1";
//    }

//    option1 = ":sout-all";
//    option2 = option2.arg(parameters);

//    setOption(option1);
//    setOption(option2);

//    VlcError::showErrmsg();

//    return l + "." + Vlc::mux()[mux];
//}

//QString VlcMedia::record(const QString &name,
//                         const QString &path,
//                         const Vlc::Mux &mux,
//                         const Vlc::AudioCodec &audioCodec,
//                         const Vlc::VideoCodec &videoCodec,
//                         int bitrate,
//                         int fps,
//                         int scale,
//                         bool duplicate)
//{
//    QString option1, option2, parameters;
//    QString l = QDir::toNativeSeparators(path + "/" + name);

//    parameters = "transcode{vcodec=%1,vb=%2,fps=%3,scale=%4,acodec=%5}:std{access=file,mux=%6,dst='%7'}";
//    parameters = parameters.arg(Vlc::videoCodec()[videoCodec], QString::number(bitrate), QString::number(fps), QString::number(scale), Vlc::audioCodec()[audioCodec], Vlc::mux()[mux], l + "." + Vlc::mux()[mux]);

//    if (duplicate) {
//        option2 = ":sout=#duplicate{dst=display,dst=\"%1\"}";
//    } else {
//        option2 = ":sout=#%1";
//    }

//    option1 = ":sout-all";
//    option2 = option2.arg(parameters);

//    setOption(option1);
//    setOption(option2);

//    VlcError::showErrmsg();

//    return l + "." + Vlc::mux()[mux];
//}

//void VlcMedia::setProgram(int program)
//{
//    QString option = "program=%1";
//    setOption(option.arg(program));
//}

//void VlcMedia::setOption(const QString &option)
//{
//    vlc_media_add_option_function vlc_media_add_option = (vlc_media_add_option_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("libvlc_media_add_option");
//    vlc_media_add_option(_vlcMedia, option.toUtf8().data());

//    VlcError::showErrmsg();
//}

//void VlcMedia::setOptions(const QStringList &options)
//{
//    vlc_media_add_option_function vlc_media_add_option = (vlc_media_add_option_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("libvlc_media_add_option");
//    foreach (const QString &option, options) {
//        vlc_media_add_option(_vlcMedia, option.toUtf8().data());
//    }

//    VlcError::showErrmsg();
//}

void VlcMedia::libvlc_callback(const libvlc_event_t *event,
                               void *data)
{
    VlcMedia *core = static_cast<VlcMedia *>(data);

    switch (event->type) {
    case libvlc_MediaMetaChanged:
        emit core->metaChanged(Vlc::Meta(event->u.media_meta_changed.meta_type));
        break;
    case libvlc_MediaSubItemAdded:
        emit core->subitemAdded(event->u.media_subitem_added.new_child);
        break;
    case libvlc_MediaDurationChanged:
        emit core->durationChanged(static_cast<int>(event->u.media_duration_changed.new_duration));
        break;
    case libvlc_MediaParsedChanged:
        emit core->parsedChanged(static_cast<bool>(event->u.media_parsed_changed.new_status));
        break;
    case libvlc_MediaFreed:
        emit core->freed(event->u.media_freed.md);
        break;
    case libvlc_MediaStateChanged:
        emit core->stateChanged(Vlc::State(event->u.media_state_changed.new_state));
        break;
    default:
        break;
    }
}
