/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#pragma once

#include <QList>
#include "libdmusic_global.h"

namespace DMusic
{
class LIBDMUSICSHARED_EXPORT EncodingDetector
{
public:
    static QList<QByteArray> detectEncodings (const QByteArray& rawData);
};
}
