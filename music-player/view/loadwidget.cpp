/*
 * Copyright (C) 2017 ~ 2017 Deepin Technology Co., Ltd.
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

#include "widget/waterprogress.h"
#include "view/helper/thememanager.h"

class LoadWidgetPrivate
{
public:
    LoadWidgetPrivate(LoadWidget *parent) : q_ptr(parent) {}

    LoadWidget *q_ptr;
    Q_DECLARE_PUBLIC(LoadWidget)
};

LoadWidget::LoadWidget(QWidget *parent) :
    QFrame(parent), d_ptr(new LoadWidgetPrivate(this))
{
//    setObjectName("LoadWidget");
    ThemeManager::instance()->regisetrWidget(this);

    auto water = new WaterProgress(this);

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

LoadWidget::~LoadWidget()
{

}
