/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "titlebar.h"

#include <QDebug>
#include <QLabel>
#include <QHBoxLayout>

#include <dthememanager.h>
#include <dsearchedit.h>

DWIDGET_USE_NAMESPACE

TitleBar::TitleBar(QWidget *parent) : QFrame(parent)
{
    setFocusPolicy(Qt::NoFocus);
    setObjectName("TitleBar");

    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(10, 5, 10, 5);

    auto iconLabel = new QLabel;
    iconLabel->setObjectName("TitleIcon");
    iconLabel->setFixedSize(20, 20);

    auto search = new SearchEdit;
    search->setObjectName("TitleSearch");
    search->setFixedSize(278, 26);
    search->setPlaceHolder(tr("Search"));
    search->clear();

    layout->addWidget(iconLabel, 0, Qt::AlignHCenter);
    layout->addStretch();
    layout->addWidget(search, 0, Qt::AlignHCenter);
    layout->addStretch();

    auto result = this->findChild<QWidget *>("DEditInsideFrame");
    if (result) {
        result->setStyleSheet("#DEditInsideFrame{background: rgba(255,255,255,0.3);}");
    }

    connect(search, &SearchEdit::focusOut,
            search, &SearchEdit::onFocusOut);
    connect(search, &SearchEdit::focusIn,
            search, &SearchEdit::onFocusIn);
    D_THEME_INIT_WIDGET(TitleBar);
}

SearchEdit::SearchEdit(QWidget *parent) : DSearchEdit(parent)
{
    m_result = new SearchResult();
    m_result->setFixedWidth(278);
    m_result->hide();
}

void SearchEdit::onFocusIn()
{
    m_result->adjustSize();
    auto pos = this->mapToGlobal(QPoint(0, this->height() + 2));
    m_result->show();
    m_result->move(pos);
}

void SearchEdit::onFocusOut()
{
    m_result->hide();
    m_result->close();
}
