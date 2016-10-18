/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "playerframe.h"

#include <QDebug>
#include <QVBoxLayout>
#include <QListWidgetItem>
#include <QStackedWidget>
#include <QPropertyAnimation>
#include <QTimer>
#include <QResizeEvent>

#include <dutility.h>
#include <dthememanager.h>

#include "titlebar.h"
#include "footer.h"
#include "importwidget.h"
#include "lyricview.h"
#include "playlistwidget.h"
#include "playlistitem.h"
#include "musiclistwidget.h"

#include "../model/musiclistmodel.h"

DWIDGET_USE_NAMESPACE

class PlayerFramePrivate
{
public:
    QStackedWidget  *stacked    = nullptr;
    ImportWidget    *import     = nullptr;
    MusicListWidget *musicList  = nullptr;
};


static void slideWidget(QWidget *left, QWidget *right)
{
    right->show();
    left->show();
    right->resize(left->size());

    int delay = 800;
    QRect leftStart = QRect(0, 0, left->width(), left->height());
    QRect leftEnd = leftStart;
    leftEnd.moveLeft(-left->width());

    QPropertyAnimation *animation = new QPropertyAnimation(left, "geometry");
    animation->setDuration(delay);
    animation->setEasingCurve(QEasingCurve::OutBounce);
    animation->setStartValue(leftStart);
    animation->setEndValue(leftEnd);
    animation->start();

    QRect rightStart = QRect(left->width(), 0, right->width(), right->height());
    QRect rightEnd = rightStart;
    rightEnd.moveRight(left->width() - 1);

    QPropertyAnimation *animation2 = new QPropertyAnimation(right, "geometry");
    animation2->setEasingCurve(QEasingCurve::OutBounce);
    animation2->setDuration(delay);
    animation2->setStartValue(rightStart);
    animation2->setEndValue(rightEnd);
    animation2->start();

    animation->connect(animation, &QPropertyAnimation::finished,
                       animation, &QPropertyAnimation::deleteLater);
    animation2->connect(animation2, &QPropertyAnimation::finished,
                        animation2, &QPropertyAnimation::deleteLater);
    animation2->connect(animation2, &QPropertyAnimation::finished,
                        left, &QWidget::hide);

}


PlayerFrame::PlayerFrame(QWidget *parent)
    : DWindow(parent), d(new PlayerFramePrivate)
{
    setFixedSize(960, 720);

    auto title = new TitleBar;
    setTitlebarWidget(title);

    auto contentWidget = new QWidget;
    contentWidget->setFixedSize(958, 720 - 40);
    contentWidget->setObjectName("BackgroundWidget");
    auto contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setMargin(0);
    contentLayout->setSpacing(0);

    auto footer = new Footer;
    footer->setFixedHeight(60);

    d->import = new ImportWidget;

    d->musicList = new MusicListWidget;

    auto lv = new LyricView;
    lv->hide();
    auto plw = new PlayListWidget;
    plw->hide();
    plw->setFixedWidth(220);

    for (int i = 0; i < 20; ++i) {
        auto item2 = new QListWidgetItem;
        plw->addItem(item2);
        plw->setItemWidget(item2, new PlayListItem);
    }

    d->stacked = new QStackedWidget;
    d->stacked->setObjectName("ContentWidget");
    d->stacked->addWidget(d->import);
    d->stacked->addWidget(d->musicList);
    d->stacked->addWidget(lv);
    d->stacked->addWidget(plw);

    contentLayout->addWidget(d->stacked);
    contentLayout->addWidget(footer);

    setContentWidget(contentWidget);

    contentWidget->resize(960-2, 720 - 40);

    D_THEME_INIT_WIDGET(PlayerFrame);
}

PlayerFrame::~PlayerFrame()
{

}

MusicListWidget *PlayerFrame::musicList()
{
    return d->musicList;
}

void PlayerFrame::resizeEvent(QResizeEvent *event)
{
//    qDebug() << event << d->stacked->size() << d->musicList->size();
    DWindow::resizeEvent(event);
}

void PlayerFrame::onMusicListChanged(const MusicListInfo &list)
{
    qDebug() << list.list.length();
    if (!list.list.isEmpty()) {
        slideWidget(d->musicList, d->import);
    } else {
        d->musicList->onMusicListChanged(list);
        slideWidget(d->import, d->musicList);
    }
}
