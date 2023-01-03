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

#include "dynamiclibraries.h"
#include <QDir>
#include <QLibrary>
#include <QLibraryInfo>
#include <QDebug>

#include "global.h"

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
                qDebug() << "[ffmpeg] resolve function:" << symbol;
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
        //never get here if obey the rule
        qDebug() << "[VLC] resolve function:" << symbol;
        return fp;
    } else {
        //cache fuctionpointer for next visiting
        m_funMap[symbol] = fp;
    }

    return fp;
}

bool DynamicLibraries::loadLibraries()
{
    QString strvlccore = libPath(libvlccoreStr);
    qDebug() << "vlccore path:" << strvlccore;
    if (QLibrary::isLibrary(strvlccore)) {
        vlccoreLib.setFileName(strvlccore);
        if (!vlccoreLib.load()) {
            qDebug() << "vlccore load error!";
            return false;
        }
    } else {
        qDebug() << "vlccore is not library!";
        return false;
    }

    QString strlibvlc = libPath(libvlcStr);
    qDebug() << "libvlc path:" << strvlccore;
    if (QLibrary::isLibrary(strlibvlc)) {
        vlcLib.setFileName(strlibvlc);
        if (!vlcLib.load()) {
            qDebug() << "libvlc load error!";
            return false;
        }
    } else {
        qDebug() << "libvlc is not library!";
        return false;
    }

    QString strlibcodec = libPath(libavcodecStr);
    qDebug() << "libavcodec path:" << strvlccore;
    if (QLibrary::isLibrary(strlibcodec)) {
        avcodecLib.setFileName(strlibcodec);
        if (!avcodecLib.load()) {
            qDebug() << "libavcodec load error!";
            return false;
        }
    } else {
        qDebug() << "libavcodec is not library!";
        return false;
    }

    QString strlibformate = libPath(libavformateStr);
    qDebug() << "libavformateLib path:" << strvlccore;
    if (QLibrary::isLibrary(strlibformate)) {
        avformateLib.setFileName(strlibformate);
        if (!avformateLib.load()) {
            qDebug() << "libavformateLib load error!";
            return false;
        }
    } else {
        qDebug() << "libavformate is not library!";
        return false;
    }
    return true;
}

QString DynamicLibraries::libPath(const QString &strlib)
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
