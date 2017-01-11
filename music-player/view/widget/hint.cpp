/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "hint.h"

#include <QLabel>
#include <QPainter>
#include <QGraphicsDropShadowEffect>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>

#include <DUtil>
#include <thememanager.h>

class HintPrivate
{
public:
    HintPrivate(Hint *parent) : q_ptr(parent) {}

    QWidget *centerWidget;

    Hint *q_ptr;
    Q_DECLARE_PUBLIC(Hint)
};

Hint::Hint(QWidget *parent) : QWidget(parent), d_ptr(new HintPrivate(this))
{
    Q_D(Hint);

    d->centerWidget = new QLabel("Unknow Hint");
}

Hint::~Hint()
{

}
