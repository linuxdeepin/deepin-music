// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ffmpegdynamicinstance.h"
#include <QDir>
#include <QLibrary>
#include <QLibraryInfo>
#include <QDebug>

const QString libdcodec = "libavcodec.so";
const QString libdformate = "libavformat.so";

FfmpegDynamicInstance::FfmpegDynamicInstance(QObject *parent) : QObject(parent)
{
    bool bret = loadVlcLibrary();
    Q_ASSERT(bret == true);
}

FfmpegDynamicInstance::~FfmpegDynamicInstance()
{
    libdavcode.unload();
    libddformate.unload();
}

FfmpegDynamicInstance *FfmpegDynamicInstance::VlcFunctionInstance()
{
    static FfmpegDynamicInstance  vlcinstance;
    return &vlcinstance;
}

QFunctionPointer FfmpegDynamicInstance::resolveSymbol(const char *symbol, bool bffmpeg)
{
    qDebug() << __func__ << symbol << bffmpeg;
    // m_funMap是非线程安全的，对读写操作进行加锁
    QMutexLocker locker(&m_funMapMutex);  // 自动加锁解锁
    if (m_funMap.contains(symbol)) {
        return m_funMap[symbol];
    }

    QFunctionPointer fgp = libdavcode.resolve(symbol);
    if (!fgp) {
        fgp = libddformate.resolve(symbol);
    }

    if (!fgp) {
        //never get here if obey the rule
        qCritical() << "[FfmpegDynamicInstance::resolveSymbol] resolve function:" << symbol << "FAILED";
        return fgp;
    } else {
        //cache fuctionpointer for next visiting
        m_funMap[symbol] = fgp;
    }

    return fgp;
}

bool FfmpegDynamicInstance::loadVlcLibrary()
{
    QString strlibcodec = libPath(libdcodec);
    if (QLibrary::isLibrary(strlibcodec)) {
        libdavcode.setFileName(strlibcodec);
        if (!libdavcode.load()) {
            return false;
        }
    } else {
        return false;
    }

    QString strformate = libPath(libdformate);
    if (QLibrary::isLibrary(strformate)) {
        libddformate.setFileName(strformate);
        if (!libddformate.load()) {
            return false;
        }
    } else {
        return false;
    }
    return true;
}

QString FfmpegDynamicInstance::libPath(const QString &strlib)
{
    QDir  dir;
    QString path  = QLibraryInfo::location(QLibraryInfo::LibrariesPath);
    dir.setPath(path);
    QStringList list = dir.entryList(QStringList() << (strlib + "*"), QDir::NoDotAndDotDot | QDir::Files); //filter name with strlib
    if (list.contains(strlib)) {
        return strlib;
    } else {
        list.sort();
    }

    Q_ASSERT(list.size() > 0);
    return list.last();
}
