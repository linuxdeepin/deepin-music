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
#ifdef DTKCORE_CLASS_DConfigFile
#include <DConfig>
#endif
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
    int specialMuteRecovery = -1;
#ifdef DTKCORE_CLASS_DConfigFile
    //需要查询是否支持特殊机型静音恢复，例如hw机型
    DConfig *dconfig = DConfig::create("org.deepin.music","org.deepin.music");
    //需要判断Dconfig文件是否合法
    if(dconfig && dconfig->isValid() && dconfig->keyList().contains("specialMuteRecovery")){
        specialMuteRecovery = dconfig->value("specialMuteRecovery").toInt();
    }
#endif
    qInfo() << "specialMuteRecovery value is:" << specialMuteRecovery;
    if(specialMuteRecovery != -1){
        boardVendorFlag = specialMuteRecovery? true:false;
    }else{
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

        process.start("bash", QStringList() << "-c" << "dmidecode | grep -i \"String 4\"");
        process.waitForStarted();
        process.waitForFinished();
        result = process.readAll();
        //qDebug() << __func__ << result;
        boardVendorFlag = boardVendorFlag
                || result.contains("PWC30", Qt::CaseInsensitive);    //w525
        process.close();
    }
    initBoardVendorFlag = true;
    qInfo() << "Whether special mute recovery mode is supported?" << boardVendorFlag;
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
    int specialBlockLockScreen = -1;
#ifdef DTKCORE_CLASS_DConfigFile
    //需要查询是否支持特殊机型阻止锁屏，例如m900机型
    DConfig *dconfig = DConfig::create("org.deepin.music","org.deepin.music");
    if(dconfig && dconfig->isValid() && dconfig->keyList().contains("specialBlockLockScreen")){
        specialBlockLockScreen = dconfig->value("specialBlockLockScreen").toInt();
    }
#endif
    qInfo() << "specialBlockLockScreen value is:" << specialBlockLockScreen;
    if(specialBlockLockScreen != -1){
        initPanguFlag = true;
        panguFlag = specialBlockLockScreen? true:false;
    }else{
        if (!initPanguFlag) {
            QDBusInterface systemInfoInterface("com.deepin.daemon.SystemInfo",
                                               "/com/deepin/daemon/SystemInfo",
                                               "org.freedesktop.DBus.Properties",
                                               QDBusConnection::sessionBus());

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
        }
    }
    qInfo() << "Whether Block the lock screen on some special models is supported, like m900?" << panguFlag;
    return panguFlag;
}
