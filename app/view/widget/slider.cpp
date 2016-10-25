/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "slider.h"

#include <QMouseEvent>
#include <dslider.h>

Slider::Slider(QWidget *parent) : QSlider(parent)
{

}

Slider::Slider(Qt::Orientation orientation, QWidget *parent): QSlider(orientation, parent)
{

}

void Slider::mousePressEvent(QMouseEvent *event)
{
    QSlider::mousePressEvent(event);
    if (event->button() == Qt::LeftButton) {
        if (orientation() == Qt::Vertical) {
            setValue(minimum() + ((maximum() - minimum()) * (height() - event->y())) / height()) ;
        } else {
            setValue(minimum() + ((maximum() - minimum()) * (event->x() )) / (width() )) ;
        }
    }
}
