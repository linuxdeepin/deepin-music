/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "musicitem.h"
#include <QLabel>
#include <QHBoxLayout>

#include <dthememanager.h>
DWIDGET_USE_NAMESPACE

#include "../model/musiclistmodel.h"

MusicItem::MusicItem(int num, const MusicInfo &info, QWidget *parent)
    : QWidget(parent), m_info(info)
{
    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(20, 5, 20, 5);

    auto number = new QLabel;
    number->setObjectName("MusicNumber");
    number->setText(QString("%1").arg(num));

    auto titel = new QLabel;
    titel->setObjectName("MusicTitle");
    titel->setText(info.title);

    auto  artist = new QLabel;
    artist->setObjectName("MusciArtist");
    artist->setText(info.artist);

    auto album = new QLabel;
    album->setObjectName("MusicAlbum");
    album->setText(info.album);

    auto length = new QLabel;
    length->setObjectName("MusicLength");
    length->setText(QString("%1").arg(info.lenght));

    QSizePolicy spNumber(QSizePolicy::Fixed, QSizePolicy::Preferred);
    number->setSizePolicy(spNumber);
    number->setFixedWidth(20);
    layout->addWidget(number, 0, Qt::AlignCenter);

    QSizePolicy spTitle(QSizePolicy::Preferred, QSizePolicy::Preferred);
    spTitle.setHorizontalStretch(2);
    titel->setSizePolicy(spTitle);
    layout->addWidget(titel, 0, Qt::AlignLeft | Qt::AlignVCenter);

    QSizePolicy spArtist(QSizePolicy::Preferred, QSizePolicy::Preferred);
    spArtist.setHorizontalStretch(1);
    artist->setSizePolicy(spArtist);
    layout->addWidget(artist, 0, Qt::AlignLeft | Qt::AlignVCenter);

    QSizePolicy spAlbum(QSizePolicy::Preferred, QSizePolicy::Preferred);
    spAlbum.setHorizontalStretch(1);
    album->setSizePolicy(spAlbum);
    layout->addWidget(album, 0, Qt::AlignLeft | Qt::AlignVCenter);

    QSizePolicy spLength(QSizePolicy::Fixed, QSizePolicy::Preferred);
    length->setSizePolicy(spLength);
    length->setFixedWidth(40);
    layout->addWidget(length, 0, Qt::AlignCenter);

    setFixedHeight(36);
    D_THEME_INIT_WIDGET(MusicItem);
}
