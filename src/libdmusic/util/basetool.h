// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
