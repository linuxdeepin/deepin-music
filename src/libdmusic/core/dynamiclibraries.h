// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DYNAMICLIBRARIES_H
#define DYNAMICLIBRARIES_H

#include <QLibrary>
#include <QMap>

class DynamicLibraries
{
public:
    static DynamicLibraries *instance();
    QFunctionPointer resolve(const char *symbol, bool ffmpeg = false);

private:
    explicit DynamicLibraries();
    ~DynamicLibraries();
    bool loadLibraries();
    QLibrary vlccoreLib;
    QLibrary vlcLib;
    QLibrary avcodecLib;
    QLibrary avformateLib;
    QLibrary avutilLib;
    QLibrary swresampleLib;

    QMap<QString, QFunctionPointer> m_funMap;
};

#endif // DYNAMICLIBRARIES_H
