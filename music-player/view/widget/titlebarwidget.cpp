/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "titlebarwidget.h"

#include <QDebug>
#include <QLabel>
#include <QHBoxLayout>
#include <QPushButton>
#include <QResizeEvent>
#include <QApplication>
#include <QFocusEvent>
#include <QGraphicsOpacityEffect>

#include <dutility.h>
#include <thememanager.h>
#include <dsearchedit.h>

#include "searchedit.h"

DWIDGET_USE_NAMESPACE

class TitleBarWidgetPrivate
{
public:
    TitleBarWidgetPrivate(TitleBarWidget *parent) : q_ptr(parent) {}

    void fixSearchPosition();

    SearchEdit  *search = nullptr;
    QPushButton *btBack = nullptr;

    TitleBarWidget *q_ptr;
    Q_DECLARE_PUBLIC(TitleBarWidget)
};

TitleBarWidget::TitleBarWidget(QWidget *parent) :
    QFrame(parent), d_ptr(new TitleBarWidgetPrivate(this))
{
    Q_D(TitleBarWidget);
    setFocusPolicy(Qt::NoFocus);
    setObjectName("TitleBarWidget");

    ThemeManager::instance()->regisetrWidget(this);

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
    iconLabel->setFixedSize(24, 24);

    d->btBack = new QPushButton;
    d->btBack->setObjectName("TitleBack");
    d->btBack->setFixedSize(24, 24);
    d->btBack->hide();

    leftLayout->addWidget(iconLabel, 0, Qt::AlignCenter);
    leftLayout->addWidget(d->btBack, 0, Qt::AlignCenter);
    leftLayout->addStretch();

    d->search = new SearchEdit(this);
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

    connect(d->search, &SearchEdit::locateMusic, this, &TitleBarWidget::locateMusicInAllMusiclist);
    connect(d->search, &SearchEdit::searchText, this, &TitleBarWidget::search);
    connect(d->btBack, &QPushButton::clicked, this, &TitleBarWidget::searchExited);

    connect(this, &TitleBarWidget::search, this, [ = ]() {
        d->btBack->show();
//        d->fixSearchPosition();
    });
    connect(this, &TitleBarWidget::searchExited, this, [ = ]() {
        d->btBack->hide();
        clearSearch();
//        d->fixSearchPosition();
    });
}

TitleBarWidget::~TitleBarWidget()
{
}

void TitleBarWidget::exitSearch()
{
    Q_D(TitleBarWidget);
    d->btBack->hide();
    clearSearch();
}

void TitleBarWidget::clearSearch()
{
    Q_D(TitleBarWidget);
    d->search->clear();
    auto edit = d->search->findChild<QWidget *>("Edit");
    if (edit) {
        QApplication::postEvent(edit, new QFocusEvent(QEvent::FocusOut, Qt::MouseFocusReason));
    }
}

void TitleBarWidget::setSearchEnable(bool enable)
{
    Q_D(TitleBarWidget);
    d->search->setEnabled(enable);
}

void TitleBarWidget::setResultWidget(SearchResult *r)
{
    Q_D(TitleBarWidget);
    d->search->setResultWidget(r);
}

void TitleBarWidget::setViewname(const QString &viewname)
{
    Q_D(TitleBarWidget);
    d->search->setViewname(viewname);
}

void TitleBarWidget::resizeEvent(QResizeEvent *event)
{
//    Q_D(TitleBarWidget);
    QFrame::resizeEvent(event);
}

void TitleBarWidgetPrivate::fixSearchPosition()
{
    Q_Q(TitleBarWidget);
    auto fixSize = QPoint(search->width() / 2, search->height() / 2);
    auto fixPos = q->geometry().center() - fixSize;
    search->setGeometry(fixPos.x(), fixPos.y(),
                        search->width(), search->height());

    qDebug() << fixPos << search->size() << search->parent();
}
