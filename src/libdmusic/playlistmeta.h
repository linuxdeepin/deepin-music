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

#include <QString>
#include <QMap>
#include "mediameta.h"

class PlaylistMeta
{
public:
    QString uuid;
    QString displayName;
    QString icon;

    MetaPtr                  playing;
    QStringList              sortMetas;

    QMap<QString, MetaPtr>   metas;
    QMap<QString, int>       invalidMetas;

    int     sortType    = 0;
    int     orderType   = 0;

    uint     sortID;
    bool    editmode    = false;
    bool    readonly    = false;
    bool    hide        = false;
    bool    active      = false;
};

Q_DECLARE_METATYPE(PlaylistMeta)
