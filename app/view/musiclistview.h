/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef MUSICLISTVIEW_H
#define MUSICLISTVIEW_H

#include <QListWidget>

class MusicListView : public QListView
{
    Q_OBJECT
public:
    explicit MusicListView(QWidget *parent = 0);

signals:

public slots:
};

#endif // MUSICLISTVIEW_H
