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

#include "global.h"

#include <QCoreApplication>
#include <DStandardPaths>
#include <QProcessEnvironment>
#include <QLibrary>
#include <QDir>
#include <QLibraryInfo>

DCORE_USE_NAMESPACE;

static QString appName;
static bool waylandMode = false;
static int engineType = 0;

QString Global::configPath()
{
    QString userConfigPath = DStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    return userConfigPath;
}

QString Global::cacheDir()
{
    QString userCachePath = DStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    return userCachePath;
}

void Global::setAppName(const QString &name)
{
    appName = name;
}

QString Global::getAppName()
{
    return appName;
}

bool Global::checkWaylandMode()
{
    auto e = QProcessEnvironment::systemEnvironment();
    QString XDG_SESSION_TYPE = e.value(QStringLiteral("XDG_SESSION_TYPE"));
    QString WAYLAND_DISPLAY = e.value(QStringLiteral("WAYLAND_DISPLAY"));

    waylandMode = false;
    if (XDG_SESSION_TYPE == QLatin1String("wayland") || WAYLAND_DISPLAY.contains(QLatin1String("wayland"), Qt::CaseInsensitive)) //是否开启wayland
        waylandMode = true;

    return waylandMode;
}

void Global::setWaylandMode(bool mode)
{
    waylandMode = mode;
}

bool Global::isWaylandMode()
{
    return waylandMode;
}

void Global::initPlaybackEngineType()
{
    engineType = 0;
    QDir dir(QLibraryInfo::location(QLibraryInfo::LibrariesPath));
    QStringList list = dir.entryList(QStringList() << QString("libvlccore.so*") << QString("libavcodec.so*"), QDir::NoDotAndDotDot | QDir::Files);
    bool vlcFlag = false, avFlag = false;
    for (auto str : list) {
        if (!vlcFlag && str.startsWith("libvlccore.so")) vlcFlag = true;
        if (!avFlag && str.startsWith("libavcodec.so")) avFlag = true;
        if (vlcFlag && avFlag) {
            engineType = 1;
            break;
        }
    }
}

// 播放引擎类型1为vlc，0为QMediaPlayer
int Global::playbackEngineType()
{
    return engineType;
}

