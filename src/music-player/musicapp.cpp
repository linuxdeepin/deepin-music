/*
 * Copyright (C) 2016 ~ 2018 Wuhan Deepin Technology Co., Ltd.
 *
 * Author:     Iceyer <me@iceyer.net>
 *
 * Maintainer: Iceyer <me@iceyer.net>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "musicapp.h"

#ifdef Q_OS_LINUX
#include <unistd.h>
#endif

#include <QDebug>

#include <MprisPlayer>

#include <DApplication>
#include <DWidgetUtil>
#include <DSettingsOption>
#include <DThemeManager>

#include "core/player.h"
#include "core/musicsettings.h"
#include "core/util/threadpool.h"
#include "presenter/presenter.h"
#include "view/mainframe.h"

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

    connect(d->playerFrame, &MainFrame::requitQuit, this, &MusicApp::quit);
}

MusicApp::~MusicApp()
{
}

void dumpGeometry(const QByteArray &geometry)
{
    if (geometry.size() < 4) {
        return;
    }
    QDataStream stream(geometry);
    stream.setVersion(QDataStream::Qt_4_0);

    const quint32 magicNumber = 0x1D9D0CB;
    quint32 storedMagicNumber;
    stream >> storedMagicNumber;
    if (storedMagicNumber != magicNumber) {
        return;
    }

    const quint16 currentMajorVersion = 2;
    quint16 majorVersion = 0;
    quint16 minorVersion = 0;

    stream >> majorVersion >> minorVersion;

    if (majorVersion > currentMajorVersion) {
        return;
    }
    // (Allow all minor versions.)

    QRect restoredFrameGeometry;
    QRect restoredNormalGeometry;
    qint32 restoredScreenNumber;
    quint8 maximized;
    quint8 fullScreen;
    qint32 restoredScreenWidth = 0;

    stream >> restoredFrameGeometry
           >> restoredNormalGeometry
           >> restoredScreenNumber
           >> maximized
           >> fullScreen;

    qDebug() << "restore geometry:" << restoredFrameGeometry
             << restoredNormalGeometry
             << restoredScreenNumber
             << maximized
             << fullScreen;

    if (majorVersion > 1) {
        stream >> restoredScreenWidth;
    }
}

void MusicApp::show()
{
    Q_D(MusicApp);
    auto geometry = MusicSettings::value("base.play.geometry").toByteArray();
    auto state = MusicSettings::value("base.play.state").toInt();
    qDebug() << "restore state:" << state;
    dumpGeometry(geometry);

    if (geometry.isEmpty()) {
        d->playerFrame->resize(QSize(1070, 680));
        d->playerFrame->show();
        Dtk::Widget::moveToCenter(d->playerFrame);
    } else {
        d->playerFrame->restoreGeometry(geometry);
        d->playerFrame->setWindowState(static_cast<Qt::WindowStates >(state));
    }
    d->playerFrame->show();
    d->playerFrame->setFocus();
}

void MusicApp::quit()
{
    Q_D(MusicApp);
    d->presenter->handleQuit();
    qDebug() << "sync config start";
    MusicSettings::sync();
#ifdef Q_OS_LINUX
    sync();
#endif
    qDebug() << "sync config finish, app exit";
    qApp->quit();
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
