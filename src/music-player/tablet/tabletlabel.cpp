// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tabletlabel.h"
#include <QPainter>
#include <QTextOption>

#include <DGuiApplicationHelper>
#include <DHiDPIHelper>
#include <DStyle>
#include <DFontSizeManager>

DWIDGET_USE_NAMESPACE

TabletLabel::TabletLabel(const QString &text, QWidget *parent, int val):
    QLabel(text, parent), m_valprop(val)
{

}

void TabletLabel::mousePressEvent(QMouseEvent *event)
{
    switch (m_valprop) {
    case 0:
        emit signalTabletDone();
        break;
    case 1:
        emit signalTabletSelectAll();
        break;
    default:
        emit signalTabletDone();
    }
    QLabel::mousePressEvent(event);
}

void TabletLabel::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    QFont fontT6 = DFontSizeManager::instance()->get(DFontSizeManager::T6);
    QColor txtcolor = QColor(DGuiApplicationHelper::instance()->applicationPalette().highlight().color());
    painter.setPen(txtcolor);
    painter.setFont(fontT6);
    QTextOption option;
    option.setAlignment(Qt::AlignVCenter);
    painter.drawText(this->rect(), this->text(), option);
}
