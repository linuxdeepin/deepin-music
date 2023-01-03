// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
