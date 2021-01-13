

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
