/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "menu.h"

#include <QGraphicsDropShadowEffect>
#include <dthememanager.h>
DWIDGET_USE_NAMESPACE

Menu::Menu(QWidget *parent) : QMenu(parent)
{
    QWidget::setAttribute(Qt::WA_TranslucentBackground, true);

//    auto m_shadowColor = QColor(0, 0, 0, 255);

//    QGraphicsDropShadowEffect *bodyShadow = new QGraphicsDropShadowEffect;
//    bodyShadow->setBlurRadius(9.0);
//    bodyShadow->setColor(m_shadowColor);
//    bodyShadow->setOffset(0.0, -2.0);
//    this->setGraphicsEffect(bodyShadow);

    D_THEME_INIT_WIDGET(Menu);
}
