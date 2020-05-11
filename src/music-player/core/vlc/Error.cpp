

#include <QtCore/QDebug>
#include <vlc/vlc.h>

#include "Error.h"

QString VlcError::errmsg()
{
    QString error;
    if (libvlc_errmsg()) {
        error = QString::fromUtf8(libvlc_errmsg());
        libvlc_clearerr();
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

void VlcError::clearerr() {}
