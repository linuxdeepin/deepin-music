/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "presenter.h"

#include <QDir>
#include <QUrl>
#include <QThread>
#include <QProcess>
#include <QApplication>
#include <QStandardPaths>
#include <QDebug>

#include "../musicapp.h"
#include "../core/player.h"
#include "../core/playlist.h"
#include "../core/playlistmanager.h"
#include "../core/mediafilemonitor.h"
#include "../core/lyricservice.h"
#include "../core/mediadatabase.h"
#include "../core/util/musicmeta.h"

class PresenterPrivate
{
public:
    PresenterPrivate()
        : settings(MusicApp::configPath() + "/Config.ini", QSettings::IniFormat)
    {

    }

    LyricService        *lyricService;
    PlaylistManager     *playlistMgr;
    MediaFileMonitor    *moniter;
    QSettings           settings;
};

Presenter::Presenter(QObject *parent)
    : QObject(parent), d(new PresenterPrivate)
{
    qRegisterMetaType<PlaylistMeta>();
    qRegisterMetaType<MusicMetaList>();
    qRegisterMetaType<MusicMeta>();
    qRegisterMetaType<PlaylistPtr >();
    qRegisterMetaType<QList<PlaylistPtr > >();

    MediaDatabase::instance();
}

Presenter::~Presenter()
{

}


void Presenter::prepareData()
{
    d->lyricService = new LyricService;
    auto work = new QThread;
    d->lyricService->moveToThread(work);
    work->start();

    connect(this, &Presenter::requestLyricCoverSearch,
            d->lyricService, &LyricService::searchMeta);
    connect(d->lyricService, &LyricService::lyricSearchFinished,
            this, &Presenter::lyricSearchFinished);
    connect(d->lyricService, &LyricService::coverSearchFinished,
            this, &Presenter::coverSearchFinished);

    d->moniter = new MediaFileMonitor;
    work = new QThread;
    d->moniter->moveToThread(work);
    connect(work, &QThread::started, d->moniter, &MediaFileMonitor::startMonitor);
    work->start();

    //! load playlist
    d->playlistMgr = new PlaylistManager(this);
    d->playlistMgr->load();

    //! load config
    d->settings.beginGroup("Config");
    auto mode = d->settings.value("Mode").toInt();
    auto lastImportPath = d->settings.value("LastImportPath").toString();
    d->settings.endGroup();

    onPlayModeChanged(mode);

    connect(this, &Presenter::importMediaFiles,
            d->moniter, &MediaFileMonitor::importPlaylistFiles);

    connect(d->moniter, &MediaFileMonitor::meidaFileImported,
    this, [ = ](PlaylistPtr playlist, MusicMetaList metalist) {
        if (playlist.isNull()) {
            qCritical() << "add music to empty playlist." << metalist.length();
            return;
        }

        if (playlist->id() != AllMusicListID) {
            PlaylistPtr allplaylist = d->playlistMgr->playlist(AllMusicListID);
            allplaylist->appendMusic(metalist);

        }

        playlist->appendMusic(metalist);
    });

    connect(d->moniter, &MediaFileMonitor::fileRemoved,
    this, [ = ](const QString & filepath) {
        auto metalist = MediaDatabase::searchMusicPath(filepath, std::numeric_limits<int>::max());
        PlaylistPtr allplaylist = d->playlistMgr->playlist(AllMusicListID);

        qDebug() << "remove" << filepath << metalist.length();

        for (auto playlist : d->playlistMgr->allplaylist()) {
            playlist->removeMusic(metalist);
        }
        emit MediaDatabase::instance()->removeMusicMetaList(metalist);
    });

    connect(d->playlistMgr, &PlaylistManager::playingPlaylistChanged,
    this, [ = ](PlaylistPtr playlist) {
        emit playingPlaylistChanged(playlist);
    });
    connect(d->playlistMgr, &PlaylistManager::musicAdded,
            this, &Presenter::musicAdded);

    connect(d->playlistMgr, &PlaylistManager::musiclistAdded,
    this, [ = ](PlaylistPtr playlist, const MusicMetaList & metalist) {
        auto favlist = d->playlistMgr->playlist(FavMusicListID);
        int length = metalist.length();
        MusicMetaList slice;
        for (int i = 0; i < length; i++) {
            auto meta = metalist.at(i);
            slice << meta;
            if (i % 30 == 0) {
                emit this->musiclistAdded(playlist, slice);
                slice.clear();
                QThread::msleep(233);
            }
        }
        if (slice.length() > 0) {
            emit this->musiclistAdded(playlist, slice);
            slice.clear();
        }
    });

    connect(d->playlistMgr, &PlaylistManager::musicRemoved,
            this, &Presenter::musicRemoved);
    connect(d->playlistMgr, &PlaylistManager::selectedPlaylistChanged,
            this, &Presenter::selectedPlaylistChanged);

    connect(Player::instance(), &Player::progrossChanged,
    this, [ = ](qint64 position, qint64 duration) {
        emit progrossChanged(position, duration);
    });
    connect(Player::instance(), &Player::musicPlayed,
    this, [ = ](PlaylistPtr playlist, const MusicMeta & info) {
        MusicMeta favInfo(info);
        favInfo.favourite = d->playlistMgr->playlist(FavMusicListID)->contains(info);
        emit this->musicPlayed(playlist, favInfo);
        qDebug() << "requestLyricCoverSearch" << info.title;
        emit this->requestLyricCoverSearch(info);
    });

    connect(this, &Presenter::changeProgress, Player::instance(), &Player::changeProgress);
    connect(this, &Presenter::play, Player::instance(), &Player::playMusic);
    connect(this, &Presenter::resume, Player::instance(), &Player::resumeMusic);
    connect(this, &Presenter::playNext, Player::instance(), &Player::playNextMusic);
    connect(this, &Presenter::playPrev, Player::instance(), &Player::playPrevMusic);
    connect(this, &Presenter::pause, Player::instance(), &Player::pause);
    connect(this, &Presenter::stop, Player::instance(), &Player::stop);

    emit dataPrepared();
}


PlaylistPtr Presenter::allMusicPlaylist()
{
    return d->playlistMgr->playlist(AllMusicListID);
}

PlaylistPtr Presenter::favMusicPlaylist()
{
    return d->playlistMgr->playlist(FavMusicListID);
}

PlaylistPtr Presenter::lastPlaylist()
{
    return d->playlistMgr->playingPlaylist();
}

QList<PlaylistPtr > Presenter::allplaylist()
{
    return d->playlistMgr->allplaylist();
}

int Presenter::playMode()
{
    return Player::instance()->mode();
}

void Presenter::onMusicRemove(PlaylistPtr playlist, const MusicMetaList &metalist)
{
    if (playlist == d->playlistMgr->playingPlaylist() ||
            playlist->id() == AllMusicListID) {
        //stop music
        for (auto &meta : metalist) {
            if (meta.hash == Player::instance()->playingMeta().hash) {
                emit coverSearchFinished(meta, "");
                emit lyricSearchFinished(meta, "");
                Player::instance()->stop();
                emit musicStoped(d->playlistMgr->playingPlaylist(), meta);
            }
        }
    }

    // TODO: do better;
    if (playlist->id() == AllMusicListID) {
        for (auto &playlist : allplaylist()) {
            playlist->removeMusic(metalist);
        }
        MediaDatabase::instance()->removeMusicMetaList(metalist);

        if (playlist->isEmpty()) {
            emit metaInfoClean();
        }
        return;


    }
    playlist->removeMusic(metalist);

}

void Presenter::onMusicDelete(PlaylistPtr , const MusicMetaList &metalist)
{
    for (auto &playlist : allplaylist()) {
        playlist->removeMusic(metalist);
    }
    MediaDatabase::instance()->removeMusicMetaList(metalist);

    QMap<QString, QString> trashFiles;
    for (auto &meta : metalist) {
        if (meta.hash == Player::instance()->playingMeta().hash) {
            Player::instance()->stop();
            emit coverSearchFinished(meta, "");
            emit lyricSearchFinished(meta, "");
            emit musicStoped(d->playlistMgr->playingPlaylist(), meta);
        }

        trashFiles.insert(meta.localPath, "");

        if (!meta.cuePath.isEmpty()) {
            trashFiles.insert(meta.cuePath, "");
        }

    }

    QProcess::startDetached("gvfs-trash", trashFiles.keys());
}

void Presenter::onMusicAdd(PlaylistPtr playlist,
                              const MusicMetaList &metalist)
{
    PlaylistPtr modifiedPlaylist = playlist;
    if (playlist.isNull()) {
        emit setPlaylistVisible(true);

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
    modifiedPlaylist->appendMusic(metalist);
}

void Presenter::onSelectedPlaylistChanged(PlaylistPtr playlist)
{
    d->playlistMgr->setSelectedPlaylist(playlist);
}

void Presenter::onRequestMusiclistMenu(const QPoint &pos)
{
    QList<PlaylistPtr > newlists = d->playlistMgr->allplaylist();
    // remove all and fav and search
    newlists.removeAll(d->playlistMgr->playlist(AllMusicListID));
    newlists.removeAll(d->playlistMgr->playlist(FavMusicListID));
    newlists.removeAll(d->playlistMgr->playlist(SearchMusicListID));

    auto selectedlist = d->playlistMgr->selectedPlaylist();
    auto favlist = d->playlistMgr->playlist(FavMusicListID);

    emit this->musiclistMenuRequested(pos, selectedlist, favlist, newlists);
}

void Presenter::onSearchText(const QString text)
{
    auto searchList = d->playlistMgr->playlist(SearchMusicListID);
    auto resultList = MediaDatabase::searchMusicMeta(text, 1000);
    searchList->reset(resultList);

    d->playlistMgr->setSelectedPlaylist(searchList);
    if (d->playlistMgr->selectedPlaylist() == searchList) {
        emit this->selectedPlaylistChanged(searchList);
    }
}

void Presenter::onLocateMusic(const QString &hash)
{
    auto allList = d->playlistMgr->playlist(AllMusicListID);

    d->playlistMgr->setSelectedPlaylist(allList);
    onMusicPlay(allList, allList->music(hash));
}

void Presenter::onPlaylistAdd(bool edit)
{
    PlaylistMeta info;
    info.editmode = edit;
    info.readonly = false;
    info.uuid = d->playlistMgr->newID();
    info.displayName = d->playlistMgr->newDisplayName();
    d->playlistMgr->addPlaylist(info);

    emit playlistAdded(d->playlistMgr->playlist(info.uuid));
}

void Presenter::onMusicPlay(PlaylistPtr playlist,  const MusicMeta &meta)
{
    auto nextMeta = meta;
    qDebug() << "Fix me: play status";
    if (0 == d->playlistMgr->playlist(AllMusicListID)->length()) {
        emit requestImportFiles();
        return;
    }

    d->playlistMgr->setPlayingPlaylist(playlist);

    if (0 == playlist->length()) {
        qDebug() << "empty";
        return;
    }

    if (!playlist->contains(nextMeta)) {
        nextMeta = playlist->next(meta);
    }

    qDebug() << nextMeta.title;

    if (Player::instance()->media().canonicalUrl() == QUrl::fromLocalFile(nextMeta.localPath)) {
        emit this->play(d->playlistMgr->playingPlaylist(), nextMeta);
        return;
    }
    qDebug() << "Fix me: play status"
             << nextMeta.hash  << nextMeta.localPath;
    // TODO: using signal;
    Player::instance()->setPlaylist(playlist);
    emit this->play(d->playlistMgr->playingPlaylist(), nextMeta);
}

void Presenter::onMusicPause(PlaylistPtr playlist, const MusicMeta &info)
{
    emit this->pause();
    emit musicPaused(playlist, info);
}

void Presenter::onMusicResume(PlaylistPtr playlist, const MusicMeta &info)
{
    emit this->resume(playlist, info);
    emit this->musicPlayed(playlist, info);
}

void Presenter::onMusicPrev(PlaylistPtr playlist, const MusicMeta &info)
{
    emit this->playPrev(playlist, info);
}

void Presenter::onMusicNext(PlaylistPtr playlist, const MusicMeta &info)
{
    emit this->playNext(playlist, info);
}

void Presenter::onToggleFavourite(const MusicMeta &info)
{
    if (d->playlistMgr->playlist(FavMusicListID)->contains(info)) {
        d->playlistMgr->playlist(FavMusicListID)->removeOneMusic(info);
    } else {
        d->playlistMgr->playlist(FavMusicListID)->appendMusic(MusicMetaList() << info);
    }
}

void Presenter::onChangeProgress(qint64 value, qint64 range)
{
//    auto position = value * Player::instance()->duration() / range;
//    if (position < 0) {
//        qCritical() << "invaild position:" << Player::instance()->media().canonicalUrl() << position;
//    }
//    Player::instance()->setPosition(position);
    emit changeProgress(value, range);
}

void Presenter::onPlayModeChanged(int mode)
{
    Player::instance()->setMode(static_cast<Player::PlayMode>(mode));

    d->settings.beginGroup("Config");
    d->settings.setValue("Mode", mode);
    d->settings.endGroup();
    d->settings.sync();
}

void Presenter::onPlayall(PlaylistPtr playlist)
{
    this->onMusicPlay(playlist, playlist->first());
}

void Presenter::onResort(PlaylistPtr playlist, int sortType)
{
    playlist->sortBy(static_cast<Playlist::SortType>(sortType));
    //store
    emit this->selectedPlaylistChanged(playlist);
}

void Presenter::onImportFiles(const QStringList &filelist)
{
//    PlaylistPtr playlist = d->playlistMgr->playlist(AllMusicListID);
    PlaylistPtr playlist = d->playlistMgr->selectedPlaylist();
    emit importMediaFiles(playlist, filelist);
    emit showMusiclist();
    return;
}

void Presenter::onImportMusicDirectory()
{
    auto musicDir =  QStandardPaths::standardLocations(QStandardPaths::MusicLocation);
    qWarning() << "scan" << musicDir;
    onImportFiles(musicDir);
}

