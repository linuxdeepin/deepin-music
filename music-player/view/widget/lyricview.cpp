/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "lyricview.h"

#include <QDebug>
#include <QPaintEvent>
#include <QPainter>
#include <QScrollBar>
#include <QTimer>
#include <QPropertyAnimation>

#include "delegate/lyriclinedelegate.h"

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
