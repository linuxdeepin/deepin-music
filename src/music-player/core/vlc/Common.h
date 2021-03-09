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

#ifndef VLCQT_COMMON_H_
#define VLCQT_COMMON_H_

#include <QtCore/QString>
#include <QtCore/QStringList>

#include "SharedExportCore.h"


namespace VlcCommon {
/*!
    \brief Common libvlc arguments
    \return libvlc arguments (QStringList)
*/
VLCQT_CORE_EXPORT QStringList args();

/*!
    \brief Set plugin path
    \param path plugin path (QString)
    \return success status
*/
//VLCQT_CORE_EXPORT bool setPluginPath(const QString &path);
}

#endif // VLCQT_COMMON_H_
