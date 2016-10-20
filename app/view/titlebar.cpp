/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "titlebar.h"

#include <QLabel>
#include <QHBoxLayout>

#include <dthememanager.h>
#include <dsearchedit.h>

DWIDGET_USE_NAMESPACE

TitleBar::TitleBar(QWidget *parent) : QFrame(parent)
{
    setObjectName("TitleBar");

    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(10, 5, 10, 5);

    auto iconLabel = new QLabel;
    iconLabel->setObjectName("TitleIcon");
    iconLabel->setFixedSize(20, 20);

    auto search = new DSearchEdit;
    search->setObjectName("TitleSearch");
    search->setFixedSize(278, 26);
    search->setPlaceHolder(tr("Search"));
    search->clear();

    layout->addWidget(iconLabel, 0, Qt::AlignHCenter);
    layout->addStretch();
    layout->addWidget(search, 0, Qt::AlignHCenter);
    layout->addStretch();

    D_THEME_INIT_WIDGET(TitleBar);
}
