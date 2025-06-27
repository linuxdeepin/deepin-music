// Copyright (C) 2020 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "global.h"
#include "util/log.h"

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
    qCDebug(dmMusic) << "DmGlobal constructor initialized";
}

void DmGlobal::setAppName(const QString &name)
{
    qCDebug(dmMusic) << "Setting application name to:" << name;
    appName = name;
}

QString DmGlobal::getAppName()
{
    qCDebug(dmMusic) << "Getting application name:" << appName;
    return appName;
}

void DmGlobal::initPath()
{
    qCDebug(dmMusic) << "Initializing application paths";
    userConfigPath = DStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    userCachePath = DStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    userMusicPath = DStandardPaths::writableLocation(QStandardPaths::MusicLocation);
    qCDebug(dmMusic) << "Paths initialized - config:" << userConfigPath
                     << "cache:" << userCachePath << "music:" << userMusicPath;
}

void DmGlobal::setConfigPath(const QString &path)
{
    if (!path.isEmpty()) {
        qCDebug(dmMusic) << "Setting config path to:" << path;
        userConfigPath = path;
    }
}

QString DmGlobal::configPath()
{
    return userConfigPath;
}

void DmGlobal::setCachePath(const QString &path)
{
    if (!path.isEmpty()) {
        qCDebug(dmMusic) << "Setting cache path to:" << path;
        userCachePath = path;
    }
}

QString DmGlobal::cachePath()
{
    qCDebug(dmMusic) << "Getting cache path:" << userCachePath;
    return userCachePath;
}

void DmGlobal::setMusicPath(const QString &path)
{
    if (!path.isEmpty()) {
        qCDebug(dmMusic) << "Setting music path to:" << path;
        userMusicPath = path;
    }
}

QString DmGlobal::musicPath()
{
    qCDebug(dmMusic) << "Getting music path:" << userMusicPath;
    return userMusicPath;
}

void DmGlobal::setUnknownAlbumText(const QString &text)
{
    if (!text.isEmpty()) {
        qCDebug(dmMusic) << "Setting unknown album text to:" << text;
        unknownAlbumStr = text;
    }
}

QString DmGlobal::unknownAlbumText()
{
    qCDebug(dmMusic) << "Getting unknown album text:" << unknownAlbumStr;
    return unknownAlbumStr;
}

void DmGlobal::setUnknownArtistText(const QString &text)
{
    if (!text.isEmpty()) {
        qCDebug(dmMusic) << "Setting unknown artist text to:" << text;
        unknownArtistStr = text;
    }
}

QString DmGlobal::unknownArtistText()
{
    qCDebug(dmMusic) << "Getting unknown artist text:" << unknownArtistStr;
    return unknownArtistStr;
}

bool DmGlobal::checkWaylandMode()
{
    auto e = QProcessEnvironment::systemEnvironment();
    QString XDG_SESSION_TYPE = e.value(QStringLiteral("XDG_SESSION_TYPE"));
    QString WAYLAND_DISPLAY = e.value(QStringLiteral("WAYLAND_DISPLAY"));

    waylandMode = false;
    if (XDG_SESSION_TYPE == QLatin1String("wayland") || WAYLAND_DISPLAY.contains(QLatin1String("wayland"), Qt::CaseInsensitive)) {
        waylandMode = true;
        qCInfo(dmMusic) << "Wayland mode detected";
    } else {
        qCInfo(dmMusic) << "X11 mode detected";
    }

    return waylandMode;
}

void DmGlobal::setWaylandMode(bool mode)
{
    qCDebug(dmMusic) << "Setting Wayland mode to:" << mode;
    waylandMode = mode;
}

bool DmGlobal::isWaylandMode()
{
    qCDebug(dmMusic) << "Checking Wayland mode:" << waylandMode;
    return waylandMode;
}

QString DmGlobal::libPath(const QString &strlib)
{
    QDir dir;
    QString path = QLibraryInfo::location(QLibraryInfo::LibrariesPath);
    dir.setPath(path);
    QStringList list = dir.entryList(QStringList() << (strlib + "*"), QDir::NoDotAndDotDot | QDir::Files);
    QString libPath;
    
    qCDebug(dmMusic) << "Searching for library:" << strlib << "in path:" << path;
    
    if (list.contains(strlib)) {
        libPath = path + "/" + strlib;
        qCDebug(dmMusic) << "Found exact library match:" << libPath;
    } else {
        list.sort();
        for (int i = list.size() - 1; i >= 0; i--) {
            if (list[i].contains(".so")) {
                libPath = path + "/" + list[i];
                qCDebug(dmMusic) << "Found compatible library:" << libPath;
                break;
            }
        }
    }
    if (libPath.isEmpty()) {
        qCWarning(dmMusic) << "Library not found in standard paths, using default:" << strlib;
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
        
    qCDebug(dmMusic) << "Checking existence of library:" << libName;
    
    QLibrary lib(libName);
    bool bExist = lib.load();
    if (!bExist) {
        qCWarning(dmMusic) << "Failed to load library:" << libName << "Error:" << lib.errorString();
        lib.setFileName(libPath(strlib));
        bExist = lib.load();
        if (bExist) {
            qCInfo(dmMusic) << "Successfully loaded library from alternate path:" << lib.fileName();
        } else {
            qCWarning(dmMusic) << "Failed to load library from alternate path. Error:" << lib.errorString();
        }
    } else {
        qCDebug(dmMusic) << "Successfully loaded library:" << libName;
    }
    return bExist;
}

void DmGlobal::initPlaybackEngineType()
{
    qCDebug(dmMusic) << "Initializing playback engine";
    engineType = 0;
    if (libExist("libvlc.so") && libExist("libavcodec.so")) {
        engineType = 1;
        qCInfo(dmMusic) << "VLC playback engine initialized successfully";
    } else {
        qCWarning(dmMusic) << "Failed to initialize VLC playback engine, falling back to default";
    }
}

void DmGlobal::setPlaybackEngineType(int type)
{
    qCDebug(dmMusic) << "Setting playback engine type to:" << type;
    engineType = type;
}

int DmGlobal::playbackEngineType()
{
    qCDebug(dmMusic) << "Getting playback engine type:" << engineType;
    return engineType;
}
