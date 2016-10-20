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
#include <QDebug>

#include <dthememanager.h>
DWIDGET_USE_NAMESPACE

#include "../musicapp.h"
#include "../core/playlist.h"

static const char *sPropertyFavourite         = "fav";
static const char *sPropertyPlayStatus        = "playstatus";

static const QString sPlayStatusValuePlaying    = "playing";
static const QString sPlayStatusValuePause      = "pause";
static const QString sPlayStatusValueStop       = "stop";

class FooterPrivate
{
public:
    QLabel      *cover   = nullptr;
    QLabel      *title   = nullptr;
    QLabel      *artlist = nullptr;
    QPushButton *btPlay  = nullptr;
    QPushButton *btPrev  = nullptr;
    QPushButton *btNext  = nullptr;
    QPushButton *btFavorite = nullptr;
    QPushButton *btLyric    = nullptr;
    QPushButton *btPlayMode = nullptr;
    QPushButton *btSound    = nullptr;


    QSharedPointer<Playlist>    m_playinglist;
    MusicInfo                   m_info;
    QSharedPointer<Playlist>    m_favlist;
    int                         m_mode;
};

Footer::Footer(QWidget *parent) : QFrame(parent)
{
    d = QSharedPointer<FooterPrivate>(new FooterPrivate);
    setObjectName("Footer");

    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(10, 10, 20, 10);
    layout->setSpacing(20);

    d->cover = new QLabel;
    d->cover->setObjectName("FooterCover");
    d->cover->setFixedSize(40, 40);

    d->title = new QLabel;
    d->title->setObjectName("FooterTitle");
    d->title->setMaximumWidth(240);
    d->title->setText(tr("Unknow Title"));

    d->artlist = new QLabel;
    d->artlist->setObjectName("FooterArtlist");
    d->artlist->setMaximumWidth(240);
    d->artlist->setText(tr("Unknow Artlist"));

    d->btPlay = new QPushButton;
    d->btPlay->setObjectName("FooterActionPlay");
    d->btPlay->setFixedSize(30, 30);

    d->btPrev = new QPushButton;
    d->btPrev->setObjectName("FooterActionPrev");
    d->btPrev->setFixedSize(30, 30);

    d->btNext = new QPushButton;
    d->btNext->setObjectName("FooterActionNext");
    d->btNext->setFixedSize(30, 30);

    d->btFavorite = new QPushButton;
    d->btFavorite->setObjectName("FooterActionFavorite");
    d->btFavorite->setFixedSize(24, 24);

    d->btLyric = new QPushButton;
    d->btLyric->setObjectName("FooterActionLyric");
    d->btLyric->setFixedSize(24, 24);

    d->btPlayMode = new QPushButton;
    d->btPlayMode->setObjectName("FooterActionPlayMode");
    d->btPlayMode->setFixedSize(24, 24);

    d->btSound = new QPushButton;
    d->btSound->setObjectName("FooterActionSound");
    d->btSound->setFixedSize(24, 24);

    auto btPlayList = new QPushButton;
    btPlayList->setObjectName("FooterActionPlayList");
    btPlayList->setFixedSize(24, 24);

    auto infoWidget = new QWidget;
    auto infoLayout = new QHBoxLayout(infoWidget);
    auto musicMetaLayout = new QVBoxLayout;
    musicMetaLayout->addWidget(d->title);
    musicMetaLayout->addWidget(d->artlist);
    musicMetaLayout->setSpacing(0);
    infoLayout->setMargin(0);
    infoLayout->addWidget(d->cover, 0, Qt::AlignLeft | Qt::AlignVCenter);
    infoLayout->addLayout(musicMetaLayout, 0);

    auto ctlWidget = new QWidget;
    auto ctlLayout = new QHBoxLayout(ctlWidget);
    ctlLayout->setMargin(0);
    ctlLayout->setSpacing(30);
    ctlLayout->addWidget(d->btPrev, 0, Qt::AlignCenter);
    ctlLayout->addWidget(d->btPlay, 0, Qt::AlignCenter);
    ctlLayout->addWidget(d->btNext, 0, Qt::AlignCenter);

    auto actWidget = new QWidget;
    auto actLayout = new QHBoxLayout(actWidget);
    actLayout->setMargin(0);
    actLayout->setSpacing(20);
    actLayout->addWidget(d->btFavorite, 0, Qt::AlignRight | Qt::AlignVCenter);
    actLayout->addWidget(d->btLyric, 0, Qt::AlignRight | Qt::AlignVCenter);
    actLayout->addWidget(d->btPlayMode, 0, Qt::AlignRight | Qt::AlignVCenter);
    actLayout->addWidget(d->btSound, 0, Qt::AlignRight | Qt::AlignVCenter);
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

    d->title->hide();
    d->artlist->hide();
    d->btPrev->hide();
    d->btNext->hide();
    d->btFavorite->hide();
    d->btLyric->hide();

    D_THEME_INIT_WIDGET(Footer);

    connect(d->btPlay, &QPushButton::clicked, this, [ = ](bool) {
        auto status = d->btPlay->property(sPropertyPlayStatus).toString();
        if (status == sPlayStatusValuePlaying) {
            emit pause(d->m_playinglist, d->m_info);
            status = sPlayStatusValuePause;
        } else {
            emit play(d->m_playinglist, d->m_info);
            status = sPlayStatusValuePlaying;
        }
        updateQssProperty(d->btPlay, sPropertyPlayStatus, status);
    });

    connect(d->btPrev, &QPushButton::clicked, this, [ = ](bool) {
        emit prev(d->m_playinglist, d->m_info);
    });
    connect(d->btNext, &QPushButton::clicked, this, [ = ](bool) {
        emit next(d->m_playinglist, d->m_info);
    });

    connect(d->btFavorite, &QPushButton::clicked, this, [ = ](bool) {
        if (d->m_favlist) {
            // TODO: signal ??
            if (d->m_favlist->contains(d->m_info)) {
                d->m_favlist->removeMusic(d->m_info);
            } else {
                // TODO: check all list has it
                d->m_favlist->appendMusic(d->m_info);
            }
        }
    });

    connect(btPlayList, &QPushButton::clicked, this, [ = ](bool) {
        emit  MusicApp::presenter()->showPlaylist();
    });

    connect(this, &Footer::initFooter, this, [ = ](QSharedPointer<Playlist> favlist, int mode) {
        d->m_mode = mode;
        d->m_favlist = favlist;

        if (!favlist) {
            return;
        }
        connect(favlist.data(), &Playlist::musicRemoved, this, [ = ](const MusicInfo & info) {
            if (info.id == d->m_info.id) {
                updateQssProperty(d->btFavorite, sPropertyFavourite, false);
            }
        });

        connect(favlist.data(), &Playlist::musicAdded, this, [ = ](const MusicInfo & info) {
            if (info.id == d->m_info.id) {
                updateQssProperty(d->btFavorite, sPropertyFavourite, true);
            }
        });
    });
}

void Footer::onMusicPlay(QSharedPointer<Playlist> palylist, const MusicInfo &info)
{
    d->title->setText(info.title);
    d->artlist->setText(info.artist);

    d->title->show();
    d->artlist->show();
    d->btPrev->show();
    d->btNext->show();
    d->btFavorite->show();
    d->btLyric->show();

    d->m_playinglist = palylist;
    d->m_info = info;
    if (d->m_favlist) {
        updateQssProperty(d->btFavorite, sPropertyFavourite, d->m_favlist->contains(info));
    }

    updateQssProperty(this, sPropertyPlayStatus, sPlayStatusValuePlaying);
    updateQssProperty(d->btPlay, sPropertyPlayStatus, sPlayStatusValuePlaying);
}

void Footer::onMusicPause(QSharedPointer<Playlist> palylist, const MusicInfo &info)
{

}

void Footer::onMusicStop(QSharedPointer<Playlist> palylist, const MusicInfo &info)
{

}

void Footer::updateQssProperty(QWidget *w, const char *name, const QVariant &value)
{
    w->setProperty(name, value);
    this->style()->unpolish(w);
    this->style()->polish(w);
    w->repaint();
}
