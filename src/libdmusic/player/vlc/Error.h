// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
