/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "thinwindow.h"

#include <QDebug>
#include <QMouseEvent>
#include <QStyle>
#include <QPainter>
#include <QHBoxLayout>

#include "../helper/xutil.h"

#include <dthememanager.h>
DWIDGET_USE_NAMESPACE;

const int WindowHandleWidth = 10;

class ThinWindowPrivate
{
public:
    ThinWindowPrivate(ThinWindow *parent) : q_ptr(parent) {}

    void setBackgroundImage(const QPixmap &srcPixmap);

    QWidget         *contentWidget = nullptr;
    QBrush          background;
    int             radius              = 4;
    int             shadowWidth         = 20;
    QMargins        shadowMargins       = QMargins(20, 20, 20, 20);
    int             resizeHandleWidth   = WindowHandleWidth;
    bool            resizable           = true;
    QColor          borderColor         = QColor(0, 0, 0, 0.2 * 255);
    QPixmap         backgrounpImage     = QPixmap();

    XUtils::CornerEdge resizingCornerEdge = XUtils::CornerEdge::kInvalid;

    ThinWindow *q_ptr;
    Q_DECLARE_PUBLIC(ThinWindow);
};


void ThinWindowPrivate::setBackgroundImage(const QPixmap &srcPixmap)
{
    Q_Q(ThinWindow);

    QRect windowRect = q->rect();
    QSize sz = windowRect.size();
    QPixmap backgroundPixmap = srcPixmap.scaled(sz, Qt::KeepAspectRatioByExpanding);

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

    backgrounpImage = QPixmap::fromImage(resultImage);
}


ThinWindow::ThinWindow(QWidget *parent) : QWidget(parent), d_ptr(new ThinWindowPrivate(this))
{
    Q_D(ThinWindow);

    QWidget::setMouseTracking(true);
    QWidget::setAttribute(Qt::WA_TranslucentBackground, true);
    QWidget::setWindowFlags(Qt::Window | Qt::FramelessWindowHint);

    auto layout = new QBoxLayout(QBoxLayout::Down, this);
    layout->setContentsMargins(d->shadowMargins);

    d->contentWidget = new QWidget(this);
    layout->addWidget(d->contentWidget);
    auto filter = new FilterMouseMove(d->contentWidget);
    d->contentWidget->installEventFilter(filter);
    filter->m_rootWidget = this;
    D_THEME_INIT_WIDGET(Widget / ThinWindow);

#ifdef Q_OS_LINUX
    XUtils::SetMouseTransparent(this, true);
#endif
}

ThinWindow::~ThinWindow()
{
}

void ThinWindow::setContentLayout(QLayout *layout)
{
    Q_D(ThinWindow);
    d->contentWidget->setLayout(layout);
}

void ThinWindow::resize(QSize sz)
{
    Q_D(ThinWindow);
    sz = QRect(QPoint(0, 0), sz).marginsAdded(d->shadowMargins).size();
    QWidget::resize(sz);
}

QSize ThinWindow::size() const
{
    Q_D(const ThinWindow);
    qDebug() << rect().size();
    return rect().size();
}

QRect ThinWindow::rect() const
{
    Q_D(const ThinWindow);
    return QWidget::rect().marginsRemoved(d->shadowMargins);
}

QBrush ThinWindow::background() const
{
    Q_D(const ThinWindow);
    return d->background;
}

int ThinWindow::radius() const
{
    Q_D(const ThinWindow);
    return d->radius;
}

QColor ThinWindow::borderColor() const
{
    Q_D(const ThinWindow);
    return d->borderColor;
}

void ThinWindow::showMinimized()
{
#ifdef Q_OS_LINUX
    XUtils::ShowMinimizedWindow(this, true);
#endif
    QWidget::showMinimized();
}

void ThinWindow::showMaximized()
{
#ifdef Q_OS_LINUX
    XUtils::ShowMaximizedWindow(this);
#endif
    this->show();
    this->activateWindow();
    this->raise();
}

void ThinWindow::showFullScreen()
{
#ifdef Q_OS_LINUX
    XUtils::ShowFullscreenWindow(this, true);
#endif
    this->show();
    this->activateWindow();
    this->raise();
}

void ThinWindow::showNormal()
{
#ifdef Q_OS_LINUX
    XUtils::ShowNormalWindow(this);
#endif
}

void ThinWindow::moveWindow(Qt::MouseButton botton)
{
#ifdef Q_OS_LINUX
    XUtils::MoveWindow(this, botton);
#endif
}

void ThinWindow::toggleMaximizedWindow()
{
#ifdef Q_OS_LINUX
    XUtils::ToggleMaximizedWindow(this);
#endif
}

void ThinWindow::setBackgroundImage(QPixmap pixmap)
{
    Q_D(ThinWindow);
    d->setBackgroundImage(pixmap);
}

void ThinWindow::setBackground(QBrush background)
{
    Q_D(ThinWindow);
    d->background = background;
}

void ThinWindow::setRadius(int radius)
{
    Q_D(ThinWindow);
    d->radius = radius;
}

void ThinWindow::setBorderColor(QColor borderColor)
{
    Q_D(ThinWindow);
    d->borderColor = borderColor;
}

void ThinWindow::mouseMoveEvent(QMouseEvent *event)
{
#ifdef Q_OS_LINUX
    Q_D(ThinWindow);

    const int x = event->x();
    const int y = event->y();

//    qDebug() << int(d->resizingCornerEdge) << d->resizable;
    if (d->resizingCornerEdge == XUtils::CornerEdge::kInvalid && d->resizable) {
        XUtils::UpdateCursorShape(this, x, y, d->shadowMargins, d->resizeHandleWidth);
    }
#endif

    return QWidget::mouseMoveEvent(event);
}

void ThinWindow::mousePressEvent(QMouseEvent *event)
{
#ifdef Q_OS_LINUX
    Q_D(ThinWindow);

    const int x = event->x();
    const int y = event->y();
    if (event->button() == Qt::LeftButton) {
        const XUtils::CornerEdge ce = XUtils::GetCornerEdge(this, x, y,
                                      d->shadowMargins,
                                      d->resizeHandleWidth);
        if (ce != XUtils::CornerEdge::kInvalid) {
            d->resizingCornerEdge = ce;
            XUtils::StartResizing(this, QCursor::pos(), ce);
        }

    }
#endif
    return QWidget::mousePressEvent(event);
}

void ThinWindow::mouseReleaseEvent(QMouseEvent *event)
{
#ifdef Q_OS_LINUX
    Q_D(ThinWindow);
    d->resizingCornerEdge = XUtils::CornerEdge::kInvalid;
#endif
    return QWidget::mouseReleaseEvent(event);
}

void ThinWindow::resizeEvent(QResizeEvent *e)
{
    Q_D(ThinWindow);
#ifdef Q_OS_LINUX
    auto resizeHandleWidth = d->resizable ? d->resizeHandleWidth : 0;
    XUtils::SetWindowExtents(this, d->shadowMargins, resizeHandleWidth);
#endif
    QWidget::resizeEvent(e);
}

void ThinWindow::paintEvent(QPaintEvent *)
{
    Q_D(const ThinWindow);

    bool outer = true;

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::HighQualityAntialiasing);
    auto radius = d->radius;
    auto penWidthf = 1.0;

//    QPainterPath frame;
//    frame.addRect(rect().marginsRemoved(QMargins(1, 1, 1, 1)));
//    painter.strokePath(frame, QPen(Qt::red));

    // draw background
    auto backgroundRect = QRectF(rect());
    if (d->backgrounpImage.isNull()) {
        QPainterPath backgroundPath;
        backgroundPath.addRoundedRect(backgroundRect, radius, radius);
        painter.fillPath(backgroundPath, d->background);
    } else {
        painter.drawPixmap(rect(), d->backgrounpImage);
    }

    // draw border
    QPainterPath borderPath;
    QRectF borderRect = QRectF(rect());
    auto borderRadius = radius;
    QMarginsF borderMargin(penWidthf / 2, penWidthf / 2, penWidthf / 2, penWidthf / 2);
    if (outer) {
        borderRadius += penWidthf / 2;
        borderRect = borderRect.marginsAdded(borderMargin);
    } else {
        borderRadius -= penWidthf / 2;
        borderRect = borderRect.marginsRemoved(borderMargin);
    }
    borderPath.addRoundedRect(borderRect, borderRadius, borderRadius);
    QPen borderPen(d->borderColor);
    borderPen.setWidthF(penWidthf);
    painter.strokePath(borderPath, borderPen);
}


FilterMouseMove::FilterMouseMove(QObject *object) : QObject(object)
{

}

FilterMouseMove::~FilterMouseMove()
{

}

bool FilterMouseMove::eventFilter(QObject *obj, QEvent *event)
{
    switch (event->type()) {
    case QEvent::Enter: {
        if (qobject_cast<QWidget *>(obj) != qobject_cast<QWidget *>(this->parent())) {
            break;
        }
        if (m_rootWidget) {
#ifdef Q_OS_LINUX
            XUtils::ResetCursorShape(m_rootWidget);
#endif
        }
        break;
    }
    default: {
    }
    }
    return QObject::eventFilter(obj, event);
}
