#pragma once

#include <QByteArray>

#include "libdmusic_global.h"

class LIBDMUSICSHARED_EXPORT BaseTool
{
public:
    static QByteArray detectEncode(const QByteArray &data, const QString &fileName);

private:
    BaseTool() {}
};
