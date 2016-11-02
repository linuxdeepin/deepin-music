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
#include "../core/lyricservice.h"
#include "../core/mediadatabase.h"

class AppPresenterPrivate
{
public:
    AppPresenterPrivate()
        : settings(MusicApp::configPath() + "/Config.ini", QSettings::IniFormat)
    {

    }

    LyricService        *lyricService;
    PlaylistManager     *playlistMgr;
    MediaFileMonitor    *moniter;
    QSettings           settings;
};

AppPresenter::AppPresenter(QObject *parent)
    : QObject(parent), d(new AppPresenterPrivate)
{
    qRegisterMetaType<PlaylistMeta>();
    qRegisterMetaType<MusicMetaList>();
    qRegisterMetaType<MusicMeta>();
    qRegisterMetaType<QSharedPointer<Playlist> >();
    qRegisterMetaType<QList<QSharedPointer<Playlist> > >();

    MediaDatabase::instance();
}

AppPresenter::~AppPresenter()
{

}


void AppPresenter::prepareData()
{
    d->lyricService = new LyricService;
    auto work = new QThread;
    d->lyricService->moveToThread(work);
    work->start();

    connect(this, &AppPresenter::requestLyricCoverSearch,
            d->lyricService, &LyricService::searchLyricCover);
    connect(d->lyricService, &LyricService::lyricSearchFinished,
            this, &AppPresenter::lyricSearchFinished);
    connect(d->lyricService, &LyricService::coverSearchFinished,
            this, &AppPresenter::coverSearchFinished);

    d->moniter = new MediaFileMonitor;
    work = new QThread;
    d->moniter->moveToThread(work);
    work->start();

    //! load playlist
    d->playlistMgr = new PlaylistManager(this);
    d->playlistMgr->load();

    //! load config
    d->settings.beginGroup("Config");
    auto mode = d->settings.value("Mode").toInt();
    d->settings.endGroup();

    onPlayModeChanged(mode);

    connect(this, &AppPresenter::importMediaFiles,
            d->moniter, &MediaFileMonitor::importPlaylistFiles);

    connect(d->moniter, &MediaFileMonitor::meidaFileImported,
    this, [ = ](QSharedPointer<Playlist> playlist, MusicMetaList metalist) {
        qWarning() << "add music to empty playlist." << metalist.length();
        if (playlist.isNull()) {
            qWarning() << "add music to empty playlist." << metalist.length();
            return;
        }

        if (playlist->id() != AllMusicListID) {
            QSharedPointer<Playlist> allplaylist = d->playlistMgr->playlist(AllMusicListID);
            allplaylist->appendMusic(metalist);

        }

        playlist->appendMusic(metalist);
    });

    connect(d->playlistMgr, &PlaylistManager::playingPlaylistChanged,
    this, [ = ](QSharedPointer<Playlist> playlist) {
        emit playingPlaylistChanged(playlist);
    });
    connect(d->playlistMgr, &PlaylistManager::musicAdded,
            this, &AppPresenter::musicAdded);

    connect(d->playlistMgr, &PlaylistManager::musiclistAdded,
    this, [ = ](QSharedPointer<Playlist> palylist, const MusicMetaList & metalist) {
        auto favlist = d->playlistMgr->playlist(FavMusicListID);
        int length = metalist.length();
        MusicMetaList slice;
        for (int i = 0; i < length; i++) {
            auto meta = metalist.at(i);
            slice << meta;
            if (i % 500 == 0) {
                emit this->musiclistAdded(palylist, slice);
                slice.clear();
                QThread::msleep(233);
            }
        }
        if (slice.length() > 0) {
            emit this->musiclistAdded(palylist, slice);
            slice.clear();
        }
    });

    connect(d->playlistMgr, &PlaylistManager::musicRemoved,
            this, &AppPresenter::musicRemoved);
    connect(d->playlistMgr, &PlaylistManager::selectedPlaylistChanged,
            this, &AppPresenter::selectedPlaylistChanged);

    connect(Player::instance(), &Player::progrossChanged,
            this, [ = ](qint64 position, qint64 duration) {
        emit progrossChanged(position, duration);
    });
    connect(Player::instance(), &Player::musicPlayed,
    this, [ = ](QSharedPointer<Playlist> palylist, const MusicMeta & info) {
        MusicMeta favInfo(info);
        favInfo.favourite = d->playlistMgr->playlist(FavMusicListID)->contains(info);
        emit this->musicPlayed(palylist, favInfo);
        emit this->requestLyricCoverSearch(info);
    });

    connect(this, &AppPresenter::changeProgress, Player::instance(), &Player::changeProgress);
    connect(this, &AppPresenter::play, Player::instance(), &Player::playMusic);
    connect(this, &AppPresenter::playNext, Player::instance(), &Player::playNextMusic);
    connect(this, &AppPresenter::playPrev, Player::instance(), &Player::playPrevMusic);
    connect(this, &AppPresenter::pause, Player::instance(), &Player::pause);
    connect(this, &AppPresenter::stop, Player::instance(), &Player::stop);

    emit dataPrepared();
}


QSharedPointer<Playlist> AppPresenter::allMusicPlaylist()
{
    return d->playlistMgr->playlist(AllMusicListID);
}

QSharedPointer<Playlist> AppPresenter::favMusicPlaylist()
{
    return d->playlistMgr->playlist(FavMusicListID);
}

QSharedPointer<Playlist> AppPresenter::lastPlaylist()
{
    return d->playlistMgr->playingPlaylist();
}

QList<QSharedPointer<Playlist> > AppPresenter::allplaylist()
{
    return d->playlistMgr->allplaylist();
}

int AppPresenter::playMode()
{
    return Player::instance()->mode();
}

void AppPresenter::onMusicRemove(QSharedPointer<Playlist> playlist, const MusicMeta &info)
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

void AppPresenter::onMusicAdd(QSharedPointer<Playlist> playlist,
                              const MusicMetaList &metalist)
{
    QSharedPointer<Playlist> modifiedPlaylist = playlist;
    if (playlist.isNull()) {
        emit showPlaylist();

        PlaylistMeta info;
        info.editmode = true;
        info.readonly = false;
        info.uuid = d->playlistMgr->newID();
        info.displayName = d->playlistMgr->newDisplayName();
        modifiedPlaylist = d->playlistMgr->addPlaylist(info);
        emit playlistAdded(d->playlistMgr->playlist(info.uuid));
    }

    if (d->playlistMgr->playlist(modifiedPlaylist->id()).isNull()) {
        qCritical() << "no list" << modifiedPlaylist->id();
        return;
    }
    qCritical() << "appendMusic" << modifiedPlaylist->id();
    modifiedPlaylist->appendMusic(metalist);
}

void AppPresenter::onSelectedPlaylistChanged(QSharedPointer<Playlist> playlist)
{
    d->playlistMgr->setSelectedPlaylist(playlist);
}

void AppPresenter::onRequestMusiclistMenu(const QPoint &pos)
{
    QList<QSharedPointer<Playlist> > newlists = d->playlistMgr->allplaylist();
    // remove all and fav and search
    newlists.removeAll(d->playlistMgr->playlist(AllMusicListID));
    newlists.removeAll(d->playlistMgr->playlist(FavMusicListID));
    newlists.removeAll(d->playlistMgr->playlist(SearchMusicListID));

    auto selectedlist = d->playlistMgr->selectedPlaylist();
    auto favlist = d->playlistMgr->playlist(FavMusicListID);

    emit this->musiclistMenuRequested(pos, selectedlist, favlist, newlists);
}

void AppPresenter::onSearchText(const QString text)
{
    auto searchList = d->playlistMgr->playlist(SearchMusicListID);
    auto resultList = MediaDatabase::searchMusicMeta(text, 1000);
    searchList->reset(resultList);

    d->playlistMgr->setSelectedPlaylist(searchList);
    //    emit this->selectedPlaylistChanged(searchList);
}

void AppPresenter::onLocateMusic(const QString &hash)
{
    auto allList = d->playlistMgr->playlist(AllMusicListID);

    d->playlistMgr->setSelectedPlaylist(allList);
    onMusicPlay(allList, allList->music(hash));
}

void AppPresenter::onPlaylistAdd(bool edit)
{
    PlaylistMeta info;
    info.editmode = edit;
    info.readonly = false;
    info.uuid = d->playlistMgr->newID();
    info.displayName = d->playlistMgr->newDisplayName();
    d->playlistMgr->addPlaylist(info);

    emit playlistAdded(d->playlistMgr->playlist(info.uuid));
}

void AppPresenter::onMusicPlay(QSharedPointer<Playlist> palylist,  const MusicMeta &info)
{
    if (0 == d->playlistMgr->playlist(AllMusicListID)->length()) {
        emit requestImportFiles();
        return;
    }

    d->playlistMgr->setPlayingPlaylist(palylist);
    if (Player::instance()->media().canonicalUrl() == QUrl::fromLocalFile(info.localpath)) {
        emit this->play(d->playlistMgr->playingPlaylist(), info);
        return;
    }
    qDebug() << "Fix me: play status"
             << info.hash  << info.localpath;
    // TODO: using signal;
    Player::instance()->setPlaylist(palylist);
    emit this->play(d->playlistMgr->playingPlaylist(), info);
}

void AppPresenter::onMusicPause(QSharedPointer<Playlist> playlist, const MusicMeta &info)
{
    emit this->pause();
    emit musicPaused(playlist, playlist->next(info));
}

void AppPresenter::onMusicPrev(QSharedPointer<Playlist> playlist, const MusicMeta &info)
{
    emit this->playPrev(playlist, info);
}

void AppPresenter::onMusicNext(QSharedPointer<Playlist> playlist, const MusicMeta &info)
{
    emit this->playNext(playlist, info);
}

void AppPresenter::onToggleFavourite(const MusicMeta &info)
{
    if (d->playlistMgr->playlist(FavMusicListID)->contains(info)) {
        d->playlistMgr->playlist(FavMusicListID)->removeMusic(info);
    } else {
        d->playlistMgr->playlist(FavMusicListID)->appendMusic(MusicMetaList() << info);
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

void AppPresenter::onResort(QSharedPointer<Playlist> palylist, int sortType)
{
    palylist->sortBy(static_cast<Playlist::SortType>(sortType));
    //store
    emit this->selectedPlaylistChanged(palylist);
}

void AppPresenter::onImportFiles(const QStringList &filelist)
{
//    QSharedPointer<Playlist> playlist = d->playlistMgr->playlist(AllMusicListID);
    QSharedPointer<Playlist> playlist = d->playlistMgr->selectedPlaylist();
    emit importMediaFiles(playlist, filelist);
    emit showMusiclist();
    return;
}

void AppPresenter::onImportMusicDirectory()
{
    auto musicDir =  QStandardPaths::standardLocations(QStandardPaths::MusicLocation);
    qWarning() << "scan" << musicDir;
    onImportFiles(musicDir);
}

