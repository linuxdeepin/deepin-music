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
#include <QApplication>
#include <QStandardPaths>

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

QString MusicApp::configPath()
{
    auto userConfigPath = QStandardPaths::standardLocations(QStandardPaths::ConfigLocation).first();
    return userConfigPath + "/" + qApp->organizationName() + "/" + qApp->applicationName();
}

void MusicApp::showPlayer()
{
    DUtility::moveToCenter(d->playerFrame);
    d->playerFrame->show();
}

void MusicApp::init()
{
    d->appPresenter = new AppPresenter;

    auto presenterWork = new QThread;
    d->appPresenter->moveToThread(presenterWork);
    presenterWork->start();

    d->playerFrame = new PlayerFrame;

    d->playerFrame->initPlaylist(d->appPresenter->allplaylist() , d->appPresenter->lastPlaylist());
    d->playerFrame->initMusiclist(d->appPresenter->allMusicPlaylist(), d->appPresenter->lastPlaylist());
    d->playerFrame->initFooter(d->appPresenter->favMusicPlaylist(),
                               d->appPresenter->lastPlaylist(),
                               d->appPresenter->playMode());
    d->playerFrame->binding(d->appPresenter);
}
