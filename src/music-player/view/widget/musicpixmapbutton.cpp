/*
 * Copyright (C) 2016 ~ 2018 Wuhan Deepin Technology Co., Ltd.
 *
 * Author:     yub.wang <yub.wang@deepin.io>
 *
 * Maintainer: yub.wang <yub.wang@deepin.io>
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

#include "musicpixmapbutton.h"

#include <QDebug>
#include <QPainter>
#include <QRect>

MusicPixmapButton::MusicPixmapButton(QWidget *parent)
    : DPushButton(parent)
{
}

void MusicPixmapButton::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.save();

    QPainterPath backgroundPath;
    backgroundPath.addRoundedRect(rect(), 10, 10);
    painter.setClipPath(backgroundPath);
    painter.drawPixmap(rect(), icon().pixmap(rect().width(), rect().height()));

    painter.restore();
}

