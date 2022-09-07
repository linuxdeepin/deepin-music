// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "musicpixmapbutton.h"

#include <QDebug>
#include <QPainter>
#include <QRect>
#include <QPainterPath>

MusicPixmapButton::MusicPixmapButton(QWidget *parent)
    : DPushButton(parent)
{
}

void MusicPixmapButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::HighQualityAntialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    QPixmap pixmap = icon().pixmap(rect().width(), rect().height());
    //pixmap.setDevicePixelRatio(devicePixelRatioF());

    QPainterPath backgroundPath;
    backgroundPath.addRoundedRect(rect(), 8, 8);
    painter.setClipPath(backgroundPath);
    painter.drawPixmap(rect(), pixmap);

    painter.setBrush(Qt::NoBrush);
    QColor BorderColor("000000");
    BorderColor.setAlphaF(0.08);
    QPen BorderPen(BorderColor);
    BorderPen.setWidthF(2);
    painter.setPen(BorderPen);
    painter.drawRoundedRect(rect(), 8, 8); //画矩形

    painter.restore();
}

