// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QtCore/QDebug>
#include <QtCore/QStringList>

#include <vlc/vlc.h>

#include "Enums.h"
#include "Error.h"
#include "Instance.h"
#include "dynamiclibraries.h"
#include "global.h"
#include "util/log.h"

void logCallback(void *data,
                 int level,
                 const libvlc_log_t *ctx,
                 const char *fmt,
                 va_list args)
{
    Q_UNUSED(ctx)

    VlcInstance *instance = static_cast<VlcInstance *>(data);
    if (instance->logLevel() > level) {
        return;
    }

    char *result;
    if (vasprintf(&result, fmt, args) < 0) {
        return; // LCOV_EXCL_LINE
    }

    QString message(result);
    free(result);

    message.prepend("VlcInstance  libvlc: ");
    switch (level) {
    case Vlc::ErrorLevel: {
        qCCritical(dmMusic) << message;
        if (message.contains("cannot write")) {
            /*****************************************
             *vlc write error. we need to stop
             * player then start it
             * **************************************/
            instance->catchPulseError(0);
        }
        break;
    }
    case Vlc::WarningLevel:
        qCWarning(dmMusic) << message;
        break;
    case Vlc::NoticeLevel:
    case Vlc::DebugLevel:
    default:
        qCDebug(dmMusic) << message;
        break;
    }
}

typedef libvlc_instance_t *(*vlc_new_function)(int, const char *const *);
typedef void (*vlc_set_user_agent_function)(libvlc_instance_t *, const char *, const char *);
typedef void (*vlc_set_app_id_function)(libvlc_instance_t *, const char *, const char *, const char *);
typedef void (*vlc_log_set_function)(libvlc_instance_t *, libvlc_log_cb, void *);
typedef void (*vlc_release_function)(libvlc_instance_t *);
typedef void (*vlc_free_function)(libvlc_instance_t *);
typedef const char *(*vlc_get_changeset_function)(void);
typedef const char *(*vlc_get_compiler_function)(void);
typedef const char *(*vlc_get_version_function)(void);
VlcInstance::VlcInstance(const QStringList &args,
                         QObject *parent)
    : QObject(parent),
      _vlcInstance(nullptr),
      _status(false),
      _logLevel(Vlc::ErrorLevel)
{
    Q_UNUSED(args)
    vlc_new_function vlc_new = (vlc_new_function)DynamicLibraries::instance()->resolve("libvlc_new");
    vlc_set_user_agent_function vlc_set_user_agent = (vlc_set_user_agent_function)DynamicLibraries::instance()->resolve("libvlc_set_user_agent");
    vlc_set_app_id_function vlc_set_app_id = (vlc_set_app_id_function)DynamicLibraries::instance()->resolve("libvlc_set_app_id");
    vlc_log_set_function vlc_log_set = (vlc_log_set_function)DynamicLibraries::instance()->resolve("libvlc_log_set");

    _vlcInstance = vlc_new(0, nullptr);
    if (_vlcInstance) {
        qCDebug(dmMusic) << "VLC instance created successfully";
    } else {
        qCCritical(dmMusic) << "Failed to create VLC instance";
    }

    vlc_set_user_agent(_vlcInstance, DmGlobal::getAppName().toStdString().c_str(), "");//name
    vlc_set_app_id(_vlcInstance, "", "", "deepin-music");//icon

    qRegisterMetaType<Vlc::Meta>("Vlc::Meta");
    qRegisterMetaType<Vlc::State>("Vlc::State");

    VlcError::showErrmsg();

    // Check if instance is running
    if (_vlcInstance) {
        vlc_log_set(_vlcInstance, logCallback, this);
        _status = true;
        qCDebug(dmMusic) << "VLC instance initialized successfully, version:" << version();
    } else {
        qCCritical(dmMusic) << "VLC Error: libvlc failed to load!";
    }
}

VlcInstance::~VlcInstance()
{
    qCDebug(dmMusic) << "Destroying VLC instance";
    //释放Instance
    if (_vlcInstance) {
        vlc_free_function vlc_free = (vlc_free_function)DynamicLibraries::instance()->resolve("libvlc_free");
        vlc_free(_vlcInstance);
        _vlcInstance = nullptr;
        qCDebug(dmMusic) << "VLC instance released";
    }
}

libvlc_instance_t *VlcInstance::core()
{
    return _vlcInstance;
}

Vlc::LogLevel VlcInstance::logLevel() const
{
    return _logLevel;
}

void VlcInstance::catchPulseError(int err)
{
    qCWarning(dmMusic) << "Caught PulseAudio error:" << err;
    Q_UNUSED(err)
    emit sendErrorOccour(0);
}

QString VlcInstance::version()
{
    // Returns libvlc version
    vlc_get_version_function vlc_get_version = (vlc_get_version_function)DynamicLibraries::instance()->resolve("libvlc_get_version");
    return QString(vlc_get_version());
}
