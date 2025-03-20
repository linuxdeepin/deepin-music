// Copyright (C) 2020 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "global.h"

#include <QVariantMap>
#include <QProcessEnvironment>
#include <QLibraryInfo>
#include <QDir>
#include <QDebug>
#include <QLibrary>

#include <DStandardPaths>

DCORE_USE_NAMESPACE;

static QString userConfigPath = "";
static QString userCachePath = "";
static QString userMusicPath = "";
static QString unknownAlbumStr = "Unknown album";
static QString unknownArtistStr = "Unknown artist";
static bool waylandMode = false;
static QString appName;
static int engineType = 0;

DmGlobal::DmGlobal(QObject *parent)
    : QObject(parent)
{

}

void DmGlobal::setAppName(const QString &name)
{
    appName = name;
}

QString DmGlobal::getAppName()
{
    return appName;
}

void DmGlobal::initPath()
{
    userConfigPath = DStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    userCachePath = DStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    userMusicPath = DStandardPaths::writableLocation(QStandardPaths::MusicLocation);
}

void DmGlobal::setConfigPath(const QString &path)
{
    if (!path.isEmpty())
        userConfigPath = path;
}

QString DmGlobal::configPath()
{
    return userConfigPath;
}

void DmGlobal::setCachePath(const QString &path)
{
    if (!path.isEmpty())
        userCachePath = path;
}

QString DmGlobal::cachePath()
{
    return userCachePath;
}

void DmGlobal::setMusicPath(const QString &path)
{
    if (!path.isEmpty())
        userMusicPath = path;
}

QString DmGlobal::musicPath()
{
    return userMusicPath;
}

void DmGlobal::setUnknownAlbumText(const QString &text)
{
    if (!text.isEmpty())
        unknownAlbumStr = text;
}

QString DmGlobal::unknownAlbumText()
{
    return unknownAlbumStr;
}

void DmGlobal::setUnknownArtistText(const QString &text)
{
    if (!text.isEmpty())
        unknownArtistStr = text;
}

QString DmGlobal::unknownArtistText()
{
    return unknownArtistStr;
}

bool DmGlobal::checkWaylandMode()
{
    auto e = QProcessEnvironment::systemEnvironment();
    QString XDG_SESSION_TYPE = e.value(QStringLiteral("XDG_SESSION_TYPE"));
    QString WAYLAND_DISPLAY = e.value(QStringLiteral("WAYLAND_DISPLAY"));

    waylandMode = false;
    if (XDG_SESSION_TYPE == QLatin1String("wayland") || WAYLAND_DISPLAY.contains(QLatin1String("wayland"), Qt::CaseInsensitive)) //是否开启wayland
        waylandMode = true;

    return waylandMode;
}

void DmGlobal::setWaylandMode(bool mode)
{
    waylandMode = mode;
}

bool DmGlobal::isWaylandMode()
{
    return waylandMode;
}

QString DmGlobal::libPath(const QString &strlib)
{
    QDir  dir;
    QString path  = QLibraryInfo::location(QLibraryInfo::LibrariesPath);
    dir.setPath(path);
    QStringList list = dir.entryList(QStringList() << (strlib + "*"), QDir::NoDotAndDotDot | QDir::Files); //filter name with strlib
    QString libPath;
    if (list.contains(strlib)) {
        libPath = path + "/" + strlib;
    } else {
        list.sort();
        for (int i = list.size() - 1; i >= 0; i--) {
            if (list[i].contains(".so")) {
                libPath = path + "/" + list[i];
                break;
            }
        }
    }
    if (libPath.isEmpty()) {
        libPath = strlib;
    }

    return libPath;
}

bool DmGlobal::libExist(const QString &strlib)
{
    // find all library paths by QLibrary
    QString libName;
    if (strlib.contains(".so"))
        libName = strlib.mid(0, strlib.indexOf(".so"));
    else
        libName = strlib;
    QLibrary lib(libName);
    bool bExist = lib.load();
    if (!bExist) {
        qWarning() << "Failed to load library:" << lib.errorString();
        lib.setFileName(libPath(strlib));
        bExist = lib.load();
    }
    return bExist;
}

void DmGlobal::initPlaybackEngineType()
{
    engineType = 0;
    if (libExist("libvlc.so") && libExist("libavcodec.so")) {
        engineType = 1;
    }
    qDebug() << "initPlaybackEngineType: " << engineType;
}

void DmGlobal::setPlaybackEngineType(int type)
{
    engineType = type;
}

int DmGlobal::playbackEngineType()
{
    return engineType;
}
