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
    /**
     * @brief libPath get absolutely library path
     * @param strlib library name
     * @return
     */
    QString libPath(const QString &strlib);
    QLibrary vlccoreLib;
    QLibrary vlcLib;
    QLibrary avcodecLib;
    QLibrary avformateLib;

    QMap<QString, QFunctionPointer> m_funMap;
};

#endif // DYNAMICLIBRARIES_H
