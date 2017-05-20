/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

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
