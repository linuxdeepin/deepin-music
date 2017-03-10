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
#include <QThread>

#include <MprisPlayer>

#include <DApplication>
#include <dutility.h>

#include <option.h>

#include "core/player.h"
#include "core/settings.h"
#include "presenter/presenter.h"
#include "view/mainframe.h"
#include "view/helper/thememanager.h"

using namespace Dtk::Widget;

class MusicAppPrivate
{
public:
    MusicAppPrivate(MusicApp *parent) : q_ptr(parent)
    {
    }

    Presenter       *appPresenter   = nullptr;
    MainFrame      *playerFrame    = nullptr;

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

void MusicApp::init()
{
    Q_D(MusicApp);
    d->appPresenter = new Presenter;

    // setTheme
    auto theme = Settings::instance()->value("base.play.theme").toString();
    auto themePrefix = Settings::instance()->value("base.play.theme_prefix").toString();

    auto dApp = qobject_cast<DApplication *>(qApp);
    dApp->setTheme(theme);
    ThemeManager::instance()->setPrefix(themePrefix);
    ThemeManager::instance()->setTheme(theme);

    d->playerFrame = new MainFrame;
    d->playerFrame->hide();

    auto presenterWork = new QThread;
    d->appPresenter->moveToThread(presenterWork);
    connect(presenterWork, &QThread::started, d->appPresenter, &Presenter::prepareData);
    connect(d->appPresenter, &Presenter::dataLoaded, this, &MusicApp::onDataPrepared);

    presenterWork->start();

    qDebug() << "TRACE:" << "start prepare data";
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

void MusicApp::openUri(const QUrl &uri)
{
    Q_D(MusicApp);
    d->appPresenter->openUri(uri);
}


void MusicApp::triggerShortcutAction(const QString &optKey)
{
    Q_D(MusicApp);
    if (optKey  == "shortcuts.all.volume_up") {
        d->appPresenter->volumeUp();
    }
    if (optKey  == "shortcuts.all.volume_down") {
        d->appPresenter->volumeDown();
    }
    if (optKey  == "shortcuts.all.next") {
        d->appPresenter->next();
    }
    if (optKey  == "shortcuts.all.play_pause") {
        d->appPresenter->togglePaly();
    }
    if (optKey  == "shortcuts.all.previous") {
        d->appPresenter->prev();
    }
}


void MusicApp::onDataPrepared()
{
    Q_D(MusicApp);
    qDebug() << "TRACE:" << "data prepared";

    d->playerFrame->binding(d->appPresenter);

    auto geometry = Settings::instance()->value("base.play.geometry").toByteArray();
    auto state = Settings::instance()->value("base.play.state").toInt();

//    qDebug() << "restore state:" << state << "gometry:" << geometry;
    if (geometry.isEmpty()) {
        d->playerFrame->resize(QSize(1070, 680));
        d->playerFrame->show();
        DUtility::moveToCenter(d->playerFrame);
    } else {
        d->playerFrame->show();
        d->playerFrame->restoreGeometry(geometry);
        d->playerFrame->setWindowState(static_cast<Qt::WindowStates >(state));
    }

    d->playerFrame->updateUI();
    d->playerFrame->setFocus();

    d->appPresenter->postAction();
    d->playerFrame->setMinimumSize(QSize(720, 480));
    d->playerFrame->focusMusicList();
    qApp->installEventFilter(d->playerFrame);
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
