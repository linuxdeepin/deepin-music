/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2011 ~ 2018 Deepin, Inc.
 *               2011 ~ 2018 Wang Yong
 *
 * Author:     Wang Yong <wangyong@deepin.com>
 * Maintainer: Wang Yong <wangyong@deepin.com>
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

#include "waveformscale.h"

#include <QDebug>
#include <QEvent>
#include <QPaintEvent>
#include <QPainter>

#include <mediameta.h>

const int WaveformScale::WAVE_WIDTH = 2;
const int WaveformScale::WAVE_TEXTHEIGHT = 25;
const int WaveformScale::WAVE_TEXTWIDTH = 38;

WaveformScale::WaveformScale(QWidget *parent) : DFrame(parent)
{
    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
    setFixedSize(56, 105);
    setObjectName("WaveformScale");
}

void WaveformScale::setValue(qint64 value)
{
    curValue = value;
    update();
}

void WaveformScale::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(Qt::NoPen);

    //draw line
    painter.setBrush(QColor("#FF8A00"));
    QRectF lineRect(rect().center().x() - WAVE_WIDTH / 2, WAVE_TEXTHEIGHT, WAVE_WIDTH, rect().height() - WAVE_TEXTHEIGHT - 20);
    painter.drawRect(lineRect);

    painter.setBrush(QColor("#0081FF"));
    //draw top
    int t_textWidth = WAVE_TEXTWIDTH;
    QRect t_textRect(rect().center().x() - t_textWidth / 2, 0, t_textWidth, 20);
    QPainterPath painterPath;
    painterPath.addRoundRect(t_textRect, 40, 80);
    QPolygonF topPoly;
    topPoly.append(QPointF(rect().center().x() - 5, 20));
    topPoly.append(QPointF(rect().center().x() + 5, 20));
    topPoly.append(QPointF(rect().center().x(), 25));
    painterPath.addPolygon(topPoly);
    painter.drawPath(painterPath);

    //draw bottom
    QPolygonF bottomPoly;
    bottomPoly.append(QPointF(rect().center().x(), rect().height() - 20));
    bottomPoly.append(QPointF(rect().center().x() - 5, rect().height() - 10));
    bottomPoly.append(QPointF(rect().center().x() + 5, rect().height() - 10));
    painter.drawPolygon(bottomPoly);

    painter.setPen(Qt::white);
    auto font  = painter.font();
    font.setFamily("SourceHanSansSC");
    font.setWeight(QFont::Normal);
    font.setPixelSize(12);
    painter.setFont(font);
    painter.drawText(t_textRect, Qt::AlignCenter, DMusic::lengthString(curValue));
}
