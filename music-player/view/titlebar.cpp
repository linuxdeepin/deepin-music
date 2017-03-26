/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "titlebar.h"

#include <QDebug>
#include <QPainter>

#include <thememanager.h>

#include "mainframe.h"

DWIDGET_USE_NAMESPACE

class TitlebarPrivate
{
public:
    TitlebarPrivate(Titlebar *parent) : q_ptr(parent)
    {
        QLinearGradient linearGradient(QPointF(0.0, 0.0), QPointF(0.0, 1.0));
        linearGradient.setColorAt(0.0, QColor(255, 255, 255, 255));
        linearGradient.setColorAt(1.0,  QColor(0xf8, 0xf8, 0xf8, 255));
        titleBackground = QBrush(linearGradient);

        borderShadowTop =  QColor(0.1 * 255,  0.1 * 255,  0.1 * 255);
    }

    void workaroundSetButtonStyle();
    void workaroundUpdateButtonViewname();

    QBrush          titleBackground;
    QColor          borderBottom;
    QColor          borderShadowTop;
    QString         viewname;

    Titlebar *q_ptr;
    Q_DECLARE_PUBLIC(Titlebar)
};

Titlebar::Titlebar(QWidget *parent) : DTitlebar(parent), d_ptr(new TitlebarPrivate(this))
{
    Q_D(Titlebar);
    ThemeManager::instance()->regisetrWidget(this, QStringList() << s_PropertyViewname);

    d->workaroundSetButtonStyle();
}

Titlebar::~Titlebar()
{

}

QBrush Titlebar::background() const
{
    Q_D(const Titlebar);
    return d->titleBackground;
}

QColor Titlebar::borderBottom() const
{
    Q_D(const Titlebar);
    return d->borderBottom;
}

QColor Titlebar::borderShadowTop() const
{
    Q_D(const Titlebar);
    return d->borderShadowTop;
}

QString Titlebar::viewname() const
{
    Q_D(const Titlebar);
    return  d->viewname;
}

void Titlebar::setBackground(QBrush titleBackground)
{
    Q_D(Titlebar);
    d->titleBackground = titleBackground;
}

void Titlebar::setBorderBottom(QColor borderBottom)
{
    Q_D(Titlebar);
    d->borderBottom = borderBottom;
}

void Titlebar::setBorderShadowTop(QColor borderShadowTop)
{
    Q_D(Titlebar);
    d->borderShadowTop = borderShadowTop;
}

void Titlebar::setViewname(QString viewname)

{
    Q_D(Titlebar);
    if (d->viewname == viewname) {
        return;
    }

    d->viewname = viewname;
    d->workaroundUpdateButtonViewname();
    emit viewnameChanged(viewname);
}


void Titlebar::paintEvent(QPaintEvent *)
{
    Q_D(const Titlebar);

    auto radius = 0;
    QPainter titlePainter(this);
    titlePainter.setRenderHint(QPainter::Antialiasing);
    titlePainter.setRenderHint(QPainter::HighQualityAntialiasing);

    auto titleBarHeight = this->height() ;
    QRectF winRect = rect();
    QPointF titleTopLeft(winRect.x(), winRect.y());

    QRectF topLeftRect(titleTopLeft,
                       QPoint(winRect.x() + 2 * radius, winRect.y() + 2 * radius));
    QRectF topRightRect(QPoint(winRect.x() + winRect.width(), winRect.y()),
                        QPoint(winRect.x() + winRect.width() - 2 * radius,
                               winRect.y() + 2 * radius));

    QPainterPath titleBorder;
    titleBorder.moveTo(winRect.x() + radius, winRect.y());
    titleBorder.lineTo(winRect.x() + winRect.width() - radius, winRect.y());
    titleBorder.arcTo(topRightRect, 90.0, 90.0);
    titleBorder.lineTo(winRect.x() + winRect.width(), winRect.y() + radius);
    titleBorder.lineTo(winRect.x() + winRect.width(), winRect.y() + titleBarHeight);
    titleBorder.lineTo(winRect.x(), winRect.y() + titleBarHeight);
    titleBorder.lineTo(winRect.x() , winRect.y() + radius);
    titleBorder.arcTo(topLeftRect, 180.0, -90.0);
    titleBorder.closeSubpath();

    titlePainter.fillPath(titleBorder, QBrush(d->titleBackground));
    QLine line(titleTopLeft.x(), winRect.y() + titleBarHeight ,
               winRect.x() + winRect.width(), winRect.y() + titleBarHeight);

    titlePainter.setPen(QPen(d->borderBottom, 1.0));
    titlePainter.drawLine(line);

    QLine lineOut(titleTopLeft.x()+radius, winRect.y() +  1,
                  winRect.x() + winRect.width()-radius, winRect.y() + 1);
    titlePainter.setPen(QPen(d->borderShadowTop, 1.0));
    titlePainter.drawLine(lineOut);
}

void TitlebarPrivate::workaroundSetButtonStyle()
{
    Q_Q(Titlebar);

    QStringList objNames;
    objNames  << "DTitlebarDWindowMinButton"
              << "DTitlebarDWindowMaxButton"
              << "DTitlebarDWindowCloseButton"
              << "DTitlebarDWindowOptionButton";

    for (auto &objname : objNames) {
        auto maxBt = q->findChild<QWidget *>(objname);
        if (!maxBt) {
            continue;
        }
        maxBt->setProperty("_d_QSSFilename", "Titlebar");
        ThemeManager::instance()->regisetrWidget(maxBt);
    }
}

void TitlebarPrivate::workaroundUpdateButtonViewname()
{
    Q_Q(Titlebar);

    QStringList objNames;
    objNames  << "DTitlebarDWindowMinButton"
              << "DTitlebarDWindowMaxButton"
              << "DTitlebarDWindowCloseButton"
              << "DTitlebarDWindowOptionButton";

    for (auto &objname : objNames) {
        auto maxBt = q->findChild<QWidget *>(objname);
        if (!maxBt) {
            continue;
        }
        maxBt->setProperty("viewname", viewname);
    }
}
