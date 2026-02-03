// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vlcdynamicinstance.h"

#include "global.h"
#include "util/log.h"

#include <QDir>
#include <QLibrary>
#include <QLibraryInfo>
#include <QDebug>

const QString libvlccore = "libvlccore.so";
const QString libvlc = "libvlc.so";
const QString libcodec = "libavcodec.so";
const QString libformate = "libavformat.so";
const QString libSDL2 = "libSDL2-2.0";

VlcDynamicInstance::VlcDynamicInstance(QObject *parent) : QObject(parent)
{
    qCDebug(dmMusic) << "Creating VlcDynamicInstance";
    bool bret = loadVlcLibrary();
    Q_ASSERT(bret == true);
    qCDebug(dmMusic) << "VLC libraries loaded:" << bret;
}

VlcDynamicInstance::~VlcDynamicInstance()
{
    qCDebug(dmMusic) << "Destroying VlcDynamicInstance";
    if (libcore.isLoaded()) {
        qCDebug(dmMusic) << "Unloading libvlccore";
        libcore.unload();
    }
    if (libdvlc.isLoaded()) {
        qCDebug(dmMusic) << "Unloading libvlc";
        libdvlc.unload();
    }
    if (libavcode.isLoaded()) {
        qCDebug(dmMusic) << "Unloading libavcodec";
        libavcode.unload();
    }
    if (libdformate.isLoaded()) {
        qCDebug(dmMusic) << "Unloading libavformat";
        libdformate.unload();
    }
    if (libsdl2.isLoaded()) {
        qCDebug(dmMusic) << "Unloading SDL2";
        libsdl2.unload();
    }
}

VlcDynamicInstance *VlcDynamicInstance::VlcFunctionInstance()
{
    static VlcDynamicInstance  vlcinstance;
    return &vlcinstance;
}

QFunctionPointer VlcDynamicInstance::resolveSymbol(const char *symbol, bool bffmpeg)
{
    if (m_funMap.contains(symbol)) {
        return m_funMap[symbol];
    }

    if (bffmpeg) {
        QFunctionPointer fgp = libavcode.resolve(symbol);
        if (!fgp) {
            fgp = libdformate.resolve(symbol);
            if (!fgp) {
                qCWarning(dmMusic) << "Failed to resolve FFmpeg symbol:" << symbol;
            }
        }
        m_funMap[symbol] = fgp;
        return fgp;
    }
    //resolve function
    QFunctionPointer fp = libdvlc.resolve(symbol);
    if (!fp) {
        fp = libcore.resolve(symbol);
    }

    if (!fp) {
        qCWarning(dmMusic) << "Failed to resolve VLC symbol:" << symbol;
        return fp;
    } else {
        qCDebug(dmMusic) << "Successfully resolved VLC symbol:" << symbol;
        //cache fuctionpointer for next visiting
        m_funMap[symbol] = fp;
    }

    return fp;
}

QFunctionPointer VlcDynamicInstance::resolveSdlSymbol(const char *symbol)
{
    if (m_funMap.contains(symbol)) {
        return m_funMap[symbol];
    }
    QFunctionPointer fp = libsdl2.resolve(symbol);
    if (!fp) {
        qCWarning(dmMusic) << "Failed to resolve SDL2 symbol:" << symbol;
    }
    return fp;
}

bool VlcDynamicInstance::loadVlcLibrary()
{
    qCDebug(dmMusic) << "Loading VLC libraries";
    
    QString strvlccore = DmGlobal::libPath(libvlccore);
    if (QLibrary::isLibrary(strvlccore)) {
        libcore.setFileName(strvlccore);
        if (!libcore.load()) {
            qCCritical(dmMusic) << "Failed to load libvlccore:" << strvlccore << "Error:" << libcore.errorString();
            return false;
        }
        qCDebug(dmMusic) << "Successfully loaded libvlccore";
    } else {
        qCCritical(dmMusic) << "Invalid library path for libvlccore:" << strvlccore;
        return false;
    }

    QString strlibvlc = DmGlobal::libPath(libvlc);
    if (QLibrary::isLibrary(strlibvlc)) {
        libdvlc.setFileName(strlibvlc);
        if (!libdvlc.load()) {
            qCCritical(dmMusic) << "Failed to load libvlc:" << strlibvlc << "Error:" << libdvlc.errorString();
            return false;
        }
        qCDebug(dmMusic) << "Successfully loaded libvlc";
    } else {
        qCCritical(dmMusic) << "Invalid library path for libvlc:" << strlibvlc;
        return false;
    }

    QString strlibcodec = DmGlobal::libPath(libcodec);
    if (QLibrary::isLibrary(strlibcodec)) {
        libavcode.setFileName(strlibcodec);
        if (!libavcode.load()) {
            qCCritical(dmMusic) << "Failed to load libavcodec:" << strlibcodec << "Error:" << libavcode.errorString();
            return false;
        }
        qCDebug(dmMusic) << "Successfully loaded libavcodec";
    } else {
        qCCritical(dmMusic) << "Invalid library path for libavcodec:" << strlibcodec;
        return false;
    }

    QString strlibformate = DmGlobal::libPath(libformate);
    if (QLibrary::isLibrary(strlibformate)) {
        libdformate.setFileName(strlibformate);
        if (!libdformate.load()) {
            qCCritical(dmMusic) << "Failed to load libavformat:" << strlibformate << "Error:" << libdformate.errorString();
            return false;
        }
        qCDebug(dmMusic) << "Successfully loaded libavformat";
    } else {
        qCCritical(dmMusic) << "Invalid library path for libavformat:" << strlibformate;
        return false;
    }
    qCInfo(dmMusic) << "All VLC libraries loaded successfully";
    return true;
}

bool VlcDynamicInstance::loadSdlLibrary()
{
    qCDebug(dmMusic) << "Loading SDL2 library";
    if (libsdl2.isLoaded()) {
        qCDebug(dmMusic) << "SDL2 already loaded";
        return true;
    }
    QString strSdl = DmGlobal::libPath(libSDL2);
    libsdl2.setFileName(strSdl);
    bool success = libsdl2.load();
    if (!success) {
        qCCritical(dmMusic) << "Failed to load SDL2:" << strSdl << "Error:" << libsdl2.errorString();
    } else {
        qCDebug(dmMusic) << "Successfully loaded SDL2";
    }
    return success;
}
