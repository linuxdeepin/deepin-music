// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dynamiclibraries.h"
#include <QDir>
#include <QLibrary>
#include <QLibraryInfo>
#include <QDebug>

#include "global.h"
#include "util/log.h"

static const QString libvlccoreStr = "libvlccore.so";
static const QString libvlcStr = "libvlc.so";
static const QString libavcodecStr = "libavcodec.so";
static const QString libavformateStr = "libavformat.so";

DynamicLibraries::DynamicLibraries()
{
    bool bret = loadLibraries();
    if (!bret) {
        DmGlobal::setPlaybackEngineType(0);
    }
}

DynamicLibraries::~DynamicLibraries()
{
    qCDebug(dmMusic) << "Unloading dynamic libraries";
    vlccoreLib.unload();
    vlcLib.unload();
    avcodecLib.unload();
    avformateLib.unload();
}

DynamicLibraries *DynamicLibraries::instance()
{
    static DynamicLibraries  instance;
    return &instance;
}

QFunctionPointer DynamicLibraries::resolve(const char *symbol, bool ffmpeg)
{
    if (m_funMap.contains(symbol)) {
        return m_funMap[symbol];
    }

    if (ffmpeg) {
        QFunctionPointer fgp = avcodecLib.resolve(symbol);
        if (!fgp) {
            fgp = avformateLib.resolve(symbol);
            if (!fgp) {
                //never get here if obey the rule
                qCWarning(dmMusic) << "[ffmpeg] resolve function:" << symbol;
            }
        }
        m_funMap[symbol] = fgp;
        return fgp;
    }
    //resolve function
    QFunctionPointer fp = vlcLib.resolve(symbol);
    if (!fp) {
        fp = vlccoreLib.resolve(symbol);
    }

    if (!fp) {
        qCWarning(dmMusic) << "Failed to resolve VLC function:" << symbol;
        return fp;
    } else {
        //cache fuctionpointer for next visiting
        m_funMap[symbol] = fp;
    }

    return fp;
}

bool DynamicLibraries::loadLibraries()
{
    QString strvlccore = DmGlobal::libPath(libvlccoreStr);
    qCDebug(dmMusic) << "Loading VLC core library from:" << strvlccore;
    if (QLibrary::isLibrary(strvlccore)) {
        vlccoreLib.setFileName(strvlccore);
        if (!vlccoreLib.load()) {
            qCCritical(dmMusic) << "Failed to load VLC core library:" << vlccoreLib.errorString();
            return false;
        }
    } else {
        qCCritical(dmMusic) << "VLC core library path is not valid:" << strvlccore;
        return false;
    }

    QString strlibvlc = DmGlobal::libPath(libvlcStr);
    qCDebug(dmMusic) << "Loading VLC library from:" << strlibvlc;
    if (QLibrary::isLibrary(strlibvlc)) {
        vlcLib.setFileName(strlibvlc);
        if (!vlcLib.load()) {
            qCCritical(dmMusic) << "Failed to load VLC library:" << vlcLib.errorString();
            return false;
        }
    } else {
        qCCritical(dmMusic) << "VLC library path is not valid:" << strlibvlc;
        return false;
    }

    QString strlibcodec = DmGlobal::libPath(libavcodecStr);
    qCDebug(dmMusic) << "Loading avcodec library from:" << strlibcodec;
    if (QLibrary::isLibrary(strlibcodec)) {
        avcodecLib.setFileName(strlibcodec);
        if (!avcodecLib.load()) {
            qCCritical(dmMusic) << "Failed to load avcodec library:" << avcodecLib.errorString();
            return false;
        }
    } else {
        qCCritical(dmMusic) << "avcodec library path is not valid:" << strlibcodec;
        return false;
    }

    QString strlibformate = DmGlobal::libPath(libavformateStr);
    qCDebug(dmMusic) << "Loading avformat library from:" << strlibformate;
    if (QLibrary::isLibrary(strlibformate)) {
        avformateLib.setFileName(strlibformate);
        if (!avformateLib.load()) {
            qCCritical(dmMusic) << "Failed to load avformat library:" << avformateLib.errorString();
            return false;
        }
    } else {
        qCCritical(dmMusic) << "avformat library path is not valid:" << strlibformate;
        return false;
    }
    qCDebug(dmMusic) << "Successfully loaded all required libraries";
    return true;
}
