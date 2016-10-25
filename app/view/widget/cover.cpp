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
    m_borderColor = QColor(0, 0, 0, 26);
    m_shadowColor = QColor(0, 0, 0, 26);

    QGraphicsDropShadowEffect *bodyShadow = new QGraphicsDropShadowEffect;
    bodyShadow->setBlurRadius(4.0);
    bodyShadow->setColor(m_shadowColor);
    bodyShadow->setOffset(0.0, 2.0);
    this->setGraphicsEffect(bodyShadow);
}

void Cover::paintEvent(QPaintEvent * /*e*/)
{
    QPainter painter(this);
    auto radius = 4;
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::HighQualityAntialiasing);

    painter.drawPixmap(0, 0, m_Background);

    QRect windowRect = QWidget::rect();
    QPoint topLeft(windowRect.x(), windowRect.y());
    QPoint bottomRight(windowRect.x() + windowRect.width(), windowRect.y() + windowRect.height());

    QPainterPath border;
    border.addRoundedRect(windowRect, radius, radius);

    QPen borderPen(m_borderColor);
    painter.strokePath(border, borderPen);
}

void Cover::setBackgroundImage(const QPixmap &bk)
{
    QImage bkImage = bk.toImage().scaled(200, 200);
    QPixmap maskPixmap(bkImage.size());
    maskPixmap.fill(Qt::transparent);
    QPainterPath path;
    path.addRoundRect(QRectF(0, 0, bkImage.width(), bkImage.height()), 4);
    QPainter bkPainter(&maskPixmap);
    bkPainter.setRenderHint(QPainter::Antialiasing);
    bkPainter.setPen(QPen(Qt::black, 1));
    bkPainter.fillPath(path, QBrush(Qt::red));

    QImage maskImage = maskPixmap.toImage();
    int nDepth = bkImage.depth();
    int nWidth = maskImage.width();
    int nHeight = maskImage.height();
    switch (nDepth) {
    case 32:
        for (int y = 0; y < nHeight; ++y) {
            quint32 *pMaskData = reinterpret_cast<quint32 *>(maskImage.scanLine(y));
            quint32 *pWaveData = reinterpret_cast<quint32 *>(bkImage.scanLine(y));
            quint32 alpha;
            for (int x = 0; x < nWidth; ++x) {
                alpha = (pMaskData[x] >> 24) << 24 | 0x00FFFFFF;
                pMaskData[x] = pWaveData[x] & (alpha);
            }
        }
        break;
    default:
        break;
    }

    // TODO: fixme , only export as png file can work
    QTemporaryFile bkTmp;
    maskImage.save(&bkTmp, "png");
    bkTmp.close();
    qDebug() << bkTmp.fileName();
    m_Background = QPixmap(bkTmp.fileName());
    bkTmp.remove();
}
