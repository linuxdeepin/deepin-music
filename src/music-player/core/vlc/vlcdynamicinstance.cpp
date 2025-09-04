// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vlcdynamicinstance.h"
#include <QDir>
#include <QLibrary>
#include <QLibraryInfo>
#include <QDebug>

const QString libvlccore = "libvlccore.so";
const QString libvlc = "libvlc.so";
const QString libcodec = "libavcodec.so";
const QString libformate = "libavformat.so";
const QString libSDL2 = "libSDL2";

VlcDynamicInstance::VlcDynamicInstance(QObject *parent) : QObject(parent)
{
    bool bret = loadVlcLibrary();
    Q_ASSERT(bret == true);
}

VlcDynamicInstance::~VlcDynamicInstance()
{
    if (libcore.isLoaded())
        libcore.unload();
    if (libdvlc.isLoaded())
        libdvlc.unload();
    if (libavcode.isLoaded())
        libavcode.unload();
    if (libdformate.isLoaded())
        libdformate.unload();
    if (libsdl2.isLoaded())
        libsdl2.unload();
}

VlcDynamicInstance *VlcDynamicInstance::VlcFunctionInstance()
{
    static VlcDynamicInstance  vlcinstance;
    return &vlcinstance;
}

QFunctionPointer VlcDynamicInstance::resolveSymbol(const char *symbol, bool bffmpeg)
{
    qDebug() << __func__ << symbol << bffmpeg;
    // m_funMap是非线程安全的，对读写操作进行加锁
    QMutexLocker locker(&m_funMapMutex);  // 自动加锁解锁
    if (m_funMap.contains(symbol)) {
        return m_funMap[symbol];
    }

    if (bffmpeg) {
        QFunctionPointer fgp = libavcode.resolve(symbol);
        if (!fgp) {
            fgp = libdformate.resolve(symbol);
            if (!fgp) {
                //never get here if obey the rule
                qCritical() << "[VlcDynamicInstance::resolveSymbol] resolve function:" << symbol << "FAILED";
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
        //never get here if obey the rule
        qCritical() << "[VlcDynamicInstance::resolveSymbol] resolve function:" << symbol << "FAILED";
        return fp;
    } else {
        //cache fuctionpointer for next visiting
        m_funMap[symbol] = fp;
    }

    return fp;
}

QFunctionPointer VlcDynamicInstance::resolveSdlSymbol(const char *symbol)
{
    // m_funMap是非线程安全的，对读写操作进行加锁
    QMutexLocker locker(&m_funMapMutex);  // 自动加锁解锁
    if (m_funMap.contains(symbol)) {
        return m_funMap[symbol];
    }
    return libsdl2.resolve(symbol);
}

bool VlcDynamicInstance::loadVlcLibrary()
{
    QString strvlccore = libPath(libvlccore);
    if (QLibrary::isLibrary(strvlccore)) {
        libcore.setFileName(strvlccore);
        if (!libcore.load())
            return false;
    } else {
        return false;
    }

    QString strlibvlc = libPath(libvlc);
    if (QLibrary::isLibrary(strlibvlc)) {
        libdvlc.setFileName(strlibvlc);
        if (!libdvlc.load()) {
            return false;
        }
    } else {
        return false;
    }

    QString strlibcodec = libPath(libcodec);
    if (QLibrary::isLibrary(strlibcodec)) {
        libavcode.setFileName(strlibcodec);
        if (!libavcode.load()) {
            return false;
        }
    } else {
        return false;
    }

    QString strlibformate = libPath(libformate);
    if (QLibrary::isLibrary(strlibformate)) {
        libdformate.setFileName(strlibformate);
        if (!libdformate.load()) {
            return false;
        }
    } else {
        return false;
    }
    return true;
}

bool VlcDynamicInstance::loadSdlLibrary()
{
    QString strSdl = libPath(libSDL2);
    if (QLibrary::isLibrary(strSdl)) {
        libsdl2.setFileName(strSdl);
        return libsdl2.load();
    } else {
        return false;
    }
}

QString VlcDynamicInstance::libPath(const QString &strlib)
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

    return libPath;
}
