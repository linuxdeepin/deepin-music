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

#include "lyricview.h"

#include <QDebug>
#include <QPaintEvent>
#include <QPainter>
#include <QScrollBar>
#include <QTimer>
#include <QPropertyAnimation>

#include <DThemeManager>

#include "delegate/lyriclinedelegate.h"

DWIDGET_USE_NAMESPACE

class LyricViewPrivate
{
public:
    LyricViewPrivate(LyricView *parent) : q_ptr(parent) {}

    LyricLineDelegate   *delegate   = nullptr;
    bool                viewMode    = false;
    QTimer              *viewTimer  = nullptr;

    LyricView *q_ptr;
    Q_DECLARE_PUBLIC(LyricView)
};

LyricView::LyricView(QWidget *parent) :
    QListView(parent), d_ptr(new LyricViewPrivate(this))
{
    Q_D(LyricView);
    DThemeManager::instance()->registerWidget(this);

    d->delegate = new LyricLineDelegate(this);
    setItemDelegate(d->delegate);

    d->viewTimer = new QTimer;
    d->viewTimer->setInterval(5 * 1000);

    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    setSelectionMode(QListView::NoSelection);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setFlow(QListView::TopToBottom);


    connect(d->viewTimer, &QTimer::timeout,
    this, [ = ]() {
        d->viewMode = false;
        update();
    });
}

LyricView::~LyricView()
{

}

bool LyricView::viewMode() const
{
    Q_D(const LyricView);
    return d->viewMode;
}

int LyricView::optical() const
{
    return this->property("id_optical").toInt();
}

void LyricView::wheelEvent(QWheelEvent *event)
{
    Q_D(LyricView);
    QListView::wheelEvent(event);
    d->viewTimer->stop();
    d->viewTimer->start();
    d->viewMode = true;
}
