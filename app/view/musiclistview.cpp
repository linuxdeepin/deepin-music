/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "musiclistview.h"

#include <dthememanager.h>
DWIDGET_USE_NAMESPACE

MusicListView::MusicListView(QWidget *parent) : QListView(parent)
{
    setObjectName("MusicListView");

    setSelectionMode(QListView::ExtendedSelection);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
//    setFocusPolicy(Qt::NoFocus);
    D_THEME_INIT_WIDGET(MusicListView);
}
