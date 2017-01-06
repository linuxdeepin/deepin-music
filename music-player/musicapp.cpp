/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "musicapp.h"

#include <QDebug>
#include <QTimer>
#include <QThread>
#include <QStandardPaths>

#include <MprisPlayer>

#include <DApplication>
#include <dutility.h>

#include "presenter/presenter.h"
#include "view/mainwindow.h"
#include "core/player.h"
#include "core/dsettings.h"
#include "core/mediafilemonitor.h"
#include "view/helper/thememanager.h"

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
    Q_DECLARE_PUBLIC(MusicApp)
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

    // setTheme
    auto theme = DSettings::instance()->option("base.play.theme").toString();
    qDebug() << "set Theme" << theme;
    auto dApp = qobject_cast<DApplication *>(qApp);
    dApp->setTheme(theme);
    ThemeManager::instance()->setTheme(theme);

    d->playerFrame = new MainWindow;
    d->playerFrame->hide();

    auto presenterWork = new QThread;
    d->appPresenter->moveToThread(presenterWork);
    connect(presenterWork, &QThread::started, d->appPresenter, &Presenter::prepareData);
    connect(d->appPresenter, &Presenter::dataLoaded, this, &MusicApp::onDataPrepared);

    presenterWork->start();
}

void MusicApp::initMpris(MprisPlayer *mprisPlayer)
{
    Q_D(MusicApp);
    mprisPlayer->setSupportedMimeTypes(Player::instance()->supportedMimeTypes());
    mprisPlayer->setSupportedUriSchemes(QStringList() << "file");
    mprisPlayer->setCanQuit(true);
    mprisPlayer->setCanRaise(true);
    mprisPlayer->setCanSetFullscreen(false);
    mprisPlayer->setHasTrackList(false);
    mprisPlayer->setDesktopEntry("/usr/share/applications/deepin-music.desktop");
//        mprisPlayer->setDesktopEntry("deepin-music");
    mprisPlayer->setIdentity("Deepin Music Player");

    mprisPlayer->setCanControl(true);
    mprisPlayer->setCanPlay(true);
    mprisPlayer->setCanGoNext(true);
    mprisPlayer->setCanGoPrevious(true);
    mprisPlayer->setCanPause(true);

    connect(mprisPlayer, &MprisPlayer::quitRequested,
            this, &MusicApp::onQuit);
    connect(mprisPlayer, &MprisPlayer::raiseRequested,
            this, &MusicApp::onRaise);

    d->appPresenter->initMpris(mprisPlayer);
}

QWidget *MusicApp::hackFrame()
{

    Q_D(MusicApp);
    return d->playerFrame;
}

void MusicApp::onDataPrepared()
{
    Q_D(MusicApp);

    d->playerFrame->initMusiclist(d->appPresenter->allMusicPlaylist(), d->appPresenter->lastPlaylist());
    d->playerFrame->initPlaylist(d->appPresenter->allplaylist() , d->appPresenter->lastPlaylist());
    d->playerFrame->initFooter(d->appPresenter->lastPlaylist(), d->appPresenter->playMode());
    d->playerFrame->initUI();

    d->playerFrame->binding(d->appPresenter);

    d->appPresenter->loadConfig();

    d->playerFrame->show();

    d->playerFrame->resize(QSize(1070, 680));
    DUtility::moveToCenter(d->playerFrame);
    d->playerFrame->setCoverBackground(d->playerFrame->coverBackground());
    d->playerFrame->setFocus();
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
