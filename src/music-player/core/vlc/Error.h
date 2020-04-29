

#ifndef VLCQT_ERROR_H_
#define VLCQT_ERROR_H_

#include "SharedExportCore.h"

/*!
    \class VlcError Error.h VLCQtCore/Error.h
    \ingroup VLCQtCore
    \brief Error handler

    A basic error handler for VLC-Qt library.
*/
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

        \deprecated Deprecated since VLC-Qt 1.1, will be removed in 2.0
    */
    Q_DECL_DEPRECATED static void clearerr();
};

#endif // VLCQT_ERROR_H_
