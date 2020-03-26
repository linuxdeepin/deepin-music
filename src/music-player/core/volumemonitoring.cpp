/*
 * Copyright (C) 2017 ~ 2018 Wuhan Deepin Technology Co., Ltd.
 *
 * Author:     Iceyer <me@iceyer.net>
 *
 * Maintainer: Iceyer <me@iceyer.net>
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

#include "volumemonitoring.h"

#include <QTimer>
#include <QDBusObjectPath>
#include <QDBusInterface>
#include <QDBusReply>

#include "util/dbusutils.h"
#include "musicsettings.h"

class VolumeMonitoringPrivate
{
public:
    VolumeMonitoringPrivate(VolumeMonitoring *parent) : q_ptr(parent) {}

    QTimer            timer;

    VolumeMonitoring *q_ptr;
    Q_DECLARE_PUBLIC(VolumeMonitoring)
};

VolumeMonitoring::VolumeMonitoring(QObject *parent)
    : QObject(parent), d_ptr(new VolumeMonitoringPrivate(this))
{
    Q_D(VolumeMonitoring);
    connect(&d->timer, SIGNAL(timeout()), this, SLOT(timeoutSlot()));
}

VolumeMonitoring::~VolumeMonitoring()
{
    stop();
}

void VolumeMonitoring::start()
{
    Q_D(VolumeMonitoring);
    d->timer.start(1000);
}

void VolumeMonitoring::stop()
{
    Q_D(VolumeMonitoring);
    d->timer.stop();
}

void VolumeMonitoring::timeoutSlot()
{
    QVariant v = DBusUtils::redDBusProperty("com.deepin.daemon.Audio", "/com/deepin/daemon/Audio",
                                            "com.deepin.daemon.Audio", "SinkInputs");

    if (!v.isValid())
        return;

    QList<QDBusObjectPath> allSinkInputsList = v.value<QList<QDBusObjectPath> >();

    QString sinkInputPath;
    for (auto curPath : allSinkInputsList) {
        QVariant nameV = DBusUtils::redDBusProperty("com.deepin.daemon.Audio", curPath.path(),
                                                    "com.deepin.daemon.Audio.SinkInput", "Name");

        if (!nameV.isValid() || nameV.toString() != "Music")
            continue;

        sinkInputPath = curPath.path();
        break;
    }
    if (sinkInputPath.isEmpty())
        return;

    QDBusInterface ainterface("com.deepin.daemon.Audio", sinkInputPath,
                              "com.deepin.daemon.Audio.SinkInput",
                              QDBusConnection::sessionBus());
    if (!ainterface.isValid()) {
        return ;
    }

    //获取音量
    QVariant volumeV = DBusUtils::redDBusProperty("com.deepin.daemon.Audio", sinkInputPath,
                                                  "com.deepin.daemon.Audio.SinkInput", "Volume");

    //获取音量
    QVariant muteV = DBusUtils::redDBusProperty("com.deepin.daemon.Audio", sinkInputPath,
                                                "com.deepin.daemon.Audio.SinkInput", "Mute");

    int volume = volumeV.toDouble() * 100;
    bool mute = muteV.toBool();

    auto oldMute = MusicSettings::value("base.play.mute").toBool();
    auto oldVolume = MusicSettings::value("base.play.volume").toInt();

    if (volume != oldMute)
        Q_EMIT volumeChanged(volume);
    if (mute != oldVolume)
        Q_EMIT muteChanged(muteV.toBool());
}
