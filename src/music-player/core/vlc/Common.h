

#ifndef VLCQT_COMMON_H_
#define VLCQT_COMMON_H_

#include <QtCore/QString>
#include <QtCore/QStringList>

#include "SharedExportCore.h"


namespace VlcCommon {
/*!
    \brief Common libvlc arguments
    \return libvlc arguments (QStringList)
*/
VLCQT_CORE_EXPORT QStringList args();

/*!
    \brief Set plugin path
    \param path plugin path (QString)
    \return success status
*/
//VLCQT_CORE_EXPORT bool setPluginPath(const QString &path);
}

#endif // VLCQT_COMMON_H_
