/*
 * Copyright (C) 2016 ~ 2018 Wuhan Deepin Technology Co., Ltd.
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

#include "searchmetaitem.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QVariant>

#include <DPushButton>
#include <DLabel>

class SearchMetaItemPrivate
{
public:
    SearchMetaItemPrivate(SearchMetaItem *parent) : q_ptr(parent) {}

    DLabel      *metaTitle   = nullptr;
    DLabel      *metaArtist  = nullptr;
    DLabel      *metaLength  = nullptr;
    DLabel      *checkedBt   = nullptr;

    SearchMetaItem *q_ptr;
    Q_DECLARE_PUBLIC(SearchMetaItem)
};

SearchMetaItem::SearchMetaItem(QWidget *parent) : DFrame(parent), d_ptr(new SearchMetaItemPrivate(this))
{
    Q_D(SearchMetaItem);
    setObjectName("SearchMetaItem");
    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(10, 0, 10, 0);

    auto interFrame = new DFrame();
    interFrame->setObjectName("SearchMetaItemInterFrame");
    auto interLayout = new QHBoxLayout(interFrame);
    interLayout->setMargin(0);

    auto metaInfoLayout = new QVBoxLayout;
    metaInfoLayout->setContentsMargins(10, 10, 10, 10);
    metaInfoLayout->setSpacing(5);

    d->metaTitle = new DLabel;
    d->metaTitle->setObjectName("SearchMetaItemTitle");
    d->metaArtist = new DLabel;
    d->metaArtist->setObjectName("SearchMetaItemArtist");

    metaInfoLayout->addWidget(d->metaTitle);
    metaInfoLayout->addWidget(d->metaArtist);

    d->checkedBt = new DLabel;
    d->checkedBt->setObjectName("SearchMetaItemCheck");
    d->checkedBt->setFixedSize(16, 16);

    d->metaLength = new DLabel;
    d->metaLength->setObjectName("SearchMetaItemLength");

    interLayout->addLayout(metaInfoLayout);
    interLayout->addStretch();
    interLayout->addWidget(d->checkedBt);
    interLayout->addWidget(d->metaLength);

    layout->addWidget(interFrame);
}

SearchMetaItem::~SearchMetaItem()
{
}

void SearchMetaItem::initUI(const DMusic::SearchMeta &meta)
{
    Q_D(SearchMetaItem);
    d->metaTitle->setText(meta.name);
    d->metaArtist->setText(meta.artists.first().name);
    d->metaLength->setText(DMusic::lengthString(meta.length));
}

void SearchMetaItem::setChecked(bool check)
{
    Q_D(SearchMetaItem);
    d->checkedBt->setProperty("active", check);
    d->checkedBt->update();
}
