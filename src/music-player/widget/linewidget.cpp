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

#include "linewidget.h"

#include <QDebug>
#include <QHBoxLayout>
#include <QResizeEvent>
#include <QApplication>
#include <QFocusEvent>
#include <QGraphicsOpacityEffect>

#include <DWidgetUtil>
#include <DPushButton>
#include <DLabel>
#include <DHorizontalLine>
#include <DFontSizeManager>
#include <DGuiApplicationHelper>

#include "label.h"

DWIDGET_USE_NAMESPACE


LineWidget::LineWidget(QWidget *parent) :
    DWidget(parent)
{
    setFocusPolicy(Qt::NoFocus);
    setObjectName("LineWidget");
}

LineWidget::~LineWidget()
{
}

void LineWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::HighQualityAntialiasing);
    QBrush brush(painter.pen().color());
    painter.setBrush(brush);
    painter.drawRect(0, 0, this->width(), this->height());
    QWidget::paintEvent(event);
}
