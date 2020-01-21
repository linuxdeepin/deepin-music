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

#pragma once

#include <QObject>

class VolumeMonitoringPrivate;
class VolumeMonitoring : public QObject
{
    Q_OBJECT
public:
    explicit VolumeMonitoring(QObject *parent = Q_NULLPTR);
    ~VolumeMonitoring();

    void start();
    void stop();

signals:
    void volumeChanged(int volume);
    void muteChanged(bool mute);

public slots:
    void timeoutSlot();

private:
    QScopedPointer<VolumeMonitoringPrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), VolumeMonitoring)
};
