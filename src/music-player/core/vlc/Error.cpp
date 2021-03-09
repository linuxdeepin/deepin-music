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

#include <QtCore/QDebug>
#include <vlc/vlc.h>

#include "Error.h"
#include "core/vlc/vlcdynamicinstance.h"

typedef void (*vlc_clearerr_function)(void);
typedef const char *(*vlc_errmsg_function)(void);
QString VlcError::errmsg()
{
    QString error;
    vlc_errmsg_function vlc_errmsg = (vlc_errmsg_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("libvlc_errmsg");
    if (vlc_errmsg()) {
        error = QString::fromUtf8(vlc_errmsg());
        vlc_clearerr_function vlc_clearerr = (vlc_clearerr_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("libvlc_clearerr");
        vlc_clearerr();
    }

    return error;
}

void VlcError::showErrmsg()
{
    // Outputs libvlc error message if there is any
    QString error = errmsg();
    if (!error.isEmpty()) {
        qWarning() << "VlcError libvlc"
                   << "Error:" << error;
    }
}

//void VlcError::clearerr() {}
