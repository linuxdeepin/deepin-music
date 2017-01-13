/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

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
