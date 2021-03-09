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
    if (m_funMap.contains(symbol)) {
        return m_funMap[symbol];
    }

    QFunctionPointer fgp = libdavcode.resolve(symbol);
    if (!fgp) {
        fgp = libddformate.resolve(symbol);
    }

    if (!fgp) {
        //never get here if obey the rule
        qDebug() << "[FfmpegDynamicInstance::resolveSymbol] resolve function:" << symbol;
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
