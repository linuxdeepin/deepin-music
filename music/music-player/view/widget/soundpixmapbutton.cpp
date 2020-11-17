/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
*
* Author:     Iceyer<Iceyer@uniontech.com>
* Maintainer: Iceyer <Iceyer@uniontech.com>
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "soundpixmapbutton.h"

#include <QDebug>
#include <QPainter>
#include <QRect>
#include <QPainterPath>

SoundPixmapButton::SoundPixmapButton(QWidget *parent)
    : DPushButton(parent)
{


}

void SoundPixmapButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::HighQualityAntialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    QPixmap pixmap = icon().pixmap(rect().width(), rect().height());

    QPainterPath backgroundPath;
    backgroundPath.addRoundedRect(rect(), 8, 8);
    painter.setClipPath(backgroundPath);
    painter.drawPixmap(rect(), pixmap);
#if 0
    painter.setBrush(Qt::NoBrush);
    QColor BorderColor("000000");
    BorderColor.setAlphaF(0.08);
    QPen BorderPen(BorderColor);
    BorderPen.setWidthF(2);
    painter.setPen(BorderPen);
    painter.drawRoundedRect(rect(), 8, 8); //画矩形
#endif
    painter.restore();
}

