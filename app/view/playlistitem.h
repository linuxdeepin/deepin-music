/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef PLAYLISTITEM_H
#define PLAYLISTITEM_H

#include <QWidget>

class PlayListItem : public QWidget
{
    Q_OBJECT
public:
    explicit PlayListItem(QWidget *parent = 0);

signals:

public slots:
};

#endif // PLAYLISTITEM_H