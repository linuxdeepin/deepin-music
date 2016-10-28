/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "widgethellper.h"

#include <QMatrix3x3>
#include <QImage>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QPainter>
#include <QGraphicsBlurEffect>

QT_BEGIN_NAMESPACE
extern Q_WIDGETS_EXPORT void qt_blurImage(QPainter *p, QImage &blurImage, qreal radius, bool quality, bool alphaOnly, int transposed = 0);
QT_END_NAMESPACE

namespace WidgetHellper
{

QImage applyEffectToImage(const QImage src, QGraphicsEffect *effect, int extent = 0)
{
    if (src.isNull()) { return QImage(); }  //No need to do anything else!
    if (!effect) { return src; }            //No need to do anything else!
    QGraphicsScene scene;
    QGraphicsPixmapItem item;
    item.setPixmap(QPixmap::fromImage(src));
    item.setGraphicsEffect(effect);
    scene.addItem(&item);
    QImage res(src.size() + QSize(extent * 2, extent * 2), QImage::Format_ARGB32);
    res.fill(Qt::transparent);
    QPainter ptr(&res);
    scene.render(&ptr, QRectF(), QRectF(-extent, -extent, src.width() + extent * 2, src.height() + extent * 2));
    return res;
}

QPixmap blurImage(const QImage &image, int radius)
{
    QGraphicsBlurEffect *blur = new QGraphicsBlurEffect;
    blur->setBlurRadius(radius);
    QImage result = applyEffectToImage(image, blur);
    int cropWidth = 20;
    QRect rect(cropWidth, cropWidth,
               result.width() - cropWidth * 2,
               result.height() - cropWidth * 2);
    QImage cropped = result.copy(rect);
    return QPixmap::fromImage(cropped);
}


};
