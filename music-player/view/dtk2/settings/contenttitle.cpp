/*
 * Copyright (C) 2016 ~ 2017 Deepin Technology Co., Ltd.
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

#include "contenttitle.h"

#include <QHBoxLayout>
#include <QLabel>

class ContentTitlePrivate
{
public:
    ContentTitlePrivate(ContentTitle *parent) : q_ptr(parent) {}

    QLabel          *titleLabel = nullptr;

    ContentTitle *q_ptr;
    Q_DECLARE_PUBLIC(ContentTitle)
};

ContentTitle::ContentTitle(QWidget *parent) :
    QWidget(parent), d_ptr(new ContentTitlePrivate(this))
{
    Q_D(ContentTitle);

    setObjectName("ContentTitle");

    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(10);

    d->titleLabel = new QLabel();
    d->titleLabel->setObjectName("ContentTitleText");
    layout->addWidget(d->titleLabel, 0, Qt::AlignLeft);
    d->titleLabel->setStyleSheet("#ContentTitleText{font-weight: 550; font-size: 14px;}");

    auto lineLabel = new QLabel;
    lineLabel->setObjectName("ContentTitleLine");
    lineLabel->setFixedHeight(1);
    lineLabel->setStyleSheet("#ContentTitleLine{border: 1px solid rgba(0,0,0,0.1);}");
    layout->addWidget(lineLabel);
    lineLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setFixedHeight(20);
}

ContentTitle::~ContentTitle()
{

}

void ContentTitle::setTitle(const QString &title)
{
    Q_D(ContentTitle);
    d->titleLabel->setText(title);
}
