/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "soundvolume.h"

#include <QDebug>
#include <QPainter>
#include <QWheelEvent>
#include <QVBoxLayout>

#include <DUtil>
#include <dslider.h>

using namespace Dtk::Widget;

class SoundVolumePrivate
{
public:
    SoundVolumePrivate(SoundVolume *parent) : q_ptr(parent) {}

    bool        mouseIn     = false;
    QSlider     *volSlider  = nullptr;

    SoundVolume *q_ptr;
    Q_DECLARE_PUBLIC(SoundVolume);
};

SoundVolume::SoundVolume(QWidget *parent) : QWidget(parent), d_ptr(new SoundVolumePrivate(this))
{
    Q_D(SoundVolume);
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(40, 106);
    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 5, 0, 11);
    layout->setSpacing(0);

    d->volSlider = new QSlider(Qt::Vertical);
    d->volSlider->setMinimum(0);
    d->volSlider->setMaximum(100);
    d->volSlider->setValue(50);
    d->volSlider->setFixedHeight(90);

    layout->addStretch();
    layout->addWidget(d->volSlider, 0, Qt::AlignCenter);
    layout->addStretch();
    setFixedSize(24, 106);

    connect(d->volSlider, &QSlider::valueChanged,
            this, &SoundVolume::volumeChanged);
}

SoundVolume::~SoundVolume()
{

}

int SoundVolume::volume() const
{
    Q_D(const SoundVolume);
    return d->volSlider->value();
}

void SoundVolume::deleyHide()
{
    Q_D(SoundVolume);
    d->mouseIn = false;
    DUtil::TimerSingleShot(1000, [this]() {
        Q_D(SoundVolume);
        if (!d->mouseIn) {
            hide();
        }
    });
}

void SoundVolume::onVolumeChanged(int volume)
{
    Q_D(SoundVolume);
    d->volSlider->blockSignals(true);
    d->volSlider->setValue(volume);
    d->volSlider->blockSignals(false);
}

void SoundVolume::showEvent(QShowEvent *event)
{
    Q_D(SoundVolume);
    d->mouseIn = true;
    QWidget::showEvent(event);
}

void SoundVolume::enterEvent(QEvent *event)
{
    Q_D(SoundVolume);
    d->mouseIn = true;
    QWidget::enterEvent(event);
}

void SoundVolume::leaveEvent(QEvent *event)
{
    Q_D(SoundVolume);
    d->mouseIn = false;
    deleyHide();
    QWidget::leaveEvent(event);
}

void SoundVolume::wheelEvent(QWheelEvent *event)
{
    QWidget::wheelEvent(event);
//    Q_D(SoundVolume);
//    if (event->angleDelta().y() > 0) {
//        d->volSlider->setValue(d->volSlider->value() + 5);
//    } else {
//        d->volSlider->setValue(d->volSlider->value() - 5);
//    }
}

void SoundVolume::paintEvent(QPaintEvent * /*event*/)
{
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::HighQualityAntialiasing);
    QPainterPath path;

    const qreal radius = 4;
    const qreal triHeight = 6;
    const qreal triWidth = 6;
    const qreal height = this->height() - triHeight;
    const qreal width = this->width();

    QRectF topRightRect(QPointF(0, 0),
                        QPointF(2 * radius, 2 * radius));
    QRectF bottomRightRect(QPointF(0, height - 2 * radius),
                           QPointF(2 * radius, height));
    QRectF topLeftRect(QPointF(width, 0),
                       QPointF(width - 2 * radius, 2 * radius));
    QRectF bottomLeftRect(QPointF(width, height),
                          QPointF(width - 2 * radius, height - 2 * radius));

    path.moveTo(radius, 0.0);
    path.lineTo(width - radius, 0.0);
    path.arcTo(topLeftRect, 90.0, 90.0);
    path.lineTo(width, height - radius);
    path.arcTo(bottomLeftRect, 180.0, -90.0);
    path.lineTo(width / 2 + triWidth / 2, height);
    path.lineTo(width / 2, height + triHeight);
    path.lineTo(width / 2 - triWidth / 2, height);
    path.lineTo(radius, height);

    path.arcTo(bottomRightRect, 270.0, -90.0);
    path.lineTo(0.0, radius);

    path.arcTo(topRightRect, 180.0, -90.0);
    path.lineTo(radius, 0.0);

    painter.fillPath(path, Qt::white);

    QPen pen(QColor(0, 0, 0, 51));
    pen.setWidth(1);
    painter.strokePath(path, pen);
}
