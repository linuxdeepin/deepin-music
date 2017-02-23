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

static auto borderPenWidth = 1.0;

class CoverPrivate
{
public:
    CoverPrivate(Cover *parent): q_ptr(parent) {}

    int     m_radius        = 4;
    QColor  m_borderColor;
    QColor  m_shadowColor;
    QPixmap m_Background;

    QMarginsF outterMargins = QMarginsF(borderPenWidth, borderPenWidth, borderPenWidth, borderPenWidth);

    Cover *q_ptr;
    Q_DECLARE_PUBLIC(Cover)
};

Cover::Cover(QWidget *parent)
    : Label("", parent), d_ptr(new CoverPrivate(this))
{
    Q_D(Cover);

    QWidget::setAttribute(Qt::WA_TranslucentBackground, true);
    d->m_radius = 4;
    d->m_borderColor = QColor(255, 0, 0, 152);
    d->m_shadowColor = QColor(0, 255, 0, 126);

//    QGraphicsDropShadowEffect *bodyShadow = new QGraphicsDropShadowEffect;
//    bodyShadow->setBlurRadius(4.0);
//    bodyShadow->setColor(d->m_shadowColor);
//    bodyShadow->setOffset(2.0, 4.0);
//    this->setGraphicsEffect(bodyShadow);

//    connect(this, &Cover::shadowColorChanged, this, [ = ](QColor shadowColor) {
//        bodyShadow->setColor(shadowColor);
//    });
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

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::HighQualityAntialiasing);
    auto radius = d->m_radius;
    auto innerBorderColor = d->m_borderColor;
    auto outerBorderColor = d->m_shadowColor;
    auto backgroundColor = QColor(255, 255, 255, 255);
    auto penWidthf = borderPenWidth;

    // draw background
    auto backgroundRect = QRectF(rect()).marginsRemoved(d->outterMargins);
    if (d->m_Background.isNull()) {
        QPainterPath backgroundPath;
        backgroundPath.addRoundedRect(backgroundRect, radius, radius);
        painter.fillPath(backgroundPath, backgroundColor);
    } else {
        painter.drawPixmap(backgroundRect.toRect(), d->m_Background);
    }

    // draw border
    QPainterPath innerBorderPath;
    QRectF borderRect = QRectF(rect()).marginsRemoved(d->outterMargins);
    auto borderRadius = radius;
    QMarginsF borderMargin(penWidthf / 2, penWidthf / 2, penWidthf / 2, penWidthf / 2);
    borderRadius -= penWidthf / 2;
    borderRect = borderRect.marginsRemoved(borderMargin);
    innerBorderPath.addRoundedRect(borderRect, borderRadius, borderRadius);
    QPen innerBorderPen(innerBorderColor);
    innerBorderPen.setWidthF(penWidthf);
    painter.strokePath(innerBorderPath, innerBorderPen);

    QPainterPath outerBorderPath;
    borderRect = QRectF(rect()).marginsRemoved(d->outterMargins);
    borderRadius = radius;
    borderRadius += penWidthf / 2;
    borderRect = borderRect.marginsAdded(borderMargin);

    outerBorderPath.addRoundedRect(borderRect, borderRadius, borderRadius);
    QPen outerBorderPen(outerBorderColor);
    outerBorderPen.setWidthF(penWidthf);
    painter.strokePath(outerBorderPath, outerBorderPen);
}

void Cover::setCoverPixmap(const QPixmap &pixmap)
{
    Q_D(Cover);
    int radius = d->m_radius;

    auto coverRect = QRectF(rect()).marginsRemoved(d->outterMargins);
    auto sz = coverRect.size().toSize();
    QPixmap backgroundPixmap = pixmap.scaled(sz, Qt::KeepAspectRatioByExpanding);

    QPixmap maskPixmap(sz);
    maskPixmap.fill(Qt::transparent);
    QPainterPath path;
    path.addRoundedRect(QRectF(0, 0, sz.width(), sz.height()), double(radius), double(radius));
    QPainter bkPainter(&maskPixmap);
    bkPainter.setRenderHints(QPainter::Antialiasing | QPainter::HighQualityAntialiasing);
    bkPainter.fillPath(path, QBrush(Qt::red));

    QPainter::CompositionMode mode = QPainter::CompositionMode_SourceIn;
    QImage resultImage = QImage(sz, QImage::Format_ARGB32_Premultiplied);
    QPainter painter(&resultImage);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.fillRect(resultImage.rect(), Qt::transparent);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    painter.drawImage(0, 0, maskPixmap.toImage());
    painter.setCompositionMode(mode);
    painter.drawImage(0, 0, backgroundPixmap.toImage());
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
