// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
