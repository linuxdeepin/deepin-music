// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VLCQT_SHARED_EXPORT_CORE_H
#define VLCQT_SHARED_EXPORT_CORE_H

#include <QtCore/qglobal.h>

#if defined(VLCQT_CORE_LIBRARY)
#define VLCQT_CORE_EXPORT Q_DECL_EXPORT
#else
#define VLCQT_CORE_EXPORT Q_DECL_IMPORT
#endif

#endif // SHARED_EXPORT_CORE_H
