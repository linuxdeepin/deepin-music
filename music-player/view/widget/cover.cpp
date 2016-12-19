/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "cover.h"

#include <QDebug>
#include <QPainter>
#include <QGraphicsDropShadowEffect>

class CoverPrivate
{
public:
    CoverPrivate(Cover *parent): q_ptr(parent) {}

    int     m_radius;
    QColor  m_borderColor;
    QColor  m_shadowColor;
    QString m_backgroundUrl;
    QPixmap m_Background;

    Cover *q_ptr;
    Q_DECLARE_PUBLIC(Cover);
};

Cover::Cover(QWidget *parent)
    : Label("", parent), d_ptr(new CoverPrivate(this))
{
    Q_D(Cover);

    QWidget::setAttribute(Qt::WA_TranslucentBackground, true);
    d->m_radius = 4;
    d->m_borderColor = QColor(0, 0, 0, 52);
    d->m_shadowColor = QColor(0, 0, 0, 26);

    QGraphicsDropShadowEffect *bodyShadow = new QGraphicsDropShadowEffect;
    bodyShadow->setBlurRadius(4.0);
    bodyShadow->setColor(d->m_shadowColor);
    bodyShadow->setOffset(2.0, 4.0);
    this->setGraphicsEffect(bodyShadow);

    connect(this, &Cover::shadowColorChanged, this, [ = ](QColor shadowColor) {
        bodyShadow->setColor(shadowColor);
    });
}

Cover::~Cover()
{

}

int Cover::radius() const
{
    Q_D(const Cover);
    return  d->m_radius ;
}

QColor Cover::borderColor() const
{
    Q_D(const Cover);
    return  d->m_borderColor ;
}

QColor Cover::shadowColor() const
{
    Q_D(const Cover);
    return  d->m_shadowColor ;
}

void Cover::paintEvent(QPaintEvent *e)
{
    Q_D(const Cover);
    auto radius = d->m_radius;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::HighQualityAntialiasing);

    QRect windowRect = QWidget::rect();

    painter.drawPixmap(0, 0, d->m_Background);

    QPainterPath border;
    border.addRoundedRect(windowRect, radius, radius);

    QPen borderPen(d->m_borderColor, 1);
    painter.strokePath(border, borderPen);

    QWidget::paintEvent(e);
}

void Cover::setCoverPixmap(const QPixmap &pixmap)
{
    Q_D(Cover);
    int radius = d->m_radius;
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

    QPixmap backgroundImage = pixmap.scaled(sz, Qt::KeepAspectRatioByExpanding);

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

    d->m_Background = QPixmap::fromImage(resultImage);
}

void Cover::setRadius(int radius)
{
    Q_D(Cover);
    d->m_radius = radius;
}

void Cover::setBorderColor(QColor borderColor)
{
    Q_D(Cover);
    d->m_borderColor = borderColor;
}

void Cover::setShadowColor(QColor shadowColor)
{
    Q_D(Cover);
    d->m_shadowColor = shadowColor;
}
