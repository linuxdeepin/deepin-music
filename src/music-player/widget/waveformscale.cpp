// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "waveformscale.h"

#include <DGuiApplicationHelper>
#include <DFontSizeManager>

#include <QDebug>
#include <QEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QPainterPath>

#include <mediameta.h>

DGUI_USE_NAMESPACE

const int WaveformScale::WAVE_WIDTH = 2;
const int WaveformScale::WAVE_TEXTHEIGHT = 25;
const int WaveformScale::WAVE_TEXTDEFAULTWIDTH = 38;
const int WaveformScale::WAVE_OFFSET = 13;// 整体向下偏移

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

    QFont fontT8 = DFontSizeManager::instance()->get(DFontSizeManager::T8);
    QFontMetrics timeFm(fontT8);
    QString timeStr = DMusic::lengthString(curValue);
    int timeWidth = timeFm.width(timeStr) + 5;

    //draw line
    painter.setBrush(QColor("#FF8A00"));
    QRectF lineRect(rect().center().x() - WAVE_WIDTH / 2, WAVE_TEXTHEIGHT + WAVE_OFFSET, WAVE_WIDTH, rect().height() - WAVE_TEXTHEIGHT - 20);
    painter.drawRect(lineRect);
    // 进度图标颜色跟随系统活动色
    QColor color = DGuiApplicationHelper::instance()->applicationPalette().highlight().color();
    painter.setBrush(color);
    //draw top
    int t_textWidth = 0;
    if (timeWidth <= WAVE_TEXTDEFAULTWIDTH) {
        t_textWidth = WAVE_TEXTDEFAULTWIDTH;
    } else {
        t_textWidth = timeWidth;
    }

    QRect t_textRect(rect().center().x() - t_textWidth / 2, 0 + WAVE_OFFSET, t_textWidth, 20);
    QPainterPath painterPath;
    painterPath.addRoundedRect(t_textRect, 8, 8, Qt::AbsoluteSize);
    QPolygonF topPoly;
    topPoly.append(QPointF(rect().center().x() - 5, 20 + WAVE_OFFSET));
    topPoly.append(QPointF(rect().center().x() + 5, 20 + WAVE_OFFSET));
    topPoly.append(QPointF(rect().center().x(), 25 + WAVE_OFFSET));
    painterPath.addPolygon(topPoly);
    painter.drawPath(painterPath);

    //draw bottom
    QPolygonF bottomPoly;
    bottomPoly.append(QPointF(rect().center().x(), rect().height() - 23 + WAVE_OFFSET));
    bottomPoly.append(QPointF(rect().center().x() - 5, rect().height() - 16 + WAVE_OFFSET));
    bottomPoly.append(QPointF(rect().center().x() + 5, rect().height() - 16 + WAVE_OFFSET));
    painter.drawPolygon(bottomPoly);

    painter.setPen(Qt::white);
    fontT8.setFamily("SourceHanSansSC");
    fontT8.setWeight(QFont::Normal);
    painter.setFont(fontT8);
    QTextOption option;
    option.setAlignment(Qt::AlignCenter);
    painter.drawText(t_textRect, timeStr, option);
}
