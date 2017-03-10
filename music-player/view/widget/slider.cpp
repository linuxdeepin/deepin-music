/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "slider.h"

#include <QDebug>

#include <QTime>
#include <QMouseEvent>

static const int s_valueUpdateDelay = 1500;

Slider::Slider(QWidget *parent) : QSlider(parent)
{
    setTracking(false);
}

Slider::Slider(Qt::Orientation orientation, QWidget *parent): QSlider(orientation, parent)
{
    m_delaySetValueTimer.setInterval(s_valueUpdateDelay);
//    connect(&m_delaySetValueTimer, &QTimer::timeout,
//    this, [ = ]() {
//        m_delaySetValueTimer.stop();
//        this->blockSignals(false);
//    });
}

void Slider::mouseReleaseEvent(QMouseEvent *event)
{
    this->blockSignals(false);
    QSlider::mouseReleaseEvent(event);
    emit valueAccpet(value());
}

void Slider::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton
            || event->button() == Qt::MiddleButton
            || event->button() == Qt::RightButton) {
        if (orientation() == Qt::Vertical) {
            setSliderPosition(minimum() + ((maximum() - minimum()) * (height() - event->y())) / height()) ;
        } else {
            setSliderPosition(minimum() + ((maximum() - minimum()) * (event->x())) / (width()));
        }
    }
    this->blockSignals(true);
    // Block QSlider event;
//    QSlider::mousePressEvent(event);
}

void Slider::mouseMoveEvent(QMouseEvent *event)
{
    auto valueRange = this->maximum()  - this->minimum();
    auto viewRange = this->width();

    if (0 == viewRange) {
        return;
    }

    auto value = (event->x() - this->x()) * valueRange / this->width();
//    m_delaySetValueTimer.stop();
//    m_delaySetValueTimer.start();
    setSliderPosition(value);
    QSlider::mouseMoveEvent(event);
}

void Slider::wheelEvent(QWheelEvent *e)
{
    e->accept();
}
