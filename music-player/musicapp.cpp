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

#include <MprisPlayer>

#include <DApplication>
#include <dutility.h>

#include <option.h>

#include "core/player.h"
#include "core/settings.h"
#include "core/util/threadpool.h"
#include "presenter/presenter.h"
#include "view/mainframe.h"
#include "view/helper/thememanager.h"

using namespace Dtk::Widget;

class MusicAppPrivate
{
public:
    MusicAppPrivate(MusicApp *parent) : q_ptr(parent) {}

    void initMpris(const QString &serviceName);
    void triggerShortcutAction(const QString &optKey);
    void onDataPrepared();
    void onQuit();
    void onRaise();

    Presenter       *presenter      = nullptr;
    MainFrame       *playerFrame    = nullptr;

    MusicApp *q_ptr;
    Q_DECLARE_PUBLIC(MusicApp)
};

void MusicAppPrivate::initMpris(const QString &serviceName)
{
    Q_Q(MusicApp);
    auto mprisPlayer =  new MprisPlayer();
    mprisPlayer->setServiceName(serviceName);

    mprisPlayer->setSupportedMimeTypes(Player::instance()->supportedMimeTypes());
    mprisPlayer->setSupportedUriSchemes(QStringList() << "file");
    mprisPlayer->setCanQuit(true);
    mprisPlayer->setCanRaise(true);
    mprisPlayer->setCanSetFullscreen(false);
    mprisPlayer->setHasTrackList(false);
    // setDesktopEntry: see https://specifications.freedesktop.org/mpris-spec/latest/Media_Player.html#Property:DesktopEntry for more
    mprisPlayer->setDesktopEntry("deepin-music");
    mprisPlayer->setIdentity("Deepin Music Player");

    mprisPlayer->setCanControl(true);
    mprisPlayer->setCanPlay(true);
    mprisPlayer->setCanGoNext(true);
    mprisPlayer->setCanGoPrevious(true);
    mprisPlayer->setCanPause(true);

    q->connect(mprisPlayer, &MprisPlayer::quitRequested, q, [ = ]() {
        onQuit();
    });
    q->connect(mprisPlayer, &MprisPlayer::raiseRequested, q, [ = ]() {
        onRaise();
    });

    presenter->initMpris(mprisPlayer);
}

void MusicAppPrivate::triggerShortcutAction(const QString &optKey)
{
    if (optKey  == "shortcuts.all.volume_up") {
        presenter->volumeUp();
    }
    if (optKey  == "shortcuts.all.volume_down") {
        presenter->volumeDown();
    }
    if (optKey  == "shortcuts.all.next") {
        presenter->next();
    }
    if (optKey  == "shortcuts.all.play_pause") {
        presenter->togglePaly();
    }
    if (optKey  == "shortcuts.all.previous") {
        presenter->prev();
    }
}

void MusicAppPrivate::onDataPrepared()
{
    Q_Q(MusicApp);
    qDebug() << "TRACE:" << "data prepared";

    playerFrame->postInitUI();
    playerFrame->binding(presenter);
    qApp->installEventFilter(playerFrame);
    playerFrame->connect(playerFrame, &MainFrame::triggerShortcutAction,
    q, [ = ](const QString & optKey) {
        this->triggerShortcutAction(optKey);
    });

    presenter->postAction();

    initMpris("DeepinMusic");
}

void MusicAppPrivate::onQuit()
{
//    appPresenter->deleteLater();
//    playerFrame->deleteLater();
//    this->deleteLater();
}

void MusicAppPrivate::onRaise()
{
    playerFrame->show();
    playerFrame->raise();
    playerFrame->activateWindow();
}


MusicApp::MusicApp(MainFrame *frame, QObject *parent)
    : QObject(parent), d_ptr(new MusicAppPrivate(this))
{
    Q_D(MusicApp);
    d->playerFrame = frame;
}

MusicApp::~MusicApp()
{
}

void MusicApp::show()
{
    Q_D(MusicApp);
    auto geometry = AppSettings::instance()->value("base.play.geometry").toByteArray();
    auto state = AppSettings::instance()->value("base.play.state").toInt();
    qDebug() << "restore state:" << state << "gometry:" << geometry;

    if (geometry.isEmpty()) {
        d->playerFrame->resize(QSize(1070, 680));
        d->playerFrame->show();
        DUtility::moveToCenter(d->playerFrame);
    } else {
        d->playerFrame->restoreGeometry(geometry);
        d->playerFrame->setWindowState(static_cast<Qt::WindowStates >(state));
    }
    d->playerFrame->show();
    d->playerFrame->setFocus();
}

void MusicApp::init()
{
    Q_D(MusicApp);

//    auto mediaCount = AppSettings::instance()->value("base.play.media_count").toInt();
//    auto mediaCount = 1;
    d->playerFrame->initUI(true);
    qDebug() << "TRACE:" << "create MainFrame";
//    d->playerFrame->initUI(0 != mediaCount);
    show();

    qDebug() << "TRACE:" << "create Presenter";
    d->presenter = new Presenter;
    auto presenterWork = ThreadPool::instance()->newThread();
    d->presenter->moveToThread(presenterWork);
    connect(presenterWork, &QThread::started, d->presenter, &Presenter::prepareData);
    connect(d->presenter, &Presenter::dataLoaded, this, [ = ]() {
        d->onDataPrepared();
    });

    presenterWork->start();
    qDebug() << "TRACE:" << "start prepare data";
}
