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
#include <QPushButton>
#include <QGraphicsOpacityEffect>

#include <dutility.h>
#include <dthememanager.h>
#include <dsearchedit.h>

#include "widget/searchedit.h"

DWIDGET_USE_NAMESPACE

class TitleBarPrivate
{
public:
    TitleBarPrivate(TitleBar *parent) : q_ptr(parent) {}

    void fixSearchPosition();

    SearchEdit *search;

    TitleBar *q_ptr;
    Q_DECLARE_PUBLIC(TitleBar);
};

TitleBar::TitleBar(QWidget *parent) :
    QFrame(parent), d_ptr(new TitleBarPrivate(this))
{
    Q_D(TitleBar);

    setFocusPolicy(Qt::NoFocus);
    setObjectName("TitleBar");

    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(5, 5, 10, 5);

    auto leftWidget = new QFrame;
    leftWidget->setObjectName("TitleLeft");
    leftWidget->setFixedWidth(148);
    auto leftLayout = new QHBoxLayout(leftWidget);
    leftLayout->setSpacing(10);
    leftLayout->setMargin(0);

    auto iconLabel = new QLabel;
    iconLabel->setObjectName("TitleIcon");
    iconLabel->setFixedSize(20, 20);

    auto btBack = new QPushButton;
    btBack->setObjectName("TitleBack");
    btBack->setFixedSize(24, 24);
    btBack->hide();

    leftLayout->addWidget(iconLabel, 0, Qt::AlignCenter);
    leftLayout->addWidget(btBack, 0, Qt::AlignCenter);
    leftLayout->addStretch();

    d->search = new SearchEdit();
    d->search->setObjectName("TitleSearch");
    d->search->setFixedSize(278, 26);
    d->search->setPlaceHolder(tr("Search"));
    d->search->clear();

    auto rightWidget = new QFrame;
    rightWidget->setObjectName("TitleLeft");
    rightWidget->setFixedWidth(1);

    layout->addWidget(leftWidget, 0,  Qt::AlignCenter);
    layout->addStretch();
    layout->addWidget(d->search, 0,  Qt::AlignCenter);
    layout->addStretch();
    layout->addWidget(rightWidget, 0,  Qt::AlignCenter);

    auto result = this->findChild<QWidget *>("DEditInsideFrame");
    if (result) {
        result->setStyleSheet("#DEditInsideFrame{background: rgba(255,255,255,0.3);}");
    }

    connect(d->search, &SearchEdit::locateMusic, this, &TitleBar::locateMusicInAllMusiclist);
    connect(d->search, &SearchEdit::searchText, this, &TitleBar::search);
    connect(btBack, &QPushButton::clicked, this, &TitleBar::exitSearch);

    connect(this, &TitleBar::search, this, [ = ]() {
        btBack->show();
//        d->fixSearchPosition();
    });
    connect(this, &TitleBar::exitSearch, this, [ = ]() {
        btBack->hide();
//        d->fixSearchPosition();
    });

    D_THEME_INIT_WIDGET(TitleBar);
}

TitleBar::~TitleBar()
{

}
#include <QResizeEvent>
void TitleBar::resizeEvent(QResizeEvent *event)
{
    Q_D(TitleBar);
//    QFrame::resizeEvent(event);
//    setFixedSize(event->size());
//    d->fixSearchPosition();
}

void TitleBarPrivate::fixSearchPosition()
{
    Q_Q(TitleBar);
    auto fixSize = QPoint(search->width() / 2, search->height() / 2);
    auto fixPos = q->geometry().center() - fixSize;
    search->setGeometry(fixPos.x(), fixPos.y(),
                        search->width(), search->height());

    qDebug() << fixPos << search->size() << search->parent();
}
