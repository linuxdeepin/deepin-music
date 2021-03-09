/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     ZouYa <zouya@uniontech.com>
 *
 * Maintainer: WangYu <wangyu@uniontech.com>
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

#include "cover.h"

#include <QDebug>
#include <QPainter>
#include <QGraphicsDropShadowEffect>
#include <QPainterPath>

static auto borderPenWidth = 1.0;

Cover::Cover(QWidget *parent)
    : Label("", parent)
{
    QWidget::setAttribute(Qt::WA_TranslucentBackground, true);
    m_borderColor = QColor(0, 0, 0);
    m_borderColor.setAlphaF(0.08);
    m_shadowColor = QColor(0, 255, 0, 126);
    outterMargins = QMarginsF(borderPenWidth, borderPenWidth, borderPenWidth, borderPenWidth);

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

void Cover::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::HighQualityAntialiasing | QPainter::SmoothPixmapTransform);
    auto radius = m_radius;
    auto innerBorderColor = m_borderColor;
    //auto outerBorderColor = d->m_shadowColor;
    auto backgroundColor = QColor(255, 255, 255, 255);
    //auto penWidthf = borderPenWidth;

    // draw background
    //auto backgroundRect = QRectF(rect()).marginsRemoved(d->outterMargins);
    auto backgroundRect = rect();
    QPainterPath backgroundPath;
    backgroundPath.addRoundedRect(backgroundRect, radius, radius);
    painter.setClipPath(backgroundPath);
    painter.setPen(Qt::NoPen);
    if (m_Background.isNull()) {
        painter.fillPath(backgroundPath, backgroundColor);
    } else {
        painter.drawPixmap(backgroundRect, m_Background);
    }

    painter.setBrush(Qt::NoBrush);
    QPen BorderPen(innerBorderColor);
    BorderPen.setWidthF(2);
    painter.setPen(BorderPen);
    painter.drawRoundedRect(backgroundRect, radius, radius); //画矩形

    // draw border
//    QPainterPath innerBorderPath;
//    QRectF borderRect = QRectF(rect()).marginsRemoved(d->outterMargins);
//    auto borderRadius = radius;
//    QMarginsF borderMargin(penWidthf / 2, penWidthf / 2, penWidthf / 2, penWidthf / 2);
//    borderRadius -= penWidthf / 2;
//    borderRect = borderRect.marginsRemoved(borderMargin);
//    innerBorderPath.addRoundedRect(borderRect, borderRadius, borderRadius);
//    QPen innerBorderPen(innerBorderColor);
//    innerBorderPen.setWidthF(penWidthf / 2);
//    painter.strokePath(innerBorderPath, innerBorderPen);

//    QPainterPath outerBorderPath;
//    borderRect = QRectF(rect()).marginsRemoved(d->outterMargins);
//    borderRadius = radius;
//    borderRadius += penWidthf / 2;
//    borderRect = borderRect.marginsAdded(borderMargin);

//    outerBorderPath.addRoundedRect(borderRect, borderRadius, borderRadius);
//    QPen outerBorderPen(outerBorderColor);
//    outerBorderPen.setWidthF(penWidthf / 2);
//    painter.strokePath(outerBorderPath, outerBorderPen);
}

void Cover::setCoverPixmap(const QPixmap &pixmap)
{
    int radius = m_radius;

    auto ratio = this->devicePixelRatioF();
    auto coverRect = QRectF(rect()).marginsRemoved(outterMargins);
    auto sz = coverRect.size().toSize() * ratio;

    // 添加非空判断，减少警告日志输出与不必要的性能损耗
    QPixmap backgroundPixmap;
    if (!pixmap.isNull()) {
        backgroundPixmap = pixmap.scaled(sz, Qt::KeepAspectRatioByExpanding);
    }
    // TODO: fix ratio by Dtk::Widget::DHiDPIHelper
    backgroundPixmap.setDevicePixelRatio(1);
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

    m_Background = QPixmap::fromImage(resultImage);
}
