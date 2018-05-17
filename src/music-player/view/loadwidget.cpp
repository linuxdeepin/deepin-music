/*
 * Copyright (C) 2017 ~ 2018 Wuhan Deepin Technology Co., Ltd.
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

#include "loadwidget.h"

#include <QVBoxLayout>
#include <QLabel>

#include <DWaterProgress>

#include <DThemeManager>

DWIDGET_USE_NAMESPACE

LoadWidget::LoadWidget(QWidget *parent) :
    QFrame(parent)
{
    DThemeManager::instance()->registerWidget(this);

    auto water = new DWaterProgress(this);
    water->setValue(50);
    water->setTextVisible(false);
    water->start();

    auto label = new QLabel(tr("Loading..."));
    label->setObjectName("LoadWidgetLabel");

    auto vbox = new QVBoxLayout;
    setLayout(vbox);

    vbox->addStretch();
    vbox->addWidget(water, 0, Qt::AlignCenter);
    vbox->addSpacing(10);
    vbox->addWidget(label, 0, Qt::AlignCenter);
    vbox->addStretch();
}

