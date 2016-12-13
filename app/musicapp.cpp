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
#include <QThread>
#include <QApplication>
#include <QStandardPaths>

#include <dutility.h>

#include "presenter/presenter.h"
#include "view/mainwindow.h"

using namespace Dtk::Widget;

class MusicAppPrivate
{
public:
    Presenter    *appPresenter   = nullptr;
    PlayerFrame     *playerFrame    = nullptr;
};

MusicApp::MusicApp(QObject *parent)
    : QObject(parent), d(new MusicAppPrivate)
{

}

MusicApp::~MusicApp()
{

}

QString MusicApp::configPath()
{
    auto userConfigPath = QStandardPaths::standardLocations(QStandardPaths::ConfigLocation).first();
    return userConfigPath + "/" + qApp->organizationName() + "/" + qApp->applicationName();
}

QString MusicApp::cachePath()
{
    auto userCachePath = QStandardPaths::standardLocations(QStandardPaths::CacheLocation).first();
    return userCachePath;
}

void MusicApp::init()
{
    d->appPresenter = new Presenter;
    d->playerFrame = new PlayerFrame;

    auto presenterWork = new QThread;
    d->appPresenter->moveToThread(presenterWork);
    connect(presenterWork, &QThread::started, d->appPresenter, &Presenter::prepareData);
    connect(d->appPresenter, &Presenter::dataPrepared, this, &MusicApp::onDataPrepared);

    presenterWork->start();
}

void MusicApp::onDataPrepared()
{
    d->playerFrame->initMusiclist(d->appPresenter->allMusicPlaylist(), d->appPresenter->lastPlaylist());
    d->playerFrame->initPlaylist(d->appPresenter->allplaylist() , d->appPresenter->lastPlaylist());
    d->playerFrame->initFooter(d->appPresenter->lastPlaylist(), d->appPresenter->playMode());

    d->playerFrame->binding(d->appPresenter);

    DUtility::moveToCenter(d->playerFrame);
    d->playerFrame->show();
}
