/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "musicapp.h"

#include <QTimer>
#include <dutility.h>
#include <QThread>
#include <QDebug>
#include <QMediaPlayer>

#include "presenter/apppresenter.h"

#include "presenter/apppresenter.h"
#include "view/playerframe.h"
#include "view/musiclistwidget.h"

using namespace Dtk::Widget;

class MusicAppPrivate
{
public:
    AppPresenter    *appPresenter   = nullptr;
    PlayerFrame     *playerFrame    = nullptr;
};

MusicApp::MusicApp(QObject *parent)
    : QObject(parent), d(new MusicAppPrivate)
{

}

MusicApp::~MusicApp()
{

}

AppPresenter *MusicApp::presenter()
{
    return MusicApp::instance().d->appPresenter;
}

void MusicApp::showPlayer()
{
    qDebug() << "show";
    DUtility::moveToCenter(d->playerFrame);
    d->playerFrame->show();
}

void MusicApp::init()
{
    d->appPresenter = new AppPresenter;

    auto presenterWork = new QThread;
    d->appPresenter->moveToThread(presenterWork);
    d->appPresenter->player()->moveToThread(presenterWork);
    presenterWork->start();

    qDebug() << presenterWork << QThread::currentThread();

    d->playerFrame = new PlayerFrame;
    connect(d->appPresenter, &AppPresenter::musicListChanged,
            d->playerFrame, &PlayerFrame::onMusicListChanged);

    connect(d->appPresenter, &AppPresenter::musicAdded,
            d->playerFrame->musicList(), &MusicListWidget::onMusicAdded);

    connect(d->playerFrame->musicList(), &MusicListWidget::musicClicked,
            d->appPresenter, &AppPresenter::onMusicPlay);

}
