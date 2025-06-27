// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QtCore/QDebug>
#include <vlc/vlc.h>

#include "Error.h"
#include "dynamiclibraries.h"
#include "util/log.h"

typedef void (*vlc_clearerr_function)(void);
typedef const char *(*vlc_errmsg_function)(void);
QString VlcError::errmsg()
{
    qCDebug(dmMusic) << "Getting VLC error message";
    QString error;
    vlc_errmsg_function vlc_errmsg = (vlc_errmsg_function)DynamicLibraries::instance()->resolve("libvlc_errmsg");
    if (vlc_errmsg()) {
        error = QString::fromUtf8(vlc_errmsg());
        qCDebug(dmMusic) << "VLC error message:" << error;
        vlc_clearerr_function vlc_clearerr = (vlc_clearerr_function)DynamicLibraries::instance()->resolve("libvlc_clearerr");
        vlc_clearerr();
    } else {
        qCDebug(dmMusic) << "No VLC error message available";
    }

    return error;
}

void VlcError::showErrmsg()
{
    qCDebug(dmMusic) << "Showing VLC error message";
    QString error = errmsg();
    if (!error.isEmpty()) {
        qCWarning(dmMusic) << "VLC error:" << error;
    } else {
        qCDebug(dmMusic) << "No VLC error to show";
    }
}

//void VlcError::clearerr() {}
