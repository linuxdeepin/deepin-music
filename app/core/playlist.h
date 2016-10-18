/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef PLAYLIST_H
#define PLAYLIST_H

#include <QObject>
#include "../model/musiclistmodel.h"

class Playlist : public QObject
{
    Q_OBJECT
public:
    explicit Playlist(QObject *parent = 0);

signals:

public slots:

private:
    MusicListInfo d;
};

#endif // PLAYLIST_H
