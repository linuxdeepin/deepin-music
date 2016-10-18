/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "playlistitem.h"

#include <QLabel>
#include <QLineEdit>
#include <QHBoxLayout>

#include <dthememanager.h>
DWIDGET_USE_NAMESPACE

PlayListItem::PlayListItem(QWidget *parent) : QWidget(parent)
{
    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(10, 0, 10, 0);
    layout->setSpacing(0);

    auto icon = new QLabel;
    icon->setObjectName("PlayListIcon");
    icon->setFixedSize(48, 48);

    auto title = new QLineEdit;
    title->setObjectName("PlayListTitle");
    title->setFixedHeight(20);
    title->setText("New Playlist");

    layout->addWidget(icon, 0, Qt::AlignCenter);
    layout->addSpacing(10);

    QSizePolicy spTitle(QSizePolicy::Preferred, QSizePolicy::Preferred);
    spTitle.setHorizontalStretch(100);
    title->setSizePolicy(spTitle);
    layout->addWidget(title, 0, Qt::AlignCenter);

    setFixedHeight(56);
    D_THEME_INIT_WIDGET(PlayListItem);
}
