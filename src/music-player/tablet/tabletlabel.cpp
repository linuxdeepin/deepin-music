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
