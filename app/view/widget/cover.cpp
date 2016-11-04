/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "cover.h"

#include <QPainter>
#include <QTemporaryFile>
#include <QDebug>
#include <QGraphicsDropShadowEffect>

Cover::Cover(QWidget *parent) : QWidget(parent)
{
    QWidget::setAttribute(Qt::WA_TranslucentBackground, true);
    m_radius = 4;
    m_borderColor = QColor(0, 0, 0, 52);
    m_shadowColor = QColor(0, 0, 0, 26);

    QGraphicsDropShadowEffect *bodyShadow = new QGraphicsDropShadowEffect;
    bodyShadow->setBlurRadius(4.0);
    bodyShadow->setColor(m_shadowColor);
    bodyShadow->setOffset(2.0, 4.0);
    this->setGraphicsEffect(bodyShadow);

    connect(this, &Cover::shadowColorChanged, this, [ = ](QColor shadowColor) {
        bodyShadow->setColor(shadowColor);
    });
}

void Cover::paintEvent(QPaintEvent *e)
{
    auto radius = m_radius;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::HighQualityAntialiasing);

    QRect windowRect = QWidget::rect();

    painter.drawPixmap(0, 0, m_Background);

    QPainterPath border;
    border.addRoundedRect(windowRect, radius, radius);

    QPen borderPen(m_borderColor, 1);
    painter.strokePath(border, borderPen);

    QWidget::paintEvent(e);
}

void Cover::setBackgroundImage(const QPixmap &backgroundPixmap)
{
    int radius = m_radius;
    QSize sz = size();
    QPainter::CompositionMode mode = QPainter::CompositionMode_SourceIn;

    QPixmap destinationImage(sz);
    destinationImage.fill(Qt::transparent);
    QPainterPath path;
    path.addRoundRect(QRectF(0, 0, sz.width(), sz.height()), radius);
    QPainter bkPainter(&destinationImage);
    bkPainter.setRenderHint(QPainter::Antialiasing);
    bkPainter.setPen(QPen(Qt::white, 1));
    bkPainter.fillPath(path, QBrush(Qt::red));

    QPixmap backgroundImage = backgroundPixmap.scaled(sz, Qt::KeepAspectRatioByExpanding);

    QImage resultImage = QImage(sz, QImage::Format_ARGB32_Premultiplied);
    QPainter painter(&resultImage);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.fillRect(resultImage.rect(), Qt::transparent);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    painter.drawImage(0, 0, destinationImage.toImage());
    painter.setCompositionMode(mode);
    painter.drawImage(0, 0, backgroundImage.toImage());
    painter.setCompositionMode(QPainter::CompositionMode_DestinationOver);
    painter.end();

    m_Background = QPixmap::fromImage(resultImage);
}
