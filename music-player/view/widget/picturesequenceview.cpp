/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "picturesequenceview.h"

#include <QTimer>
#include <QPainter>

class PictureSequenceViewPrivate
{
public:
    PictureSequenceViewPrivate(PictureSequenceView *parent) : q_ptr(parent)
    {
        timer = new QTimer;
        timer->setInterval(40);
    }

    QStringList         sequence;
    QList<QPixmap>      sequencePixmaps;
    QTimer              *timer;
    int                 currentIndex = 0;

    PictureSequenceView *q_ptr;
    Q_DECLARE_PUBLIC(PictureSequenceView)
};

PictureSequenceView::PictureSequenceView(QWidget *parent) : QLabel(parent), d_ptr(new PictureSequenceViewPrivate(this))
{
    Q_D(PictureSequenceView);
    QWidget::setAttribute(Qt::WA_TranslucentBackground, true);
    QWidget::setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    connect(d->timer, &QTimer::timeout,
    this, [ = ] {
        if (d->sequencePixmaps.length() <= 0)
            return ;
        d->currentIndex = (d->currentIndex + 1) % d->sequencePixmaps.length();
        this->setPixmap(d->sequencePixmaps.value(d->currentIndex));
    });
}

PictureSequenceView::~PictureSequenceView()
{
    Q_D(PictureSequenceView);
}

void PictureSequenceView::paintEvent(QPaintEvent *e)
{
    QLabel::paintEvent(e);
}

void PictureSequenceView::start()
{
    Q_D(PictureSequenceView);
    d->timer->start();
}

void PictureSequenceView::pause()
{
    Q_D(PictureSequenceView);
    d->timer->stop();
}

void PictureSequenceView::setPictureSequence(const QStringList &sequence)
{
    Q_D(PictureSequenceView);
    if (sequence.length() <= 0) {
        return ;
    }
    d->sequencePixmaps.clear();
    for (auto url : sequence) {
        d->sequencePixmaps << QPixmap(url);
    }
    this->setPixmap(d->sequencePixmaps.first());
}

void PictureSequenceView::stop()
{
    Q_D(PictureSequenceView);
    d->timer->stop();
}
