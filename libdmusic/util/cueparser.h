/*
 * Copyright (C) 2016 ~ 2018 Wuhan Deepin Technology Co., Ltd.
 *
 * Author:     Iceyer <me@iceyer.net>
 *
 * Maintainer: Iceyer <me@iceyer.net>
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

#pragma once

#include <QSharedPointer>
#include "../mediameta.h"

namespace DMusic
{

class CueParserPrivate;
class LIBDMUSICSHARED_EXPORT CueParser
{
public:
    explicit CueParser(const QString &filepath);
    explicit CueParser(const QString &filepath, QByteArray codeName);
    ~CueParser();

    MetaPtrList metalist() const;
    QString     mediaFilepath() const;
    QString     cueFilepath() const;

private:
    QScopedPointer<CueParserPrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), CueParser)
};

typedef QSharedPointer<CueParser>  CueParserPtr;

}
