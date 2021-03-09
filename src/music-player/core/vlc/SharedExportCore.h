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

#ifndef VLCQT_SHARED_EXPORT_CORE_H
#define VLCQT_SHARED_EXPORT_CORE_H

#include <QtCore/qglobal.h>

#if defined(VLCQT_CORE_LIBRARY)
#define VLCQT_CORE_EXPORT Q_DECL_EXPORT
#else
#define VLCQT_CORE_EXPORT Q_DECL_IMPORT
#endif

#endif // SHARED_EXPORT_CORE_H
