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

#include <MprisPlayer>

#include <dutility.h>

#include "presenter/presenter.h"
#include "view/mainwindow.h"
#include "core/player.h"
#include "core/mediafilemonitor.h"

using namespace Dtk::Widget;

class MusicAppPrivate
{
public:
    MusicAppPrivate(MusicApp *parent) : q_ptr(parent)
    {
    }

    Presenter       *appPresenter   = nullptr;
    MainWindow      *playerFrame    = nullptr;

    MusicApp *q_ptr;
    Q_DECLARE_PUBLIC(MusicApp);
};



MusicApp::MusicApp(QObject *parent)
    : QObject(parent), d_ptr(new MusicAppPrivate(this))
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
    Q_D(MusicApp);
    d->appPresenter = new Presenter;
    d->playerFrame = new MainWindow;

    auto presenterWork = new QThread;
    d->appPresenter->moveToThread(presenterWork);
    connect(presenterWork, &QThread::started, d->appPresenter, &Presenter::prepareData);
    connect(d->appPresenter, &Presenter::dataLoaded, this, &MusicApp::onDataPrepared);

    presenterWork->start();
}

void MusicApp::initMpris(MprisPlayer *mprisPlayer)
{
    mprisPlayer->setSupportedMimeTypes(Player::instance()->supportedMimeTypes());
    mprisPlayer->setSupportedUriSchemes(QStringList() << "file");
    mprisPlayer->setCanQuit(true);
    mprisPlayer->setCanRaise(true);
    mprisPlayer->setCanSetFullscreen(false);
    mprisPlayer->setHasTrackList(false);
    mprisPlayer->setDesktopEntry("/usr/share/applications/deepin-music.desktop");
    mprisPlayer->setIdentity("Deepin Music Player");

    connect(mprisPlayer, &MprisPlayer::quitRequested,
            this, &MusicApp::onQuit);
    connect(mprisPlayer, &MprisPlayer::raiseRequested,
            this, &MusicApp::onRaise);
}

void MusicApp::onDataPrepared()
{
    Q_D(MusicApp);
    d->playerFrame->initMusiclist(d->appPresenter->allMusicPlaylist(), d->appPresenter->lastPlaylist());
    d->playerFrame->initPlaylist(d->appPresenter->allplaylist() , d->appPresenter->lastPlaylist());
    d->playerFrame->initFooter(d->appPresenter->lastPlaylist(), d->appPresenter->playMode());

    d->playerFrame->binding(d->appPresenter);

    DUtility::moveToCenter(d->playerFrame);
    d->playerFrame->show();
}

void MusicApp::onQuit()
{
    Q_D(MusicApp);
    d->playerFrame->close();
}

void MusicApp::onRaise()
{
    Q_D(MusicApp);
    d->playerFrame->show();
    d->playerFrame->raise();
    d->playerFrame->activateWindow();
}
