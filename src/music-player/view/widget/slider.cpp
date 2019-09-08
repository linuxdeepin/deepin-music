/*
 * Copyright (C) 2016 ~ 2018 Wuhan Deepin Technology Co., Ltd.
 *
 * Author:     Iceyer <me@iceyer.net>
 *
 * Maintainer: Iceyer <me@iceyer.net>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "slider.h"

#include <QDebug>

#include <QMouseEvent>
#include <QVariantAnimation>

class SliderPrivate
{
public:
    SliderPrivate(Slider *parent) : q_ptr(parent) {}

    QVariantAnimation *currentAnimation = Q_NULLPTR;
    QString themeTemplate;

    QString updateQSS(qreal value);

    Slider *q_ptr;
    Q_DECLARE_PUBLIC(Slider)
};

Slider::Slider(Qt::Orientation orientation, QWidget *parent) :
    DSlider(orientation, parent), dd_ptr(new SliderPrivate(this))
{
    Q_D(Slider);

    connect(this, &Slider::hoverd, this, [ = ](bool hovered) {
        if (d->currentAnimation) {
            d->currentAnimation->stop();
            d->currentAnimation->deleteLater();
            d->currentAnimation = Q_NULLPTR;
        }

        d->currentAnimation = new QVariantAnimation;
        d->currentAnimation->setStartValue(hovered ? 0.0 : 1.0);
        d->currentAnimation->setEndValue(hovered ? 1.0 : 0.0);
        d->currentAnimation->setDuration(150);

        connect(d->currentAnimation, &QVariantAnimation::valueChanged,
        this, [ = ](const QVariant & value) {
            setStyleSheet(d->updateQSS(value.toReal()));
        });
        d->currentAnimation->start();
    });
}

Slider::~Slider()
{

}

void Slider::resizeEvent(QResizeEvent *event)
{
    Q_D(Slider);
    DSlider::resizeEvent(event);
    if (!d->currentAnimation) {
        setStyleSheet(d->updateQSS(0));
    }
}

void Slider::mouseReleaseEvent(QMouseEvent *event)
{
    this->blockSignals(false);
    DSlider::mouseReleaseEvent(event);
    Q_EMIT valueAccpet(value());
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
}

void Slider::mouseMoveEvent(QMouseEvent *event)
{
    auto valueRange = this->maximum()  - this->minimum();
    auto viewRange = this->width();

    if (0 == viewRange) {
        return;
    }

    auto value = (event->x() - this->x()) * valueRange / this->width();
    setSliderPosition(value);
}

void Slider::enterEvent(QEvent *event)
{
    setProperty("hover", true);
    Q_EMIT hoverd(true);
    DSlider::enterEvent(event);
}

void Slider::leaveEvent(QEvent *event)
{
    setProperty("hover", false);
    Q_EMIT hoverd(false);
    DSlider::leaveEvent(event);
}

void Slider::wheelEvent(QWheelEvent *e)
{
    e->accept();
}

QString SliderPrivate::updateQSS(qreal value)
{
    Q_Q(Slider);
    auto delta = 0.00001;
    qreal h = q->height();
    qreal realH = h * (1.0 + value) / 2.0 ;
    Q_EMIT q->realHeightChanged(realH);
    auto v2 = (h - realH) / h;
    auto v1 = v2 - delta;
    v1 = (v1 < 0) ? 0 : v1;
    auto v4 = (h - 1.0) / h;
    auto v3 = v4 - delta;
    auto v6 = (h - realH + 1.0) / h;
    auto v5 = v6 - delta;
    auto v7 = v4 - delta;
//    qDebug() << themeTemplate.arg(v1).arg(v2).arg(v3).arg(v4).arg(v5).arg(v6).arg(v7);
    return themeTemplate.arg(v1).arg(v2).arg(v3).arg(v4).arg(v5).arg(v6).arg(v7);

}
