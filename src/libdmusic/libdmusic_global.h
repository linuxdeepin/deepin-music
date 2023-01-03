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

#ifndef LIBDMUSIC_GLOBAL_H
#define LIBDMUSIC_GLOBAL_H

#include <QtCore/qglobal.h>

//#if defined(STATIC_LIB)
//#  define LIBDMUSICSHARED_EXPORT
//#else

//#endif

#if defined(LIBDMUSIC_LIBRARY)
#  define LIBDMUSICSHARED_EXPORT Q_DECL_EXPORT
#else
#  define LIBDMUSICSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // LIBDMUSIC_GLOBAL_H
