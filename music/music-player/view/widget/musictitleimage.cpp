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

#include "musictitleimage.h"

#include <QPainter>
#include <QPainterPath>

MusicTitleImageWidget::MusicTitleImageWidget(QWidget *parent)
    : DWidget( parent)
{
}

MusicTitleImageWidget::~MusicTitleImageWidget()
{
}

void MusicTitleImageWidget::setPixmap(QPixmap pixmap)
{
    image = pixmap;
}

void MusicTitleImageWidget::setThemeType(int type)
{
    this->type = type;
}

void MusicTitleImageWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::HighQualityAntialiasing | QPainter::SmoothPixmapTransform);

    QRect rect = this->rect();
    QPainterPath path;
    int cornerSize = 20;
    int arcR = cornerSize / 2;
    path.moveTo(rect.left() + arcR, rect.top());
    path.arcTo(rect.left(), rect.top(), cornerSize, cornerSize, 90.0, 90.0);

    path.lineTo(rect.left(), rect.bottom());
    path.lineTo(rect.right(), rect.bottom());

    path.lineTo(rect.right(), rect.top() + arcR);
    path.arcTo(rect.right() - cornerSize, rect.top(), cornerSize, cornerSize, 0.0, 90.0);

    path.lineTo(rect.top(), rect.left());
//    QPainterPath path;
//    path.addRoundRect(rect, 10, 10);

    painter.save();
    painter.setClipPath(path);

    double windowScale = (width() * 1.0) / height();
    int imageWidth = static_cast<int>(image.height() * windowScale);
    QPixmap coverImage;
    if (imageWidth > image.width()) {
        int imageheight = static_cast<int>(image.width() / windowScale);
        coverImage = image.copy(0, (image.height() - imageheight) / 2, image.width(), imageheight);
    } else {
        int imageheight = image.height();
        coverImage = image.copy((image.width() - imageWidth) / 2, 0, imageWidth, imageheight);
    }
    painter.drawPixmap(rect, coverImage);

    QColor fillColor("#FFFFFF");
    if (type != 1) {
        fillColor = QColor("#000000");
    }
    fillColor.setAlphaF(0.6);
    painter.setBrush(fillColor);

    QColor penColor("#000000");
    penColor.setAlphaF(0.08);
    QPen pen(penColor, 2);
    painter.setPen(pen);
//    painter.drawRoundRect(rect, 10, 10);
    painter.drawPath(path);

    painter.restore();
}

