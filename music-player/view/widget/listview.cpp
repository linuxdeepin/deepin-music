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

#include "listview.h"

#include <QScrollBar>

class ListViewPrivate
{
public:
    ListViewPrivate(ListView *parent) : q_ptr(parent) {}

    void checkScrollbarSize();

    QScrollBar          *vscrollBar   = nullptr;

    ListView *q_ptr;
    Q_DECLARE_PUBLIC(ListView)
};

void ListViewPrivate::checkScrollbarSize()
{
    Q_Q(ListView);

    auto itemCount = q->model()->rowCount();
    auto size = q->size();
    auto scrollBarWidth = 8;
    vscrollBar->resize(scrollBarWidth, size.height() - 2);
    vscrollBar->move(size.width() - scrollBarWidth - 2, 0);
    vscrollBar->setSingleStep(1);
    vscrollBar->setPageStep(size.height() / 36);
    vscrollBar->setMinimum(0);

    if (itemCount > size.height() / 36) {
        vscrollBar->show();
        vscrollBar->setMaximum(itemCount - size.height() / 36);
    } else {
        vscrollBar->hide();
        vscrollBar->setMaximum(0);
    }
}

ListView::ListView(QWidget *parent) :
    QListView(parent), d_ptr(new ListViewPrivate(this))
{
    Q_D(ListView);

    d->vscrollBar = new QScrollBar(this);
    d->vscrollBar->setObjectName("MusicListViewScrollBar");
    d->vscrollBar->setOrientation(Qt::Vertical);
    d->vscrollBar->raise();

    connect(d->vscrollBar, &QScrollBar::valueChanged,
    this, [ = ](int value) {
        verticalScrollBar()->setValue(value);
    });
}

ListView::~ListView()
{

}

void ListView::updateScrollbar()
{
    Q_D(ListView);
    d->checkScrollbarSize();
}

void ListView::wheelEvent(QWheelEvent *event)
{
    Q_D(ListView);
    QListView::wheelEvent(event);
    d->vscrollBar->setSliderPosition(verticalScrollBar()->sliderPosition());
}

void ListView::resizeEvent(QResizeEvent *event)
{
    Q_D(ListView);
    QListView::resizeEvent(event);
    d->checkScrollbarSize();
}
