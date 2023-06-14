// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "global.h"

#include <QCoreApplication>
#include <DStandardPaths>
#include <QProcessEnvironment>
#include <QLibrary>
#include <QDir>
#include <QLibraryInfo>
#include <QDBusInterface>
#include <QDebug>

DCORE_USE_NAMESPACE;

static QString appName;
static bool waylandMode = false;
static int engineType = 0;
static bool initBoardVendorFlag = false;
static bool boardVendorFlag = false;
static bool initPanguFlag = false;
static bool panguFlag = false;


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

bool Global::checkBoardVendorType()
{
//    QFile file("/sys/class/dmi/id/board_vendor");
//    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
//        QString result(file.readAll());
//        boardVendorFlag = result.contains("HUAWEI");
//        file.close();
//    }
    QProcess process;
    process.start("dmidecode", QStringList() << "-s" << "system-product-name");
    process.waitForStarted();
    process.waitForFinished();
    QString result(process.readAll());
    boardVendorFlag = result.contains("KLVV", Qt::CaseInsensitive)
            || result.contains("KLVU", Qt::CaseInsensitive)
            || result.contains("PGUV", Qt::CaseInsensitive)
            || result.contains("PGUW", Qt::CaseInsensitive)
            || result.contains("L540", Qt::CaseInsensitive)
            || result.contains("W585", Qt::CaseInsensitive);
            process.close();

    initBoardVendorFlag = true;
    return boardVendorFlag;
}

void Global::setBoardVendorType(bool type)
{
    initBoardVendorFlag = true;
    boardVendorFlag = type;
}

bool Global::boardVendorType()
{
    if (!initBoardVendorFlag)
        checkBoardVendorType();
    return boardVendorFlag;
}

bool Global::isPangu()
{
    if (!initPanguFlag) {
        QString validFrequency = "CurrentSpeed";
        QDBusInterface systemInfoInterface("com.deepin.daemon.SystemInfo",
                                           "/com/deepin/daemon/SystemInfo",
                                           "org.freedesktop.DBus.Properties",
                                           QDBusConnection::sessionBus());
        qDebug() << "systemInfoInterface.isValid: " << systemInfoInterface.isValid();

        initPanguFlag = true;
        if (!systemInfoInterface.isValid())
            return false;

        QDBusMessage replyCpu = systemInfoInterface.call("Get", "com.deepin.daemon.SystemInfo", "CPUHardware");
        QList<QVariant> outArgsCPU = replyCpu.arguments();
        if (outArgsCPU.count()) {
            QString CPUHardware = outArgsCPU.at(0).value<QDBusVariant>().variant().toString();
            qInfo() << __FUNCTION__ << __LINE__ << "Current CPUHardware: " << CPUHardware;

            if (CPUHardware.contains("PANGU")) {

                panguFlag = true;
            }
        }
        panguFlag = false;
    }
    return !panguFlag;
}
