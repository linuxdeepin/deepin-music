/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#pragma once

#include "../music.h"

class CueParser
{
public:
    explicit CueParser(const QString &filepath);
    explicit CueParser(const QString &filepath, QByteArray codeName);


    MusicMetaList metalist;
    QString musicFilePath;
};
