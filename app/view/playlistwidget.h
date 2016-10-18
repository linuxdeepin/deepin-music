/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef PLAYLISTWIDGET_H
#define PLAYLISTWIDGET_H

#include <QListWidget>

class PlayListWidget : public QListWidget
{
    Q_OBJECT
public:
    explicit PlayListWidget(QWidget *parent = 0);

signals:

public slots:
};

#endif // PLAYLISTWIDGET_H
