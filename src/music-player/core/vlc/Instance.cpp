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
#include <QtCore/QStringList>
#include "../util/global.h"
#include <vlc/vlc.h>
#include "Enums.h"
#include "Error.h"
#include "Instance.h"
#include "core/vlc/vlcdynamicinstance.h"



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
        qCritical(message.toUtf8().data(), NULL);
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
        qWarning(message.toUtf8().data(), NULL);
        break;
    case Vlc::NoticeLevel:
    case Vlc::DebugLevel:
    default:
        qDebug(message.toUtf8().data(), NULL);
        break;
    }
}

typedef libvlc_instance_t *(*vlc_new_function)(int, const char *const *);
typedef void (*vlc_set_user_agent_function)(libvlc_instance_t *, const char *, const char *);
typedef void (*vlc_set_app_id_function)(libvlc_instance_t *, const char *, const char *, const char *);
typedef void (*vlc_log_set_function)(libvlc_instance_t *, libvlc_log_cb, void *);
typedef void (*vlc_release_function)(libvlc_instance_t *);
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
    vlc_new_function vlc_new = (vlc_new_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("libvlc_new");
    vlc_set_user_agent_function vlc_set_user_agent = (vlc_set_user_agent_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("libvlc_set_user_agent");
    vlc_set_app_id_function vlc_set_app_id = (vlc_set_app_id_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("libvlc_set_app_id");
    vlc_log_set_function vlc_log_set = (vlc_log_set_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("libvlc_log_set");

    _vlcInstance = vlc_new(0, nullptr);
    vlc_set_user_agent(_vlcInstance, Global::getAppName().toStdString().c_str(), "");//name
    vlc_set_app_id(_vlcInstance, "", "", "deepin-music");//icon

    qRegisterMetaType<Vlc::Meta>("Vlc::Meta");
    qRegisterMetaType<Vlc::State>("Vlc::State");

    VlcError::showErrmsg();

    // Check if instance is running
    if (_vlcInstance) {
        vlc_log_set(_vlcInstance, logCallback, this);
        _status = true;
        qDebug() << "Using libvlc version:" << version();
    } else {
        qCritical() << "VLC Error: libvlc failed to load!";
    }
}

VlcInstance::~VlcInstance()
{
//    if (_status && _vlcInstance) {
//        vlc_release_function vlc_release = (vlc_release_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("libvlc_release");
//        vlc_release(_vlcInstance);
//    }
}

libvlc_instance_t *VlcInstance::core()
{
    return _vlcInstance;
}

//bool VlcInstance::status() const
//{
//    return _status;
//}

Vlc::LogLevel VlcInstance::logLevel() const
{
    return _logLevel;
}

//void VlcInstance::setLogLevel(Vlc::LogLevel level)
//{
//    _logLevel = level;
//}

//QString VlcInstance::changeset()
//{
//    // Returns libvlc changeset
//    vlc_get_changeset_function vlc_get_changeset = (vlc_get_changeset_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("libvlc_get_changeset");
//    return QString(vlc_get_changeset());
//}

void VlcInstance::catchPulseError(int err)
{
    Q_UNUSED(err)
    emit sendErrorOccour(0);
}

//QString VlcInstance::compiler()
//{
//    // Returns libvlc compiler version
//    vlc_get_compiler_function vlc_get_compiler = (vlc_get_compiler_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("libvlc_get_compiler");
//    return QString(vlc_get_compiler());
//}

QString VlcInstance::version()
{
    // Returns libvlc version
    vlc_get_version_function vlc_get_version = (vlc_get_version_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("libvlc_get_version");
    return QString(vlc_get_version());
}
