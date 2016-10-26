/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "apppresenter.h"

#include <QDir>
#include <QUrl>
#include <QThread>
#include <QApplication>
#include <QStandardPaths>
#include <QDebug>

#include "../musicapp.h"
#include "../core/player.h"
#include "../core/playlistmanager.h"
#include "../core/mediafilemonitor.h"

class AppPresenterPrivate
{
public:
    AppPresenterPrivate()
        : settings(MusicApp::configPath() + "/Config.ini", QSettings::IniFormat)
    {

    }

    PlaylistManager     playlistMgr;
    MediaFileMonitor     *moniter;
    QSettings           settings;
};

AppPresenter::AppPresenter(QObject *parent)
    : QObject(parent), d(new AppPresenterPrivate)
{
    qRegisterMetaType<MusicListInfo>();
    qRegisterMetaType<MusicInfo>();
    qRegisterMetaType<QSharedPointer<Playlist> >();
    qRegisterMetaType<QList<QSharedPointer<Playlist> > >();

    d->moniter = new MediaFileMonitor;
    auto work = new QThread;
    d->moniter->moveToThread(work);
    work->start();

    //! load playlist
    d->playlistMgr.load();

    //! load config
    d->settings.beginGroup("Config");
    auto mode = d->settings.value("Mode").toInt();
    d->settings.endGroup();

    onPlayModeChanged(mode);

    connect(this, &AppPresenter::importMediaFiles,
            d->moniter, &MediaFileMonitor::importPlaylistFiles);

    connect(d->moniter, &MediaFileMonitor::meidaFileImported,
    this, [ = ](QSharedPointer<Playlist> playlist, const MusicInfo & info) {
        if (playlist.isNull()) {
            qWarning() << "add music to empty playlist." << info.url;
            return;
        }
        QSharedPointer<Playlist> allplaylist = d->playlistMgr.playlist(AllMusicListID);
        allplaylist->appendMusic(info);
        playlist->appendMusic(info);
    });

    connect(&d->playlistMgr, &PlaylistManager::playingPlaylistChanged,
    this, [ = ](QSharedPointer<Playlist> playlist) {
        emit playingPlaylistChanged(playlist);
    });
    connect(&d->playlistMgr, &PlaylistManager::musicAdded,
            this, &AppPresenter::musicAdded);
    connect(&d->playlistMgr, &PlaylistManager::musicRemoved,
            this, &AppPresenter::musicRemoved);
    connect(&d->playlistMgr, &PlaylistManager::selectedPlaylistChanged,
            this, &AppPresenter::selectedPlaylistChanged);

    connect(Player::instance(), &Player::progrossChanged, this, [ = ](qint64 position, qint64 duration) {
        emit progrossChanged(position, duration);
    });
    connect(Player::instance(), &Player::musicPlayed,
    this, [ = ](QSharedPointer<Playlist> palylist, const MusicInfo & info) {
        MusicInfo favInfo(info);
        favInfo.favourite = d->playlistMgr.playlist(FavMusicListID)->contains(info);
        emit this->musicPlayed(palylist, favInfo);
    });

    connect(this, &AppPresenter::changeProgress, Player::instance(), &Player::changeProgress);
    connect(this, &AppPresenter::play, Player::instance(), &Player::playMusic);
    connect(this, &AppPresenter::playNext, Player::instance(), &Player::playNextMusic);
    connect(this, &AppPresenter::playPrev, Player::instance(), &Player::playPrevMusic);
    connect(this, &AppPresenter::pause, Player::instance(), &QMediaPlayer::pause);
    connect(this, &AppPresenter::stop, Player::instance(), &QMediaPlayer::stop);

}

AppPresenter::~AppPresenter()
{

}

QSharedPointer<Playlist> AppPresenter::allMusicPlaylist()
{
    return d->playlistMgr.playlist(AllMusicListID);
}

QSharedPointer<Playlist> AppPresenter::favMusicPlaylist()
{
    return d->playlistMgr.playlist(FavMusicListID);
}

QSharedPointer<Playlist> AppPresenter::lastPlaylist()
{
    return d->playlistMgr.playingPlaylist();
}

QList<QSharedPointer<Playlist> > AppPresenter::allplaylist()
{
    return d->playlistMgr.allplaylist();
}

int AppPresenter::playMode()
{
    return Player::instance()->mode();
}

void AppPresenter::onMusicRemove(QSharedPointer<Playlist> playlist, const MusicInfo &info)
{
    playlist->removeMusic(info);

    // TODO: do better;
    if (playlist->id() == AllMusicListID) {
        for (auto &playlist : allplaylist()) {
            if (playlist->contains(info)) {
                playlist->removeMusic(info);
            }
        }
    }
}

void AppPresenter::onMusicAdd(const QString &id, const MusicInfo &info)
{
    QString listid = id;
    if (id == "New") {
        emit showPlaylist();

        MusicListInfo info;
        info.editmode = true;
        info.readonly = false;
        info.id = d->playlistMgr.newID();
        info.displayName = d->playlistMgr.newDisplayName();
        qDebug() << "get new " << info.displayName;
        d->playlistMgr.addPlaylist(info);

        emit playlistAdded(d->playlistMgr.playlist(info.id));
        listid = info.id;
    }
    if (d->playlistMgr.playlist(listid).isNull()) {
        qDebug() << "no list";
        return;
    }
    d->playlistMgr.playlist(listid)->appendMusic(info);
}

void AppPresenter::onSelectedPlaylistChanged(QSharedPointer<Playlist> playlist)
{
    d->playlistMgr.setSelectedPlaylist(playlist);
}

void AppPresenter::onRequestMusiclistMenu(MusicItem *item, const QPoint &pos)
{
    QList<QSharedPointer<Playlist> > newlists = d->playlistMgr.allplaylist();
    // remove all and fav and search
    newlists.removeAll(d->playlistMgr.playlist(AllMusicListID));
    newlists.removeAll(d->playlistMgr.playlist(FavMusicListID));
    newlists.removeAll(d->playlistMgr.playlist(SearchMusicListID));

    auto selectedlist = d->playlistMgr.selectedPlaylist();
    auto favlist = d->playlistMgr.playlist(FavMusicListID);

    emit this->musiclistMenuRequested(item, pos, selectedlist, favlist, newlists);
}

void AppPresenter::onPlaylistAdd(bool edit)
{
    MusicListInfo info;
    info.editmode = edit;
    info.readonly = false;
    info.id = d->playlistMgr.newID();
    info.displayName = d->playlistMgr.newDisplayName();
    d->playlistMgr.addPlaylist(info);

    emit playlistAdded(d->playlistMgr.playlist(info.id));
}

void AppPresenter::onMusicPlay(QSharedPointer<Playlist> palylist,  const MusicInfo &info)
{
    if (0 == d->playlistMgr.playlist(AllMusicListID)->length()) {
        emit requestImportFiles();
        return;
    }

    d->playlistMgr.setPlayingPlaylist(palylist);
    if (Player::instance()->media().canonicalUrl() == QUrl::fromLocalFile(info.url)) {
        emit this->play(d->playlistMgr.playingPlaylist(), info);
        return;
    }
    qDebug() << "Fix me: play status" << info.id  << info.url << Player::instance();
    // TODO: using signal;
    Player::instance()->setPlaylist(palylist);
    emit this->play(d->playlistMgr.playingPlaylist(), info);
}

void AppPresenter::onMusicPause(QSharedPointer<Playlist> playlist, const MusicInfo &info)
{
    emit this->pause();
    emit musicPaused(playlist, playlist->next(info));
}

void AppPresenter::onMusicPrev(QSharedPointer<Playlist> playlist, const MusicInfo &info)
{
    emit this->playPrev(playlist, info);
}

void AppPresenter::onMusicNext(QSharedPointer<Playlist> playlist, const MusicInfo &info)
{
    emit this->playNext(playlist, info);
}

void AppPresenter::onToggleFavourite(const MusicInfo &info)
{
    if (d->playlistMgr.playlist(FavMusicListID)->contains(info)) {
        d->playlistMgr.playlist(FavMusicListID)->removeMusic(info);
    } else {
        d->playlistMgr.playlist(FavMusicListID)->appendMusic(info);
    }
}

void AppPresenter::onChangeProgress(qint64 value, qint64 range)
{
//    auto position = value * Player::instance()->duration() / range;
//    if (position < 0) {
//        qCritical() << "invaild position:" << Player::instance()->media().canonicalUrl() << position;
//    }
//    Player::instance()->setPosition(position);
    emit changeProgress(value, range);
}

void AppPresenter::onPlayModeChanged(int mode)
{
    Player::instance()->setMode(static_cast<Player::PlayMode>(mode));

    d->settings.beginGroup("Config");
    d->settings.setValue("Mode", mode);
    d->settings.endGroup();
    d->settings.sync();
}

void AppPresenter::onPlayall(QSharedPointer<Playlist> playlist)
{
    this->onMusicPlay(playlist, playlist->first());
}

void AppPresenter::onImportFiles(const QStringList &filelist)
{
//    QSharedPointer<Playlist> playlist = d->playlistMgr.playlist(AllMusicListID);
    QSharedPointer<Playlist> playlist = d->playlistMgr.selectedPlaylist();
    emit importMediaFiles(playlist, filelist);
    emit showMusiclist();
    return;
}

void AppPresenter::onImportMusicDirectory()
{
    auto musicDir =  QStandardPaths::standardLocations(QStandardPaths::MusicLocation);
    qDebug() << "scan" << musicDir;
    onImportFiles(musicDir);
}

