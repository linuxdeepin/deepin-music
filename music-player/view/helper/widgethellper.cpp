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
#include <QGraphicsBlurEffect>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QPainter>
#include <QWidget>
#include <QStyleFactory>

QT_BEGIN_NAMESPACE
extern Q_WIDGETS_EXPORT void qt_blurImage(QPainter *p, QImage &blurImage, qreal radius, bool quality, bool alphaOnly, int transposed = 0);
QT_END_NAMESPACE

namespace WidgetHelper
{

QImage applyEffectToImage(const QImage src, QGraphicsEffect *effect, int extent = 0)
{
    if (src.isNull()) { return QImage(); }
    if (!effect) { return src; }
    QGraphicsScene scene;
    QGraphicsPixmapItem item;
    item.setPixmap(QPixmap::fromImage(src));
    item.setGraphicsEffect(effect);
    scene.addItem(&item);
    QImage res(src.size() + QSize(extent * 2, extent * 2), QImage::Format_ARGB32);
    res.fill(Qt::transparent);
    QPainter ptr(&res);
    scene.render(&ptr, QRectF(),
                 QRectF(-extent, -extent, src.width() + extent * 2,
                        src.height() + extent * 2));
    return res;
}

QPixmap blurImage(const QImage &image, int radius)
{
    QGraphicsBlurEffect *blur = new QGraphicsBlurEffect;
    blur->setBlurRadius(radius);
    QImage result = applyEffectToImage(image, blur);
    auto cropFactor = 0.9;
    QRect rect((1 - cropFactor) / 2 * result.width() , (1 - cropFactor) / 2 * result.height(),
               result.width() *cropFactor,
               result.height() *cropFactor);
    QImage cropped = result.copy(rect);
    return QPixmap::fromImage(cropped).scaled(image.size());
}

QImage cropRect(const QImage &image, QSize sz)
{
    QImage newImage = image.scaled(sz, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    auto center = QRectF(newImage.rect()).center();
    auto topLeft = QPointF(center.x() - sz.width() / 2.0,
                           center.y() - sz.height() / 2.0);
    if (topLeft.x() < 0) {
        topLeft.setX(0);
    }
    if (topLeft.y() < 0) {
        topLeft.setY(0);
    }

    QRect crop(topLeft.toPoint(), sz);
    return newImage.copy(crop);
}

void slideRight2LeftWidget(QWidget *left, QWidget *right, int delay)
{
    right->show();
    left->show();
    right->resize(left->size());

    QRect leftStart = QRect(0, 0, left->width(), left->height());
    QRect leftEnd = leftStart;
    leftEnd.moveLeft(-left->width());

    QPropertyAnimation *animation = new QPropertyAnimation(left, "geometry");
    animation->setDuration(delay);
    animation->setEasingCurve(QEasingCurve::InOutCubic);
    animation->setStartValue(leftStart);
    animation->setEndValue(leftEnd);
    animation->start();

    QRect rightStart = QRect(left->width(), 0, right->width(), right->height());
    QRect rightEnd = rightStart;
    rightEnd.moveRight(left->width() - 1);

    QPropertyAnimation *animation2 = new QPropertyAnimation(right, "geometry");
    animation2->setEasingCurve(QEasingCurve::InOutCubic);
    animation2->setDuration(delay);
    animation2->setStartValue(rightStart);
    animation2->setEndValue(rightEnd);
    animation2->start();

    animation->connect(animation, &QPropertyAnimation::finished,
                       animation, &QPropertyAnimation::deleteLater);
    animation2->connect(animation2, &QPropertyAnimation::finished,
                        animation2, &QPropertyAnimation::deleteLater);
    animation2->connect(animation2, &QPropertyAnimation::finished,
                        left, &QWidget::hide);
}

void slideBottom2TopWidget(QWidget *top, QWidget *bottom, int delay)
{
    bottom->show();
    top->show();

    QRect topStart = QRect(0, 0, top->width(), top->height());
    QRect topEnd = topStart;
    topEnd.moveTop(-top->height());

    QPropertyAnimation *animation = new QPropertyAnimation(top, "geometry");
    animation->setDuration(delay);
    animation->setEasingCurve(QEasingCurve::InOutCubic);
    animation->setStartValue(topStart);
    animation->setEndValue(topEnd);
    animation->start();
    animation->connect(animation, &QPropertyAnimation::finished,
                       animation, &QPropertyAnimation::deleteLater);
    animation->connect(animation, &QPropertyAnimation::finished,
                       top, &QWidget::hide);

    QRect bottomStart = QRect(0, top->height(), bottom->width(), bottom->height());
    QRect bottomEnd = bottomStart;
    bottomEnd.moveTo(0, 0);

    QPropertyAnimation *animation2 = new QPropertyAnimation(bottom, "geometry");
    animation2->setEasingCurve(QEasingCurve::InOutCubic);
    animation2->setDuration(delay);
    animation2->setStartValue(bottomStart);
    animation2->setEndValue(bottomEnd);
    animation2->start();
    animation2->connect(animation2, &QPropertyAnimation::finished,
                        animation2, &QPropertyAnimation::deleteLater);

    auto bottomOpacity = new QGraphicsOpacityEffect(bottom);
    bottom->setGraphicsEffect(bottomOpacity);
    bottomOpacity->setOpacity(0);
    QPropertyAnimation *animation3 = new QPropertyAnimation(bottomOpacity, "opacity");
    animation3->setEasingCurve(QEasingCurve::InCubic);
    animation3->setDuration(delay);
    animation3->setStartValue(0.0);
    animation3->setEndValue(1.0);
    animation3->start();
    animation3->connect(animation3, &QPropertyAnimation::finished,
                        animation3, &QPropertyAnimation::deleteLater);
    animation3->connect(animation3, &QPropertyAnimation::finished,
    bottom, [ = ]() {
        bottom->setGraphicsEffect(nullptr);
    });


    auto topOpacity = new QGraphicsOpacityEffect();
    top->setGraphicsEffect(topOpacity);
    topOpacity->setOpacity(1);
    QPropertyAnimation *animation4 = new QPropertyAnimation(topOpacity, "opacity");
    animation4->setEasingCurve(QEasingCurve::InCubic);
    animation4->setDuration(delay);
    animation4->setStartValue(1.0);
    animation4->setEndValue(0.0);
    animation4->start();
    animation4->connect(animation4, &QPropertyAnimation::finished,
                        animation4, &QPropertyAnimation::deleteLater);
    animation4->connect(animation4, &QPropertyAnimation::finished,
    top, [ = ]() {
        top->setGraphicsEffect(nullptr);
    });
}

void slideTop2BottomWidget(QWidget *top, QWidget *bottom, int delay)
{
    bottom->show();
    top->show();
    bottom->resize(top->size());

    QRect topStart = QRect(0, 0, top->width(), top->height());
    QRect topEnd = topStart;
    topEnd.moveTo(0, top->height());

    QPropertyAnimation *animation = new QPropertyAnimation(top, "geometry");
    animation->setDuration(delay);
    animation->setEasingCurve(QEasingCurve::InOutCubic);
    animation->setStartValue(topStart);
    animation->setEndValue(topEnd);
    animation->start();
    animation->connect(animation, &QPropertyAnimation::finished,
                       animation, &QPropertyAnimation::deleteLater);
    animation->connect(animation, &QPropertyAnimation::finished,
                       top, &QWidget::hide);

    QRect bottomStart = QRect(0, -top->height(), bottom->width(), bottom->height());
    QRect bottomEnd = bottomStart;
    bottomEnd.moveBottom(top->height() - 1);

    QPropertyAnimation *animation2 = new QPropertyAnimation(bottom, "geometry");
    animation2->setEasingCurve(QEasingCurve::InOutCubic);
    animation2->setDuration(delay);
    animation2->setStartValue(bottomStart);
    animation2->setEndValue(bottomEnd);
    animation2->start();
    animation2->connect(animation2, &QPropertyAnimation::finished,
                        animation2, &QPropertyAnimation::deleteLater);

    auto bottomOpacity = new QGraphicsOpacityEffect(bottom);
    bottom->setGraphicsEffect(bottomOpacity);
    bottomOpacity->setOpacity(0);

    QPropertyAnimation *animation3 = new QPropertyAnimation(bottomOpacity, "opacity");
    animation3->setEasingCurve(QEasingCurve::InCubic);
    animation3->setDuration(delay);
    animation3->setStartValue(0.0);
    animation3->setEndValue(1.0);
    animation3->start();
    animation3->connect(animation3, &QPropertyAnimation::finished,
                        animation3, &QPropertyAnimation::deleteLater);
    animation3->connect(animation3, &QPropertyAnimation::finished,
    bottom, [ = ]() {
        bottom->setGraphicsEffect(nullptr);
    });


    auto topOpacity = new QGraphicsOpacityEffect(top);
    top->setGraphicsEffect(topOpacity);
    topOpacity->setOpacity(0.99);

    QPropertyAnimation *animation4 = new QPropertyAnimation(topOpacity, "opacity");
    animation4->setEasingCurve(QEasingCurve::InCubic);
    animation4->setDuration(delay);
    animation4->setStartValue(1.0);
    animation4->setEndValue(0.0);
    animation4->start();
    animation4->connect(animation4, &QPropertyAnimation::finished,
                        animation4, &QPropertyAnimation::deleteLater);
    animation4->connect(animation4, &QPropertyAnimation::finished,
    bottom, [ = ]() {
        top->setGraphicsEffect(nullptr);
    });
}

void slideEdgeWidget(QWidget *right, QRect start, QRect end, int delay, bool hide)
{
    right->show();

    QPropertyAnimation *animation2 = new QPropertyAnimation(right, "geometry");
    animation2->setEasingCurve(QEasingCurve::InCurve);
    animation2->setDuration(delay);
    animation2->setStartValue(start);
    animation2->setEndValue(end);
    animation2->start();
    animation2->connect(animation2, &QPropertyAnimation::finished,
                        animation2, &QPropertyAnimation::deleteLater);
    if (hide)
        animation2->connect(animation2, &QPropertyAnimation::finished,
                            right, &QWidget::hide);

}

QPixmap coverPixmap(const QString &coverPath, QSize sz)
{
    return QPixmap::fromImage(cropRect(QImage(coverPath), sz));
}

void workaround_updateStyle(QWidget *parent, const QString &theme)
{
    parent->setStyle(QStyleFactory::create(theme));
    for (auto obj : parent->children()) {
        auto w = qobject_cast<QWidget *>(obj);
        if (!w) {
            continue;
        }

//        qDebug() << "update widget style" << w << theme;
        workaround_updateStyle(w, theme);
    }

}

}
