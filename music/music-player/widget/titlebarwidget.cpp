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

#include "titlebarwidget.h"

#include <QDebug>
#include <QHBoxLayout>
#include <QResizeEvent>
#include <QApplication>
#include <QFocusEvent>
#include <QGraphicsOpacityEffect>

#include <DWidgetUtil>
#include <DPushButton>
#include <DSearchEdit>
#include <DLabel>

#include "searchedit.h"

#include "ac-desktop-define.h"

DWIDGET_USE_NAMESPACE


TitlebarWidget::TitlebarWidget(QWidget *parent) :
    DWidget(parent)
{
    setFocusPolicy(Qt::NoFocus);
    setObjectName("TitlebarWidget");

    auto layout = new QHBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    m_search = new SearchEdit(this);
    m_search->setObjectName("TitleSearch");
    m_search->setFixedWidth(354);
    m_search->setPlaceHolder(tr("Search"));
    m_search->clear();
    layout->addWidget(m_search, Qt::AlignCenter);
    QObject::connect(m_search, &SearchEdit::sigFoucusIn,
                     this, &TitlebarWidget::slotSearchEditFoucusIn);
}

TitlebarWidget::~TitlebarWidget()
{

}

void TitlebarWidget::setResultWidget(SearchResult *result)
{
    m_search->setResultWidget(result);
}

void TitlebarWidget::slotSearchEditFoucusIn()
{
    //emit sigSearchEditFoucusIn();
    m_search->lineEdit()->setFocus();
}

void TitlebarWidget::resizeEvent(QResizeEvent *event)
{
    DWidget::resizeEvent(event);
}

