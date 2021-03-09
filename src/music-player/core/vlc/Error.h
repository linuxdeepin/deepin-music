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

#ifndef VLCQT_ERROR_H_
#define VLCQT_ERROR_H_

#include "SharedExportCore.h"


class VLCQT_CORE_EXPORT VlcError
{
public:
    /*!
        \brief A human-readable error message for the last libvlc error in the calling thread.

        The resulting string is valid until another error occurs.
        \return error message (QString)
        \warning There may be no output, if there is no error.
    */
    static QString errmsg();

    /*!
        \brief A human-readable error message displayed into standard output for the last libvlc error in the calling thread.

        The resulting string is valid until another error occurs.
        \warning There may be no output, if there is no error.
    */
    static void showErrmsg();

    /*!
        \brief Clears the libvlc error status for the current thread.

        This does nothing.

    */
    //Q_DECL_DEPRECATED static void clearerr();
};

#endif // VLCQT_ERROR_H_
