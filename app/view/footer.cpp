/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "footer.h"

#include <QStyle>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>

#include <dthememanager.h>
DWIDGET_USE_NAMESPACE

#include "../musicapp.h"

Footer::Footer(QWidget *parent) : QFrame(parent)
{
    setObjectName("Footer");

    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(10, 10, 20, 10);
    layout->setSpacing(20);

    auto cover = new QLabel;
    cover->setObjectName("FooterCover");
    cover->setFixedSize(40, 40);

    auto title = new QLabel;
    title->setObjectName("FooterTitle");
    title->setMaximumWidth(240);
    title->setText(tr("Unknow Title TitleTitleTitleTitleTitleTitleTitleTitleTitleTitleTitleTitleTitle"));

    auto artlist = new QLabel;
    artlist->setObjectName("FooterArtlist");
    artlist->setMaximumWidth(240);
    artlist->setText(tr("Unknow Artlist"));

    auto btPlay = new QPushButton;
    btPlay->setObjectName("FooterActionPlay");
    btPlay->setFixedSize(30, 30);

    auto btPrev = new QPushButton;
    btPrev->setObjectName("FooterActionPrev");
    btPrev->setFixedSize(30, 30);

    auto btNext = new QPushButton;
    btNext->setObjectName("FooterActionNext");
    btNext->setFixedSize(30, 30);

    auto btFavorite = new QPushButton;
    btFavorite->setObjectName("FooterActionFavorite");
    btFavorite->setFixedSize(24, 24);

    auto btLyric = new QPushButton;
    btLyric->setObjectName("FooterActionLyric");
    btLyric->setFixedSize(24, 24);

    auto btPlayMode = new QPushButton;
    btPlayMode->setObjectName("FooterActionPlayMode");
    btPlayMode->setFixedSize(24, 24);

    auto btSound = new QPushButton;
    btSound->setObjectName("FooterActionSound");
    btSound->setFixedSize(24, 24);

    auto btPlayList = new QPushButton;
    btPlayList->setObjectName("FooterActionPlayList");
    btPlayList->setFixedSize(24, 24);

    auto infoWidget = new QWidget;
    auto infoLayout = new QHBoxLayout(infoWidget);
    auto musicMetaLayout = new QVBoxLayout;
    musicMetaLayout->addWidget(title);
    musicMetaLayout->addWidget(artlist);
    musicMetaLayout->setSpacing(0);
    infoLayout->setMargin(0);
    infoLayout->addWidget(cover, 0, Qt::AlignLeft | Qt::AlignVCenter);
    infoLayout->addLayout(musicMetaLayout, 0);

    auto ctlWidget = new QWidget;
    auto ctlLayout = new QHBoxLayout(ctlWidget);
    ctlLayout->setMargin(0);
    ctlLayout->setSpacing(30);
    ctlLayout->addWidget(btPrev, 0, Qt::AlignCenter);
    ctlLayout->addWidget(btPlay, 0, Qt::AlignCenter);
    ctlLayout->addWidget(btNext, 0, Qt::AlignCenter);

    auto actWidget = new QWidget;
    auto actLayout = new QHBoxLayout(actWidget);
    actLayout->setMargin(0);
    actLayout->setSpacing(20);
    actLayout->addWidget(btFavorite, 0, Qt::AlignRight | Qt::AlignVCenter);
    actLayout->addWidget(btLyric, 0, Qt::AlignRight | Qt::AlignVCenter);
    actLayout->addWidget(btPlayMode, 0, Qt::AlignRight | Qt::AlignVCenter);
    actLayout->addWidget(btSound, 0, Qt::AlignRight | Qt::AlignVCenter);
    actLayout->addWidget(btPlayList, 0, Qt::AlignRight | Qt::AlignVCenter);


    QSizePolicy sp(QSizePolicy::Preferred, QSizePolicy::Preferred);
    sp.setHorizontalStretch(33);
    infoWidget->setSizePolicy(sp);
    ctlWidget->setSizePolicy(sp);
    actWidget->setSizePolicy(sp);

    layout->addWidget(infoWidget, 0, Qt::AlignLeft | Qt::AlignVCenter);
    layout->addStretch();
    layout->addWidget(ctlWidget, 0, Qt::AlignCenter);
    layout->addStretch();
    layout->addWidget(actWidget, 0, Qt::AlignRight | Qt::AlignVCenter);

    title->hide();
    artlist->hide();
    btPrev->hide();
    btNext->hide();
    btFavorite->hide();
    btLyric->hide();

    D_THEME_INIT_WIDGET(Footer);

    connect(MusicApp::presenter(), &AppPresenter::musicPlayed,
    this, [ = ](const MusicInfo & info) {
        title->setText(info.title);
        artlist->setText(info.artist);

        title->show();
        artlist->show();
        btPrev->show();
        btNext->show();
        btFavorite->show();
        btLyric->show();

        this->setProperty("playstatus", "active");
        this->style()->unpolish(this);
        this->style()->polish(this);
    });

    connect(btPlayList, &QPushButton::clicked, this, [ = ](bool) {
        emit  MusicApp::presenter()->showPlaylist();
    });
}

void Footer::onMusicPlay(const MusicInfo &info)
{

}
