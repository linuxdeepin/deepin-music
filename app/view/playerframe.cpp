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
#include "playlistview.h"
#include "playlistitem.h"
#include "musiclistwidget.h"

#include "../model/musiclistmodel.h"
#include "../core/playlist.h"
#include "../musicapp.h"

DWIDGET_USE_NAMESPACE

class PlayerFramePrivate
{
public:
    QStackedWidget  *stacked    = nullptr;
    ImportWidget    *import     = nullptr;
    MusicListWidget *musicList  = nullptr;
    PlaylistWidget  *playlist   = nullptr;
};


static void slideWidget(QWidget *left, QWidget *right)
{
    right->show();
    left->show();
    right->resize(left->size());

    int delay = 500;
    QRect leftStart = QRect(0, 0, left->width(), left->height());
    QRect leftEnd = leftStart;
    leftEnd.moveLeft(-left->width());

    QPropertyAnimation *animation = new QPropertyAnimation(left, "geometry");
    animation->setDuration(delay);
    animation->setEasingCurve(QEasingCurve::InCurve);
    animation->setStartValue(leftStart);
    animation->setEndValue(leftEnd);
    animation->start();

    QRect rightStart = QRect(left->width(), 0, right->width(), right->height());
    QRect rightEnd = rightStart;
    rightEnd.moveRight(left->width() - 1);

    QPropertyAnimation *animation2 = new QPropertyAnimation(right, "geometry");
    animation2->setEasingCurve(QEasingCurve::InCurve);
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

static void slideEdgeWidget(QWidget *right, QRect start, QRect end, bool hide = false)
{
    right->show();

    int delay = 200;

    QPropertyAnimation *animation2 = new QPropertyAnimation(right, "geometry");
    animation2->setEasingCurve(QEasingCurve::InCurve);
    animation2->setDuration(delay);
    animation2->setStartValue(start);
    animation2->setEndValue(end);
    animation2->start();
    animation2->connect(animation2, &QPropertyAnimation::finished,
                        animation2, &QPropertyAnimation::deleteLater);
    if (hide)
        animation2->connect(animation2, &QPropertyAnimation::finished,
                            right, &QWidget::hide);

}


PlayerFrame::PlayerFrame(QWidget *parent)
    : DWindow(parent), d(new PlayerFramePrivate)
{
//    setFixedSize(960, 720);

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

    d->playlist = new PlaylistWidget();
    d->playlist->hide();
    d->playlist->setFixedWidth(220);

    d->stacked = new QStackedWidget;
    d->stacked->setObjectName("ContentWidget");
    d->stacked->addWidget(d->import);
    d->stacked->addWidget(d->musicList);
    d->stacked->addWidget(lv);
    d->stacked->addWidget(d->playlist);

    contentLayout->addWidget(d->stacked);
    contentLayout->addWidget(footer);

    setContentWidget(contentWidget);

    D_THEME_INIT_WIDGET(PlayerFrame);

    resize(960, 720);
}

PlayerFrame::~PlayerFrame()
{

}

void PlayerFrame::initMusiclist(QSharedPointer<Playlist> playlist)
{

    if (playlist.isNull() || playlist->info().list.isEmpty()) {
        qDebug() << playlist->info().displayName
                 << playlist->info().list.isEmpty();
        return;
    }
    d->import->hide();
    d->musicList->onMusicListChanged(playlist);
    // TODO: auto change size
    d->musicList->resize(958, 720 - 40);
    d->musicList->raise();
    d->musicList->show();
}

void PlayerFrame::updatePlaylist(QList<QSharedPointer<Playlist> > playlists)
{
    d->playlist->updatePlaylist(playlists);
}

void PlayerFrame::binding(AppPresenter *presenter)
{
    connect(presenter, &AppPresenter::musicListChanged,
            this, &PlayerFrame::onMusicListChanged);
    connect(presenter, &AppPresenter::musicListLoaded,
            this, &PlayerFrame::onMusicListLoaded);
    connect(presenter, &AppPresenter::musicAdded,
            d->musicList, &MusicListWidget::onMusicAdded);
    connect(presenter, &AppPresenter::playlistAdded,
            d->playlist, &PlaylistWidget::onPlaylistAdded);

    connect(d->musicList, &MusicListWidget::musicClicked,
            presenter, &AppPresenter::onMusicPlay);
    connect(d->musicList, &MusicListWidget::musicAddToPlaylist,
            presenter, &AppPresenter::onMusicAddToplaylist);

    connect(d->playlist, &PlaylistWidget::addPlaylist,
            presenter, &AppPresenter::onPlaylistAdd);
    connect(d->playlist, &PlaylistWidget::selectPlaylist,
            presenter, &AppPresenter::onPlaylistSelected);

    connect(presenter, &AppPresenter::showPlaylist,
    this, [ = ]() {
        d->playlist->setFixedHeight(d->stacked->height());

        QRect start(this->width(), 0,
                    d->playlist->width(),
                    d->playlist->height());
        QRect end(this->width() - d->playlist->width(), 0,
                  d->playlist->width(),
                  d->playlist->height());

        d->playlist->raise();

        if (d->playlist->isVisible()) {
            slideEdgeWidget(d->playlist, end, start, true);
        } else {
            slideEdgeWidget(d->playlist, start, end);
        }
    });

    connect(presenter, &AppPresenter::showMusiclist,
    this, [ = ]() {
        slideWidget(d->import, d->musicList);
        d->musicList->resize(d->import->size());
        d->musicList->raise();
        d->musicList->show();
    });
}

void PlayerFrame::resizeEvent(QResizeEvent *event)
{
    DWindow::resizeEvent(event);
}

void PlayerFrame::onMusicListChanged(QSharedPointer<Playlist> playlist)
{
    qDebug() << "onMusicListChanged";
    d->musicList->onMusicListChanged(playlist);
    d->playlist->onCurrentPlaylistChanded(playlist);
}

void PlayerFrame::onMusicListLoaded(QSharedPointer<Playlist> playlist)
{
    if (playlist->info().list.isEmpty()) {
        return;
    }
    d->musicList->onMusicListChanged(playlist);
    d->musicList->resize(d->import->size());
    d->musicList->raise();
    d->musicList->show();
}
