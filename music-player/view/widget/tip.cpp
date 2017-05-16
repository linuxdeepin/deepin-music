/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "tip.h"

#include <QDebug>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QGraphicsDropShadowEffect>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>

#include <DUtil>

#include <thememanager.h>

class TipPrivate
{
public:
    TipPrivate(Tip *parent) : q_ptr(parent) {}

    void setBackgroundImage(const QPixmap &srcPixmap);

    QBrush          background;
    int             radius              = 4;
    int             shadowWidth         = 20;
    QMargins        shadowMargins       = QMargins(20, 20, 20, 20);
    QColor          borderColor         = QColor(0, 0, 0, 0.2 * 255);

    QLabel          *textLable          = nullptr;
    QFrame          *m_interFrame       = nullptr;


    Tip *q_ptr;
    Q_DECLARE_PUBLIC(Tip)
};


Tip::Tip(const QPixmap &icon, const QString &text, QWidget *parent)
    : QFrame(parent), d_ptr(new TipPrivate(this))
{
    Q_D(Tip);

//    setWindowFlags(Qt::ToolTip | Qt::FramelessWindowHint);
//    setAttribute(Qt::WA_TranslucentBackground);
    setObjectName("Tip");
    setContentsMargins(0, 0, 0, 0);

    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    d->m_interFrame = new QFrame(this);
    d->m_interFrame->setContentsMargins(0, 0, 0, 0);
    auto interlayout = new QHBoxLayout(d->m_interFrame);
    interlayout->setContentsMargins(10, 0, 10, 0);
    interlayout->setSpacing(5);
    auto iconLabel = new QLabel;
    iconLabel->setObjectName("TipIcon");
    iconLabel->setFixedSize(icon.size());
    if (icon.isNull()) {
        iconLabel->hide();
    } else {
        iconLabel->setPixmap(icon);
    }

    d->textLable = new QLabel(text);
    d->textLable->setObjectName("TipText");
    d->textLable->setAlignment(Qt::AlignCenter);

    interlayout->addWidget(iconLabel, 0, Qt::AlignVCenter);
    interlayout->addWidget(d->textLable, 0, Qt::AlignVCenter);
    layout->addWidget(d->m_interFrame, 0, Qt::AlignVCenter);
    ThemeManager::instance()->regisetrWidget(this);

    adjustSize();

    auto *bodyShadow = new QGraphicsDropShadowEffect;
    bodyShadow->setBlurRadius(10.0);
    bodyShadow->setColor(QColor(0, 0, 0, 0.1 * 255));
    bodyShadow->setOffset(0, 2.0);
    this->setGraphicsEffect(bodyShadow);
    hide();

    setFixedHeight(40);
}

Tip::~Tip()
{

}

QBrush Tip::background() const
{
    Q_D(const Tip);
    return d->background;
}

void Tip::setText(const QString text)
{
    Q_D(const Tip);
    d->textLable->setText(text);
}

int Tip::radius() const
{
    Q_D(const Tip);
    return d->radius;
}

QColor Tip::borderColor() const
{
    Q_D(const Tip);
    return d->borderColor;
}

void Tip::setBackground(QBrush background)
{
    Q_D(Tip);
    d->background = background;
}

void Tip::setRadius(int radius)
{
    Q_D(Tip);
    d->radius = radius;
}

void Tip::setBorderColor(QColor borderColor)
{
    Q_D(Tip);
    d->borderColor = borderColor;
}

void Tip::pop(QPoint center)
{
    Q_D(Tip);
    this->show();
    center = center - QPoint(width() / 2, height() / 2);
    this->move(center);

    auto topOpacity = new QGraphicsOpacityEffect(d->m_interFrame);
    topOpacity->setOpacity(1);
    d->m_interFrame->setGraphicsEffect(topOpacity);

    QPropertyAnimation *animation4 = new QPropertyAnimation(topOpacity, "opacity");
//    animation4->setEasingCurve(QEasingCurve::InCubic);
    animation4->setDuration(2000);
    animation4->setStartValue(0);
    animation4->setKeyValueAt(0.25, 1);
    animation4->setKeyValueAt(0.5, 1);
    animation4->setKeyValueAt(0.75, 1);
    animation4->setEndValue(0);
    animation4->start();
    animation4->connect(animation4, &QPropertyAnimation::finished,
                        animation4, &QPropertyAnimation::deleteLater);
    animation4->connect(animation4, &QPropertyAnimation::finished,
    this, [ = ]() {
        d->m_interFrame->setGraphicsEffect(nullptr);
        this->hide();
    });
}

void Tip::paintEvent(QPaintEvent *)
{
    Q_D(Tip);
//    QFrame::paintEvent(e);
//    return;
    bool outer = true;

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::HighQualityAntialiasing);
    auto radius = d->radius;
    auto penWidthf = 1.0;
    auto background =  d->background;
    auto borderColor = d->borderColor;
    auto margin = 2.0;
    auto shadowMargins = QMarginsF(margin, margin, margin, margin);

//    QPainterPath frame;
//    frame.addRect(rect().marginsRemoved(QMargins(1, 1, 1, 1)));
//    painter.strokePath(frame, QPen(Qt::red));

    // draw background
    auto backgroundRect = QRectF(rect()).marginsRemoved(shadowMargins);
    QPainterPath backgroundPath;
    backgroundPath.addRoundedRect(backgroundRect, radius, radius);
    painter.fillPath(backgroundPath, background);

    // draw border
    QPainterPath borderPath;
    QRectF borderRect = QRectF(rect());
    auto borderRadius = radius;
    QMarginsF borderMargin(penWidthf / 2, penWidthf / 2, penWidthf / 2, penWidthf / 2);
    if (outer) {
        borderRadius += penWidthf / 2;
        borderRect = borderRect.marginsAdded(borderMargin).marginsRemoved(shadowMargins);
    } else {
        borderRadius -= penWidthf / 2;
        borderRect = borderRect.marginsRemoved(borderMargin).marginsRemoved(shadowMargins);
    }
    borderPath.addRoundedRect(borderRect, borderRadius, borderRadius);
    QPen borderPen(borderColor);
    borderPen.setWidthF(penWidthf);
    painter.strokePath(borderPath, borderPen);
}
