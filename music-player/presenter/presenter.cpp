/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "presenter.h"
#include "presenter_p.h"

#include <QDebug>
#include <QDir>
#include <QUrl>
#include <QThread>
#include <QProcess>
#include <QApplication>
#include <QStandardPaths>

#include "../musicapp.h"
#include "../core/player.h"
#include "../core/playlist.h"
#include "../core/playlistmanager.h"
#include "../core/mediafilemonitor.h"
#include "../core/lyricservice.h"
#include "../core/mediadatabase.h"
#include "../core/util/musicmeta.h"
#include "../core/dsettings.h"

PresenterPrivate::PresenterPrivate(Presenter *parent)
    : QObject(parent), q_ptr(parent)
{

}

void PresenterPrivate::initData()
{
    Q_Q(Presenter);

    dsettings = DSettings::instance();

    lyricService = new LyricService;
    auto work = new QThread;
    lyricService->moveToThread(work);
    work->start();

    moniter = new MediaFileMonitor;
    work = new QThread;
    moniter->moveToThread(work);
    connect(work, &QThread::started,
            moniter, &MediaFileMonitor::startMonitor);
    work->start();

    playlistMgr = new PlaylistManager;
    playlistMgr->load();

    connect(this, &PresenterPrivate::requestMetaSearch,
            lyricService, &LyricService::searchMeta);

    connect(this, &PresenterPrivate::play, Player::instance(), &Player::playMeta);
    connect(this, &PresenterPrivate::resume, Player::instance(), &Player::resume);
    connect(this, &PresenterPrivate::playNext, Player::instance(), &Player::playNextMeta);
    connect(this, &PresenterPrivate::playPrev, Player::instance(), &Player::playPrevMusic);
    connect(this, &PresenterPrivate::pause, Player::instance(), &Player::pause);
    connect(this, &PresenterPrivate::stop, Player::instance(), &Player::stop);
}


Presenter::Presenter(QObject *parent)
    : QObject(parent), d_ptr(new PresenterPrivate(this))
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
    Q_D(Presenter);

    d->initData();

    connect(d->lyricService, &LyricService::lyricSearchFinished,
            this, &Presenter::lyricSearchFinished);
    connect(d->lyricService, &LyricService::coverSearchFinished,
            this, &Presenter::coverSearchFinished);

    connect(this, &Presenter::importMediaFiles,
            d->moniter, &MediaFileMonitor::importPlaylistFiles);

    connect(d->moniter, &MediaFileMonitor::scanFinished,
    this, [ = ]() {
        qDebug() << "scanFinished";
        if (d->playlistMgr->playlist(AllMusicListID)->isEmpty()) {
            qDebug() << "scanFinished: meta library clean";
            emit metaInfoClean();
        }
    });

    connect(d->moniter, &MediaFileMonitor::insertToPlaylist,
    this, [ = ](const QString & hash, PlaylistPtr playlist) {
        auto allplaylist = d->playlistMgr->playlist(AllMusicListID);
        auto meta = allplaylist->music(hash);

        PlaylistPtr modifiedPlaylist = playlist;
        if (d->playlistMgr->playlist(modifiedPlaylist->id()).isNull()) {
            qCritical() << "no list" << modifiedPlaylist->id();
            return;
        }
        modifiedPlaylist->appendMusic(MusicMetaList() << meta);
    });


    connect(d->moniter, &MediaFileMonitor::meidaFileImported,
    this, [ = ](PlaylistPtr playlist, MusicMetaList metalist) {
        qDebug() << "improt" << metalist.length();
        if (playlist.isNull()) {
            qCritical() << "add music to empty playlist." << metalist.length();
            return;
        }

        if (playlist->id() != AllMusicListID) {
            PlaylistPtr allplaylist = d->playlistMgr->playlist(AllMusicListID);
            allplaylist->appendMusic(metalist);
        }

        playlist->appendMusic(metalist);

        emit meidaFilesImported(playlist, metalist);
    });

    connect(d->moniter, &MediaFileMonitor::fileRemoved,
    this, [ = ](const QString & filepath) {
        auto metalist = MediaDatabase::searchMusicPath(filepath, std::numeric_limits<int>::max());
        qDebug() << "remove" << filepath << metalist.length();


        MusicMeta next;
        auto playinglist = d->playlistMgr->playingPlaylist();
        for (auto playlist : d->playlistMgr->allplaylist()) {
            auto meta = playlist->removeMusic(metalist);
            if (playlist == playinglist) {
                next = meta;
            }
        }

        for (auto &meta : metalist) {
            qDebug() << meta.hash <<  Player::instance()->activeMeta().hash;
            if (meta.hash == Player::instance()->activeMeta().hash) {
                if (playinglist->isEmpty()) {
                    onMusicStop(playinglist, next);
                } else {
                    onMusicPlay(playinglist, next);
                }
            }
        }

        emit MediaDatabase::instance()->removeMusicMetaList(metalist);

        if (d->playlistMgr->playlist(AllMusicListID)->isEmpty()) {
            qDebug() << "meta library clean";
            onMusicStop(d->playlistMgr->playingPlaylist(), MusicMeta());
            emit metaInfoClean();
        }
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
            this, &Presenter::currentPlaylistChanged);

    connect(Player::instance(), &Player::positionChanged,
    this, [ = ](qint64 position, qint64 duration) {
        DSettings::instance()->setOption("base.play.last_position", position);
        emit progrossChanged(position, duration);
    });
    connect(Player::instance(), &Player::volumeChanged,
            this, &Presenter::volumeChanged);
    connect(Player::instance(), &Player::mutedChanged,
    this, [ = ](bool mute) {
        if (mute) {
            emit this->mutedChanged(mute);
        } else {
            emit this->volumeChanged(Player::instance()->volume());
        }
    });
    connect(Player::instance(), &Player::mediaPlayed,
    this, [ = ](PlaylistPtr playlist, const MusicMeta & info) {
        MusicMeta favInfo(info);
        favInfo.favourite = d->playlistMgr->playlist(FavMusicListID)->contains(info);
        emit this->musicPlayed(playlist, favInfo);
        qDebug() << "requestLyricCoverSearch" << info.title;
        d->requestMetaSearch(info);
    });
    connect(Player::instance(), &Player::mediaUpdate,
            this, &Presenter::musicMetaUpdate);

    connect(Player::instance(), &Player::mediaError,
    this, [ = ](PlaylistPtr playlist,  MusicMeta meta, Player::Error error) {
        Q_D(Presenter);
        emit musicError(playlist, meta, error);
        if (error == Player::NoError) {
            d->syncPlayerResult = false;
            if (meta.invalid) {
                meta.invalid = false;
                emit musicMetaUpdate(playlist, meta);
            }
            return;
        }

        if (!meta.invalid) {
            meta.invalid = true;
            emit musicMetaUpdate(playlist, meta);
        }
        if (d->syncPlayerResult) {
            d->syncPlayerResult = false;
            emit notifyMusciError(playlist, meta, error);
        } else {
            QThread::msleep(500);
            if (playlist->canNext()) {
                d->playNext(playlist, meta);
            }
        }
    });

    connect(this, &Presenter::musicMetaUpdate,
    this, [ = ](PlaylistPtr /*playlist*/,  MusicMeta meta) {
        Q_D(Presenter);
        qDebug() << "update" << meta.invalid << meta.length;
        for (auto playlist : allplaylist()) {
            playlist->updateMeta(meta);
        }
        // update database
        emit MediaDatabase::instance()->updateMusicMeta(meta);
    });


    auto mode = d->dsettings->option("base.play.playmode").toInt();
    Player::instance()->setMode(static_cast<Player::PlaybackMode>(mode));

    emit dataLoaded();
}

void Presenter::loadConfig()
{
    Q_D(Presenter);

    auto volume = d->dsettings->option("base.play.volume").toInt();
    Player::instance()->setVolume(volume);
    emit this->volumeChanged(Player::instance()->volume());

    auto mute = d->dsettings->option("base.play.mute").toBool();
    Player::instance()->setMuted(mute);
    emit this->mutedChanged(mute);
}

void Presenter::postAction()
{
    Q_D(Presenter);
    auto lastPlaylist = d->playlistMgr->playingPlaylist();
    auto lastMeta = lastPlaylist->first();
    auto position = 0;

    if (DSettings::instance()->option("base.play.remember_progress").toBool()) {
        auto lastPlaylistId = DSettings::instance()->option("base.play.last_playlist").toString();
        if (!d->playlistMgr->playlist(lastPlaylistId).isNull()) {
            lastPlaylist = d->playlistMgr->playlist(lastPlaylistId);
        }
        lastMeta = lastPlaylist->first();
        auto lastMetaId = DSettings::instance()->option("base.play.last_meta").toString();
        MusicMeta meta;
        meta.hash = lastMetaId;
        if (lastPlaylist->contains(meta)) {
            lastMeta = lastPlaylist->music(lastMetaId);
        }

        position = DSettings::instance()->option("base.play.last_position").toInt();
    }

    if (DSettings::instance()->option("base.play.auto_play").toBool()) {
        onSelectedPlaylistChanged(lastPlaylist);
        onSyncMusicPlay(lastPlaylist, lastMeta);
        Player::instance()->setPosition(position);
    }
}

void Presenter::onSyncMusicPlay(PlaylistPtr playlist, const MusicMeta &meta)
{
    Q_D(Presenter);
    d->syncPlayerResult = true;
    d->syncPlayerMeta = meta;

    onMusicPlay(playlist, meta);
}


PlaylistPtr Presenter::allMusicPlaylist()
{
    Q_D(Presenter);
    return d->playlistMgr->playlist(AllMusicListID);
}

PlaylistPtr Presenter::favMusicPlaylist()
{
    Q_D(Presenter);
    return d->playlistMgr->playlist(FavMusicListID);
}

PlaylistPtr Presenter::lastPlaylist()
{
    Q_D(Presenter);
    return d->playlistMgr->playingPlaylist();
}

QList<PlaylistPtr > Presenter::allplaylist()
{
    Q_D(Presenter);
    return d->playlistMgr->allplaylist();
}

int Presenter::playMode()
{
    return Player::instance()->mode();
}

void Presenter::volumeup()
{
    onVolumeChanged(Player::instance()->volume() + 5);
    emit volumeChanged(Player::instance()->volume());
}

void Presenter::volumedown()
{
    onVolumeChanged(Player::instance()->volume() - 5);
    emit volumeChanged(Player::instance()->volume());
}

void Presenter::togglePaly()
{
    Q_D(Presenter);
    auto alllist = d->playlistMgr->playlist(AllMusicListID);
    auto activeList = Player::instance()->activePlaylist();
    auto activeMeta = Player::instance()->activeMeta();
    if (activeList.isNull()) {
        onPlayall(alllist);
        return;
    }
    qDebug() << Player::instance()->status();
    switch (Player::instance()->status()) {
    case Player::Stopped:
        onMusicPlay(activeList, activeMeta);
        break;
    case Player::Playing:
        onMusicPause(activeList, activeMeta);
        break;
    case Player::Paused:
        onMusicResume(activeList, activeMeta);
        break;
    }

}

void Presenter::next()
{
    Q_D(Presenter);
    auto alllist = d->playlistMgr->playlist(AllMusicListID);
    auto activeList = Player::instance()->activePlaylist();
    auto activeMeta = Player::instance()->activeMeta();
    if (activeList.isNull()) {
        onPlayall(alllist);
        return;
    }
    onMusicNext(activeList, activeMeta);
}

void Presenter::prev()
{
    Q_D(Presenter);
    auto alllist = d->playlistMgr->playlist(AllMusicListID);
    auto activeList = Player::instance()->activePlaylist();
    auto activeMeta = Player::instance()->activeMeta();
    if (activeList.isNull()) {
        onPlayall(alllist);
        return;
    }
    onMusicPrev(activeList, activeMeta);
}

void Presenter::onMusiclistRemove(PlaylistPtr playlist, const MusicMetaList &metalist)
{
    Q_D(Presenter);
    auto playinglist = d->playlistMgr->playingPlaylist();
    MusicMeta next;

    qDebug() << "remove " << playlist->id() << metalist.length();


    // TODO: do better;
    if (playlist->id() == AllMusicListID) {
        for (auto &playlist : allplaylist()) {
            auto meta =  playlist->removeMusic(metalist);
            if (playlist == playinglist) {
                next = meta;
            }
        }

        if (playlist->isEmpty()) {
            qDebug() << "meta library clean";
            onMusicStop(playlist, MusicMeta());
            emit metaInfoClean();
        }

        MediaDatabase::instance()->removeMusicMetaList(metalist);
    } else {
        playlist->removeMusic(metalist);
    }


    if (playlist == d->playlistMgr->playingPlaylist()
            || playlist->id() == AllMusicListID) {
        //stop music
        for (auto &meta : metalist) {
            if (meta.hash == Player::instance()->activeMeta().hash) {
                if (playinglist->isEmpty()) {
                    onMusicStop(playinglist, next);
                } else {
                    onMusicPlay(playinglist, next);
                }
            }
        }
    }
}

void Presenter::onMusiclistDelete(PlaylistPtr playlist , const MusicMetaList &metalist)
{
    Q_D(Presenter);
    // find next music
    MusicMeta next;
    auto playinglist = d->playlistMgr->playingPlaylist();

    for (auto &playlist : allplaylist()) {
        auto meta = playlist->removeMusic(metalist);
        if (playlist == playinglist) {
            next = meta;
        }
    }

    if (d->playlistMgr->playlist(AllMusicListID)->isEmpty()) {
        qDebug() << "meta library clean";
        onMusicStop(playlist, MusicMeta());
        emit metaInfoClean();
    }

    MediaDatabase::instance()->removeMusicMetaList(metalist);

    QMap<QString, QString> trashFiles;
    for (auto &meta : metalist) {
        qDebug() << meta.hash <<  Player::instance()->activeMeta().hash;
        if (meta.hash == Player::instance()->activeMeta().hash) {
            if (playinglist->isEmpty()) {
                onMusicStop(playinglist, next);
            } else {
                onMusicPlay(playinglist, next);
            }
        }

        trashFiles.insert(meta.localPath, "");

        if (!meta.cuePath.isEmpty()) {
            trashFiles.insert(meta.cuePath, "");
        }
    }

    for (auto file : trashFiles.keys()) {
        emit d->moniter->fileRemoved(file);
    }
    QProcess::startDetached("gvfs-trash", trashFiles.keys());

}

void Presenter::onAddToPlaylist(PlaylistPtr playlist,
                                const MusicMetaList &metalist)
{
    Q_D(Presenter);
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
    emit notifyAddToPlaylist(modifiedPlaylist, metalist);
    modifiedPlaylist->appendMusic(metalist);
}

void Presenter::onSelectedPlaylistChanged(PlaylistPtr playlist)
{
    Q_D(Presenter);
    qDebug() << "select playlist" << playlist->id();
    d->playlistMgr->setSelectedPlaylist(playlist);
}

void Presenter::onRequestMusiclistMenu(const QPoint &pos)
{
    Q_D(Presenter);
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
    Q_D(Presenter);
    auto searchList = d->playlistMgr->playlist(SearchMusicListID);
    auto resultList = MediaDatabase::searchMusicMeta(text, 1000);
    searchList->reset(resultList);

    if (d->playlistMgr->selectedPlaylist()->id() != SearchMusicListID) {
        d->playlistBeforeSearch = d->playlistMgr->selectedPlaylist();
    }

    d->playlistMgr->setSelectedPlaylist(searchList);
    emit this->currentPlaylistChanged(searchList);
}

void Presenter::onExitSearch()
{
    Q_D(Presenter);
    qDebug() << d->playlistBeforeSearch;
    if (!d->playlistBeforeSearch.isNull()) {
        d->playlistMgr->setSelectedPlaylist(d->playlistBeforeSearch);
        emit this->currentPlaylistChanged(d->playlistBeforeSearch);
    }
}

void Presenter::onLocateMusicAtAll(const QString &hash)
{
    Q_D(Presenter);
    auto allList = d->playlistMgr->playlist(AllMusicListID);
    d->playlistMgr->setSelectedPlaylist(allList);
    emit locateMusic(allList, allList->music(hash));
//    onMusicPlay(allList, allList->music(hash));
}

void Presenter::onPlaylistAdd(bool edit)
{
    Q_D(Presenter);
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
    Q_D(Presenter);
    auto nextMeta = meta;
    if (playlist.isNull()) {
        playlist = d->playlistMgr->playlist(AllMusicListID);
    }

    qDebug() << "Fix me: play status" ;
    if (0 == d->playlistMgr->playlist(AllMusicListID)->length()) {
        emit requestImportFiles();
        return;
    }

    auto playinglist = d->playlistMgr->playingPlaylist();
    qDebug() << "stop old list" << playinglist->id() << playlist->id();
    if (playinglist->id() != playlist->id()) {
        playinglist->play(MusicMeta());
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

    //save config
    auto activePlaylist = d->playlistMgr->playingPlaylist();
    DSettings::instance()->setOption("base.play.last_playlist", activePlaylist->id());
    DSettings::instance()->setOption("base.play.last_meta", nextMeta.hash);

    // todo:
    if (Player::instance()->activeMeta().localPath == nextMeta.localPath) {
        emit d->play(d->playlistMgr->playingPlaylist(), nextMeta);
        return;
    }
    qDebug() << "Fix me: play status"
             << nextMeta.hash  << nextMeta.localPath;
    // TODO: using signal;
//    Player::instance()->setPlaylist(playlist);
    emit d->play(d->playlistMgr->playingPlaylist(), nextMeta);
}

void Presenter::onMusicPause(PlaylistPtr playlist, const MusicMeta &info)
{
    Q_D(Presenter);
    emit d->pause();
    emit musicPaused(playlist, info);
}

void Presenter::onMusicResume(PlaylistPtr playlist, const MusicMeta &info)
{
    Q_D(Presenter);
    emit d->resume(playlist, info);
    emit this->musicPlayed(playlist, info);
}

void Presenter::onMusicStop(PlaylistPtr playlist, const MusicMeta &meta)
{
    emit coverSearchFinished(meta, "");
    emit lyricSearchFinished(meta, "");
    Player::instance()->stop();
    emit this->musicStoped(playlist, meta);
}

void Presenter::onMusicPrev(PlaylistPtr playlist, const MusicMeta &meta)
{
    Q_D(Presenter);
    if (playlist->isEmpty()) {
        emit coverSearchFinished(meta, "");
        emit lyricSearchFinished(meta, "");
        Player::instance()->stop();
        emit this->musicStoped(playlist, meta);
    }
    emit d->playPrev(playlist, meta);
}

void Presenter::onMusicNext(PlaylistPtr playlist, const MusicMeta &meta)
{
    Q_D(Presenter);
    if (playlist->isEmpty()) {
        emit coverSearchFinished(meta, "");
        emit lyricSearchFinished(meta, "");
        Player::instance()->stop();
        emit this->musicStoped(playlist, meta);
    }
    qDebug() << "play next";
    emit d->playNext(playlist, meta);
}

void Presenter::onToggleFavourite(const MusicMeta &info)
{
    Q_D(Presenter);
    if (d->playlistMgr->playlist(FavMusicListID)->contains(info)) {
        d->playlistMgr->playlist(FavMusicListID)->removeOneMusic(info);
    } else {
        emit notifyAddToPlaylist(d->playlistMgr->playlist(FavMusicListID), MusicMetaList() << info);
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
    auto position = value * Player::instance()->duration() / range;
    Player::instance()->setPosition(position);
}

void Presenter::onVolumeChanged(int volume)
{
    Q_D(Presenter);
//    qDebug() << "change play volumr" << volume;
    Player::instance()->setVolume(volume);
    if (volume > 0 && Player::instance()->muted()) {
        Player::instance()->setMuted(false);
        d->dsettings->setOption("base.play.mute", false);
    }
    d->dsettings->setOption("base.play.volume", volume);
}

void Presenter::onPlayModeChanged(int mode)
{
    Q_D(Presenter);
    Player::instance()->setMode(static_cast<Player::PlaybackMode>(mode));
    d->dsettings->setOption("base.play.playmode", mode);
    d->dsettings->sync();
}

void Presenter::onToggleMute()
{
    Q_D(Presenter);
    Player::instance()->setMuted(! Player::instance()->muted());
    d->dsettings->setOption("base.play.mute", Player::instance()->muted());
}

void Presenter::onPlayall(PlaylistPtr playlist)
{
    onMusicPlay(playlist, playlist->first());
}

void Presenter::onResort(PlaylistPtr playlist, int sortType)
{
    playlist->sortBy(static_cast<Playlist::SortType>(sortType));
    //store
    emit this->playlistResorted(playlist);
}

void Presenter::onImportFiles(const QStringList &filelist)
{
    Q_D(Presenter);
//    PlaylistPtr playlist = d->playlistMgr->playlist(AllMusicListID);
    PlaylistPtr playlist = d->playlistMgr->selectedPlaylist();
    emit importMediaFiles(playlist, filelist);
//    emit showMusiclist();
    return;
}

void Presenter::onImportMusicDirectory()
{
    auto musicDir =  QStandardPaths::standardLocations(QStandardPaths::MusicLocation);
    qWarning() << "scan" << musicDir;
    onImportFiles(musicDir);
}


void Presenter::initMpris(MprisPlayer *mprisPlayer)
{
    if (!mprisPlayer) {
        return;
    }

    Q_D(Presenter);

    auto player = Player::instance();

    connect(this, &Presenter::musicPlayed, this, [ = ](PlaylistPtr playlist, const MusicMeta & meta) {
        QVariantMap metadata;
        metadata.insert(Mpris::metadataToString(Mpris::Title), meta.title);
        metadata.insert(Mpris::metadataToString(Mpris::Artist), meta.artist);
        metadata.insert(Mpris::metadataToString(Mpris::Album), meta.album);
        metadata.insert(Mpris::metadataToString(Mpris::Length), meta.length / 1000);
//        mprisPlayer->setCanSeek(true);
        mprisPlayer->setMetadata(metadata);
        mprisPlayer->setLoopStatus(Mpris::Playlist);
        mprisPlayer->setPlaybackStatus(Mpris::Playing);
        mprisPlayer->setVolume(double(player->volume()) / 100.0);
    });

    connect(mprisPlayer, &MprisPlayer::playRequested,
    this, [ = ]() {
        onMusicPlay(player->activePlaylist(), player->activeMeta());
        mprisPlayer->setPlaybackStatus(Mpris::Playing);
    });

    connect(mprisPlayer, &MprisPlayer::pauseRequested,
    this, [ = ]() {
        onMusicPause(player->activePlaylist(), player->activeMeta());
        mprisPlayer->setPlaybackStatus(Mpris::Paused);
    });

    connect(mprisPlayer, &MprisPlayer::nextRequested,
    this, [ = ]() {
        onMusicNext(player->activePlaylist(), player->activeMeta());
        mprisPlayer->setPlaybackStatus(Mpris::Playing);
    });

    connect(mprisPlayer, &MprisPlayer::previousRequested,
    this, [ = ]() {
        onMusicPrev(player->activePlaylist(), player->activeMeta());
        mprisPlayer->setPlaybackStatus(Mpris::Playing);
    });

    connect(mprisPlayer, &MprisPlayer::volumeRequested,
    this, [ = ](double volume) {
        onVolumeChanged(volume * 100);
    });

    connect(this, &Presenter::volumeChanged,
    this, [ = ](int volume) {
        mprisPlayer->setVolume(volume / 100.0);
    });

    connect(this, &Presenter::progrossChanged,
    this, [ = ](qint64 pos, qint64 length) {
        mprisPlayer->setPosition(pos);
    });

    connect(this, &Presenter::coverSearchFinished,
    this, [ = ](const MusicMeta & meta, const QByteArray & coverData) {
        if (player->activeMeta().hash != meta.hash) {
            return;
        }

        QVariantMap metadata = mprisPlayer->metadata();
        metadata.insert(Mpris::metadataToString(Mpris::ArtUrl), LyricService::coverUrl(meta));
        mprisPlayer->setMetadata(metadata);
    });
}
