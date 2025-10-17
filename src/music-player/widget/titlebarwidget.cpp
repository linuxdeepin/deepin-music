// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
#include <DApplication>

#include "searchedit.h"

#include "ac-desktop-define.h"
#include "commonservice.h"

DWIDGET_USE_NAMESPACE


TitlebarWidget::TitlebarWidget(QWidget *parent) :
    DWidget(parent)
{
    setFocusPolicy(Qt::NoFocus);
    setObjectName("TitlebarWidget");

    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    m_search = new SearchEdit(this);
    m_search->setMainWindow(parent);
    m_search->setObjectName("TitleSearch");
    m_search->setFixedWidth(354);
    m_search->setPlaceHolder(tr("Search"));
    layout->addWidget(m_search, Qt::AlignCenter);
    QObject::connect(m_search, &SearchEdit::sigFocusIn,
                     this, &TitlebarWidget::slotSearchEditFocusIn);
    connect(CommonService::getInstance(), &CommonService::signalClearEdit, this, &TitlebarWidget::slotClearEdit);
}

TitlebarWidget::~TitlebarWidget()
{

}

//void TitlebarWidget::setResultWidget(SearchResult *result)
//{
//    m_search->setResultWidget(result);
//}

void TitlebarWidget::slotSearchEditFocusIn()
{
    //emit sigSearchEditFocusIn();
    m_search->lineEdit()->setFocus();
}

void TitlebarWidget::slotClearEdit()
{
    if (m_search->text().isEmpty())
        return;
    m_search->clear();
    auto edit = m_search->findChild<QLineEdit *>();
    if (edit) {
        QApplication::postEvent(edit, new QFocusEvent(QEvent::FocusOut, Qt::MouseFocusReason));
    }
}

void TitlebarWidget::resizeEvent(QResizeEvent *event)
{
    DWidget::resizeEvent(event);
}

