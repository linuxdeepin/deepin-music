// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QList>
#include "libdmusic_global.h"

namespace DMusic {
class LIBDMUSICSHARED_EXPORT EncodingDetector
{
public:
    static QList<QByteArray> detectEncodings(const QByteArray &rawData);
};
}
