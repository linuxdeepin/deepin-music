/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "tip.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QGraphicsDropShadowEffect>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>

#include <DUtil>
#include <dthememanager.h>
DWIDGET_USE_NAMESPACE

Tip::Tip(const QPixmap &icon, const QString &text, QWidget *parent) : QFrame(parent)
{
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setObjectName("Tip");
    setFixedHeight(40);

    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(10, 0, 10, 0);
    layout->setSpacing(5);
    auto iconLabel = new QLabel;
    iconLabel->setObjectName("TipIcon");
    iconLabel->setFixedSize(icon.size());
    if (icon.isNull()) {
        iconLabel->hide();
    } else {
        iconLabel->setPixmap(icon);
    }

    auto textLable = new QLabel(text);
    textLable->setObjectName("TipText");

    layout->addWidget(iconLabel);
    layout->addWidget(textLable);

    D_THEME_INIT_WIDGET(Widget / Tip);

    adjustSize();

    auto *bodyShadow = new QGraphicsDropShadowEffect;
    bodyShadow->setBlurRadius(20.0);
    bodyShadow->setColor(QColor(0, 0, 0, 0.1 * 255));
    bodyShadow->setOffset(2.0, 4.0);
    this->setGraphicsEffect(bodyShadow);
    hide();
}

void Tip::pop(QPoint center)
{
    this->show();
    center = center - QPoint(width() / 2, height() / 2);
    this->move(center);

    auto topOpacity = new QGraphicsOpacityEffect(this);
    topOpacity->setOpacity(1);
    this->setGraphicsEffect(topOpacity);

    QPropertyAnimation *animation4 = new QPropertyAnimation(topOpacity, "opacity");
//    animation4->setEasingCurve(QEasingCurve::InCubic);
    animation4->setDuration(4000);
    animation4->setStartValue(0);
    animation4->setKeyValueAt(0.5, 1);
    animation4->setEndValue(0);
    animation4->start();
    animation4->connect(animation4, &QPropertyAnimation::finished,
                        animation4, &QPropertyAnimation::deleteLater);
    animation4->connect(animation4, &QPropertyAnimation::finished,
    this, [ = ]() {
        this->setGraphicsEffect(nullptr);
        this->hide();
    });
}

void Tip::paintEvent(QPaintEvent *e)
{
    QFrame::paintEvent(e);
//    return;
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::HighQualityAntialiasing);
    QPainterPath path;
    auto rect = this->rect();
    path.addRoundedRect(rect, 4, 4);
    painter.fillPath(path, Qt::white);

    QPen pen(QColor(0, 0, 0, 51));
    pen.setWidth(1);
    painter.strokePath(path, pen);
}
