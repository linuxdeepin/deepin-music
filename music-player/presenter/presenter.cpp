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

#include <option.h>

#include "../musicapp.h"
#include "../core/player.h"
#include "../core/playlist.h"
#include "../core/playlistmanager.h"
#include "../core/metasearchservice.h"
#include "../core/mediadatabase.h"
#include "../core/settings.h"
#include "../core/medialibrary.h"

using namespace DMusic;

static void startInNewThread(QObject *obj)
{
    auto work = new QThread;
    obj->moveToThread(work);
    work->start();
}

PresenterPrivate::PresenterPrivate(Presenter *parent)
    : QObject(parent), q_ptr(parent)
{

}

void PresenterPrivate::initBackend()
{
    MediaDatabase::instance();

    player = Player::instance();

    settings = Settings::instance();

    lyricService = new MetaSearchService;
    startInNewThread(lyricService);

    library = MediaLibrary::instance();
    library->startMonitor();

    startInNewThread(library);

    playlistMgr = new PlaylistManager;
    playlistMgr->load();

    currentPlaylist = playlistMgr->playlist(AllMusicListID);

    connect(this, &PresenterPrivate::requestMetaSearch,
            lyricService, &MetaSearchService::searchMeta);

    connect(this, &PresenterPrivate::play, player, &Player::playMeta);
    connect(this, &PresenterPrivate::resume, player, &Player::resume);
    connect(this, &PresenterPrivate::playNext, player, &Player::playNextMeta);
    connect(this, &PresenterPrivate::playPrev, player, &Player::playPrevMusic);
    connect(this, &PresenterPrivate::pause, player, &Player::pause);
    connect(this, &PresenterPrivate::stop, player, &Player::stop);
}


Presenter::Presenter(QObject *parent)
    : QObject(parent), d_ptr(new PresenterPrivate(this))
{
    MediaDatabase::instance();

    qRegisterMetaType<MetaPtr>();
    qRegisterMetaType<MetaPtrList>();
    qRegisterMetaType<QList<MediaMeta>>();
    qRegisterMetaType<PlaylistMeta>();
    qRegisterMetaType<PlaylistPtr>();
    qRegisterMetaType<QList<PlaylistPtr>>();

    qRegisterMetaType<QList<SearchMeta> >();
    qRegisterMetaType<SearchMeta>();

}

Presenter::~Presenter()
{

}


void Presenter::prepareData()
{
    Q_D(Presenter);

    d->initBackend();

    connect(d->library, &MediaLibrary::meidaFileImported,
    this, [ = ](const QString & playlistId, MetaPtrList metalist) {
        auto playlist = d->playlistMgr->playlist(playlistId);
        if (playlist.isNull()) {
            qCritical() << "invalid playlist id:" << playlistId;
            return;
        }

        if (playlist->id() != AllMusicListID) {
            PlaylistPtr allplaylist = d->playlistMgr->playlist(AllMusicListID);
            allplaylist->appendMusicList(metalist);
        }
        playlist->appendMusicList(metalist);
        emit meidaFilesImported(playlist, metalist);
    });

    connect(d->library, &MediaLibrary::scanFinished,
    this, [ = ](const QString & playlistId) {
        qDebug() << "scanFinished";
        if (d->playlistMgr->playlist(AllMusicListID)->isEmpty()) {
            qDebug() << "scanFinished: meta library clean";
            emit metaLibraryClean();
        }
    });


    //    connect(d->moniter, &MediaFileMonitor::insertToPlaylist,
    //    this, [ = ](const QString & hash, PlaylistPtr playlist) {
    //        auto allplaylist = d->playlistMgr->playlist(AllMusicListID);
    //        auto meta = allplaylist->music(hash);

    //        PlaylistPtr modifiedPlaylist = playlist;
    //        if (d->playlistMgr->playlist(modifiedPlaylist->id()).isNull()) {
    //            qCritical() << "no list" << modifiedPlaylist->id();
    //            return;
    //        }
    //        modifiedPlaylist->appendMusic(MusicMetaList() << meta);
    //    });


    //    connect(d->moniter, &MediaFileMonitor::meidaFileImported,
    //    this, [ = ](PlaylistPtr playlist, MusicMetaList metalist) {
    //        qDebug() << "import" << metalist.length();
    //        if (playlist.isNull()) {
    //            qCritical() << "add music to empty playlist." << metalist.length();
    //            return;
    //        }

    //    });

    connect(d->lyricService, &MetaSearchService::lyricSearchFinished,
            this, &Presenter::lyricSearchFinished);
    connect(d->lyricService, &MetaSearchService::coverSearchFinished,
            this, &Presenter::coverSearchFinished);
    connect(d->lyricService, &MetaSearchService::contextSearchFinished,
            this, &Presenter::contextSearchFinished);

    connect(this, &Presenter::requestContextSearch,
            d->lyricService, &MetaSearchService::searchContext);
//    connect(this, &Presenter::onChangeSearchMetaCache,
//            d->lyricService, &MetaSearchService::onChangeMetaCache);

//    connect(this, &Presenter::requestImportPaths,
//            d->moniter, &MediaFileMonitor::importPlaylistFiles);



//    connect(d->moniter, &MediaFileMonitor::fileRemoved,
//    this, [ = ](const QString & filepath) {
//        auto metalist = MediaDatabase::searchMusicPath(filepath, std::numeric_limits<int>::max());
//        qDebug() << "remove" << filepath << metalist.length();


//        MusicMeta next;
//        auto playinglist = d->player->activePlaylist();
//        for (auto playlist : d->playlistMgr->allplaylist()) {
//            auto meta = playlist->removeMusic(metalist);
//            if (playlist == playinglist) {
//                next = meta;
//            }
//        }

//        for (auto &meta : metalist) {
//            qDebug() << meta->hash <<  d->player->activeMeta().hash;
//            if (meta->hash == d->player->activeMeta().hash) {
//                if (playinglist->isEmpty()) {
//                    onMusicStop(playinglist, next);
//                } else {
//                    onMusicPlay(playinglist, next);
//                }
//            }
//        }

//        emit MediaDatabase::instance()->removeMusicMetaList(metalist);

//        if (d->playlistMgr->playlist(AllMusicListID)->isEmpty()) {
//            qDebug() << "meta library clean";
//            onMusicStop(d->player->activePlaylist(), MusicMeta());
//            emit metaInfoClean();
//        }
//    });

//    connect(d->playlistMgr, &PlaylistManager::playlistRemove,
//    this, [ = ](PlaylistPtr playlist) {
//        auto playinglist = d->player->activePlaylist();
//        if (playinglist.isNull() || playinglist->id() == playlist->id()) {
//            d->playlistMgr->setPlayingPlaylist(d->playlistMgr->playlist(AllMusicListID));
//            onPlayall(d->playlistMgr->playlist(AllMusicListID));
//        }
//    });

//    connect(d->playlistMgr, &PlaylistManager::playingPlaylistChanged,
//    this, [ = ](PlaylistPtr playlist) {
//        emit activePlaylistChanged(playlist);
//    });
//    connect(d->playlistMgr, &PlaylistManager::musicAdded,
//            this, &Presenter::musicAdded);

    connect(d->playlistMgr, &PlaylistManager::musiclistAdded,
    this, [ = ](PlaylistPtr playlist, const MetaPtrList metalist) {
        int length = metalist.length();
        MetaPtrList slice;
        for (int i = 0; i < length; i++) {
            auto meta = metalist.at(i);
            slice << meta;
            if (i % 30 == 0) {
                emit musicListAdded(playlist, slice);
                slice.clear();
                QThread::msleep(233);
            }
        }
        if (slice.length() > 0) {
            emit musicListAdded(playlist, slice);
            slice.clear();
        }
    });

    connect(d->playlistMgr, &PlaylistManager::musiclistRemoved,
    this, [ = ](PlaylistPtr playlist, const MetaPtrList metalist) {
        qDebug() << playlist << playlist->id();
        emit musicListRemoved(playlist, metalist);
    });

    connect(d->player, &Player::positionChanged,
    this, [ = ](qint64 position, qint64 duration) {
        d->settings->setOption("base.play.last_position", position);
        emit progrossChanged(position, duration);
    });
    connect(d->player, &Player::volumeChanged,
            this, &Presenter::volumeChanged);
    connect(d->player, &Player::mutedChanged,
    this, [ = ](bool mute) {
        if (mute) {
            emit this->mutedChanged(mute);
        } else {
            emit this->volumeChanged(d->player->volume());
        }
    });
    connect(d->player, &Player::mediaPlayed,
    this, [ = ](PlaylistPtr playlist, const MetaPtr meta) {
        MetaPtr favInfo(meta);
        favInfo->favourite = d->playlistMgr->playlist(FavMusicListID)->contains(meta);
        emit this->musicPlayed(playlist, favInfo);
        qDebug() << "requestLyricCoverSearch" << meta->title;
        d->requestMetaSearch(meta);
    });
//    connect(d->player, &Player::mediaUpdate,
//            this, &Presenter::musicMetaUpdate);

//    connect(d->player, &Player::mediaError,
//    this, [ = ](PlaylistPtr playlist,  MusicMeta meta, Player::Error error) {
//        Q_D(Presenter);
//        emit musicError(playlist, meta, error);
//        if (error == Player::NoError) {
//            d->syncPlayerResult = false;
//            if (meta->invalid) {
//                meta->invalid = false;
//                emit musicMetaUpdate(playlist, meta);
//            }
//            return;
//        }

//        if (!meta->invalid) {
//            meta->invalid = true;
//            emit musicMetaUpdate(playlist, meta);
//        }
//        if (d->syncPlayerResult) {
//            d->syncPlayerResult = false;
//            emit notifyMusciError(playlist, meta, error);
//        } else {
//            QThread::msleep(500);
//            if (playlist->canNext()) {
//                d->playNext(playlist, meta);
//            }
//        }
//    });

//    connect(this, &Presenter::musicMetaUpdate,
//    this, [ = ](PlaylistPtr /*playlist*/,  MusicMeta meta) {
//        Q_D(Presenter);
//        qDebug() << "update" << meta->invalid << meta->length;
//        for (auto playlist : allplaylist()) {
//            playlist->updateMeta(meta);
//        }
//        // update database
//        meta->updateIndex();
//        emit MediaDatabase::instance()->updateMusicMeta(meta);
//    });


//    auto mode = d->settings->value("base.play.playmode").toInt();
//    d->player->setMode(static_cast<Player::PlaybackMode>(mode));

    emit dataLoaded();
}

void Presenter::postAction()
{
    Q_D(Presenter);

    auto volume = d->settings->value("base.play.volume").toInt();
    d->player->setVolume(volume);
    emit this->volumeChanged(d->player->volume());

    auto mute = d->settings->value("base.play.mute").toBool();
    d->player->setMuted(mute);
    emit this->mutedChanged(mute);

    auto playmode = d->settings->value("base.play.playmode").toInt();
    emit this->modeChanged(playmode);

    auto allplaylist = d->playlistMgr->playlist(AllMusicListID);
    auto lastPlaylist = d->player->activePlaylist();
    if (lastPlaylist.isNull()) {
        lastPlaylist = allplaylist;
    }

    auto lastMeta = lastPlaylist->first();
    auto position = 0;
    auto isMetaLibClear = allplaylist->isEmpty();

    if (d->settings->value("base.play.remember_progress").toBool() && !isMetaLibClear) {
        auto lastPlaylistId = d->settings->value("base.play.last_playlist").toString();
        if (!d->playlistMgr->playlist(lastPlaylistId).isNull()) {
            lastPlaylist = d->playlistMgr->playlist(lastPlaylistId);
        }
        lastMeta = lastPlaylist->first();
        auto lastMetaId = d->settings->value("base.play.last_meta").toString();
        auto lastMeta = MediaLibrary::instance()->meta(lastMetaId);
        if (lastPlaylist->contains(lastMeta)) {
            lastMeta = lastPlaylist->music(lastMetaId);
        } else {
            lastMeta = lastPlaylist->first();
        }

        position = d->settings->value("base.play.last_position").toInt();
        onCurrentPlaylistChanged(lastPlaylist);
        emit locateMusic(lastPlaylist, lastMeta);
        emit musicPlayed(lastPlaylist, lastMeta);
        emit musicPaused(lastPlaylist, lastMeta);
        d->player->setPlayOnLoaded(false);
        d->player->setFadeInOut(false);
        d->player->loadMedia(lastPlaylist, lastMeta);
        d->player->pause();
        d->player->setPosition(position);

        emit d->requestMetaSearch(lastMeta);
    }

    if (d->settings->value("base.play.auto_play").toBool() && !lastPlaylist->isEmpty() && !isMetaLibClear) {
        onCurrentPlaylistChanged(lastPlaylist);
        onSyncMusicPlay(lastPlaylist, lastMeta);
        d->player->resume(lastPlaylist, lastMeta);
        d->player->setPosition(position);
    }

    auto fadeInOut = d->settings->value("base.play.fade_in_out").toBool();
    d->player->setFadeInOut(fadeInOut);

    if (!isMetaLibClear) {
        emit showMusicList(allplaylist);
    }

    // Add playlist
    for (auto playlist : d->playlistMgr->allplaylist()) {
        emit playlistAdded(playlist);
    }

    emit currentMusicListChanged(lastPlaylist);
}

void Presenter::onSyncMusicPlay(PlaylistPtr playlist, const MetaPtr meta)
{
    Q_D(Presenter);
    d->syncPlayerResult = true;
    d->syncPlayerMeta = meta;

    onMusicPlay(playlist, meta);
}

QList<PlaylistPtr > Presenter::allplaylist()
{
    Q_D(Presenter);
    return d->playlistMgr->allplaylist();
}

void Presenter::volumeUp()
{
    Q_D(Presenter);
    onVolumeChanged(d->player->volume() + Player::VolumeStep);
    emit volumeChanged(d->player->volume());
}

void Presenter::volumeDown()
{
    Q_D(Presenter);
    onVolumeChanged(d->player->volume() - Player::VolumeStep);
    emit volumeChanged(d->player->volume());
}

void Presenter::togglePaly()
{
    Q_D(Presenter);
    auto alllist = d->playlistMgr->playlist(AllMusicListID);
    auto activeList = d->player->activePlaylist();
    auto activeMeta = d->player->activeMeta();
    if (activeList.isNull()) {
        onPlayall(alllist);
        return;
    }
    qDebug() << d->player->status();
    switch (d->player->status()) {
    case Player::Stopped:
        onMusicPlay(activeList, activeMeta);
        break;
    case Player::Playing:
        onMusicPause(activeList, activeMeta);
        break;
    case Player::Paused:
        onMusicResume(activeList, activeMeta);
        break;
    case Player::InvalidPlaybackStatus:
        break;
    }

}

void Presenter::next()
{
    Q_D(Presenter);
    auto alllist = d->playlistMgr->playlist(AllMusicListID);
    auto activeList = d->player->activePlaylist();
    auto activeMeta = d->player->activeMeta();
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
    auto activeList = d->player->activePlaylist();
    auto activeMeta = d->player->activeMeta();
    if (activeList.isNull()) {
        onPlayall(alllist);
        return;
    }
    onMusicPrev(activeList, activeMeta);
}

void Presenter::requestImportPaths(PlaylistPtr playlist, const QStringList &filelist)
{
    Q_D(Presenter);
    Q_ASSERT(!playlist.isNull());
    d->library->importMedias(playlist->id(), filelist);
}

void Presenter::onMusiclistRemove(PlaylistPtr playlist, const MetaPtrList metalist)
{
    Q_D(Presenter);
    auto playinglist = d->player->activePlaylist();
    MetaPtr next;

    qDebug() << "remove " << playlist->id() << metalist.length();

    // TODO: do better;
    if (playlist->id() == AllMusicListID) {
        for (auto &playlist : allplaylist()) {
            auto meta =  playlist->removeMusicList(metalist);
            if (playlist == playinglist) {
                next = meta;
            }
        }

        if (playlist->isEmpty()) {
            qDebug() << "meta library clean";
            onMusicStop(playlist, next);
            emit metaLibraryClean();
        }

        MediaDatabase::instance()->removeMediaMetaList(metalist);
    } else {
        playlist->removeMusicList(metalist);
    }

    if (playlist == d->player->activePlaylist()
            || playlist->id() == AllMusicListID) {
        //stop music
        for (auto &meta : metalist) {
            if (d->player->isActiveMeta(meta)) {
                if (playinglist->isEmpty()) {
                    onMusicStop(playinglist, next);
                } else {
                    onMusicPlay(playinglist, next);
                }
            }
        }
    }
}

void Presenter::onMusiclistDelete(PlaylistPtr playlist , const MetaPtrList metalist)
{
    Q_D(Presenter);
    // find next music
    MetaPtr next;
    auto playinglist = d->player->activePlaylist();

    for (auto &playlist : allplaylist()) {
        auto meta = playlist->removeMusicList(metalist);
        if (playlist == playinglist) {
            next = meta;
        }
    }

    if (d->playlistMgr->playlist(AllMusicListID)->isEmpty()) {
        qDebug() << "meta library clean";
        onMusicStop(playlist, MetaPtr());
        emit metaLibraryClean();
    }

    MediaDatabase::instance()->removeMediaMetaList(metalist);

    QMap<QString, QString> trashFiles;
    for (auto &meta : metalist) {
        qDebug() << meta->hash <<  d->player->activeMeta()->hash;
        if (meta->hash == d->player->activeMeta()->hash) {
            if (playinglist->isEmpty()) {
                onMusicStop(playinglist, next);
            } else {
                onMusicPlay(playinglist, next);
            }
        }

        trashFiles.insert(meta->localPath, "");

        if (!meta->cuePath.isEmpty()) {
            trashFiles.insert(meta->cuePath, "");
        }
    }

    for (auto file : trashFiles.keys()) {
// FIXME:        emit d->moniter->fileRemoved(file);
    }
    QProcess::startDetached("gvfs-trash", trashFiles.keys());

}

void Presenter::onAddToPlaylist(PlaylistPtr playlist,
                                const MetaPtrList metalist)
{
    Q_D(Presenter);

    qDebug() << metalist.first();
    PlaylistPtr modifiedPlaylist = playlist;
    if (playlist.isNull()) {
        emit showPlaylist(true);

        PlaylistMeta info;
        info.editmode = true;
        info.readonly = false;
        info.uuid = d->playlistMgr->newID();
        info.displayName = d->playlistMgr->newDisplayName();
        modifiedPlaylist = d->playlistMgr->addPlaylist(info);
        emit playlistAdded(d->playlistMgr->playlist(info.uuid));
    } else {
        emit notifyAddToPlaylist(modifiedPlaylist, metalist);
    }

    if (d->playlistMgr->playlist(modifiedPlaylist->id()).isNull()) {
        qCritical() << "no list" << modifiedPlaylist->id();
        return;
    }
    modifiedPlaylist->appendMusicList(metalist);
}

void Presenter::onCurrentPlaylistChanged(PlaylistPtr playlist)
{
    Q_D(Presenter);

    Q_ASSERT(!playlist.isNull());

    qDebug() << "select playlist" << playlist->id();
    d->currentPlaylist = playlist;

    emit currentMusicListChanged(d->currentPlaylist);
}

void Presenter::onRequestMusiclistMenu(const QPoint &pos)
{
    Q_D(Presenter);
    QList<PlaylistPtr > newlists = d->playlistMgr->allplaylist();
    // remove all and fav and search
    newlists.removeAll(d->playlistMgr->playlist(AllMusicListID));
    newlists.removeAll(d->playlistMgr->playlist(FavMusicListID));
    newlists.removeAll(d->playlistMgr->playlist(SearchMusicListID));

    auto selectedlist = d->currentPlaylist;
    auto favlist = d->playlistMgr->playlist(FavMusicListID);

    emit this->requestMusicListMenu(pos, selectedlist, favlist, newlists);
}

void Presenter::onSearchText(const QString text)
{
    Q_D(Presenter);
    auto searchList = d->playlistMgr->playlist(SearchMusicListID);
    auto resultList = MediaDatabase::searchMediaMeta(text, 1000);
    searchList->reset(resultList);

    if (d->currentPlaylist->id() != SearchMusicListID) {
        d->playlistBeforeSearch = d->currentPlaylist;
    }

    d->currentPlaylist = searchList;
    emit this->currentMusicListChanged(searchList);
}

void Presenter::onExitSearch()
{
    Q_D(Presenter);
    qDebug() << d->playlistBeforeSearch;
    if (!d->playlistBeforeSearch.isNull()) {
        d->currentPlaylist = d->playlistBeforeSearch;
        emit this->currentMusicListChanged(d->playlistBeforeSearch);
    }
}

void Presenter::onLocateMusicAtAll(const QString &hash)
{
    Q_D(Presenter);
    auto allList = d->playlistMgr->playlist(AllMusicListID);
    d->currentPlaylist = allList;
    emit locateMusic(allList, allList->music(hash));
    //    onMusicPlay(allList, allList->music(hash));
}

void Presenter::onChangeSearchMetaCache(const MetaPtr meta)
{

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

void Presenter::onMusicPlay(PlaylistPtr playlist,  const MetaPtr meta)
{
    Q_D(Presenter);

    auto nextMeta = meta;
    if (playlist.isNull()) {
        playlist = d->playlistMgr->playlist(AllMusicListID);
    }

    d->player->setPlayOnLoaded(true);
    qDebug() << "Fix me: play status" ;
    if (0 == d->playlistMgr->playlist(AllMusicListID)->length()) {
        emit requestImportFiles();
        return;
    }

    auto playinglist = d->player->activePlaylist();
    if (!playinglist.isNull() && playinglist != playlist) {
        qDebug() << "stop old list" << playinglist->id() << playlist->id();
        playinglist->play(MetaPtr());
    }

    if (0 == playlist->length()) {
        qCritical() << "empty playlist";
        return;
    }

    if (!playlist->contains(nextMeta)) {
        nextMeta = playlist->next(meta);
    }

    // todo:
    if (d->player->activeMeta() == nextMeta) {
        emit d->play(d->player->activePlaylist(), nextMeta);
        return;
    }
    qDebug() << "Fix me: play status"
             << nextMeta->hash  << nextMeta->localPath;
    // TODO: using signal;
//    d->player->setPlaylist(playlist);
    emit d->play(playlist, nextMeta);
}

void Presenter::onMusicPause(PlaylistPtr playlist, const MetaPtr info)
{
    Q_D(Presenter);
    emit d->pause();
    emit musicPaused(playlist, info);
}

void Presenter::onMusicResume(PlaylistPtr playlist, const MetaPtr info)
{
    Q_D(Presenter);
    emit d->resume(playlist, info);
    emit this->musicPlayed(playlist, info);
}

void Presenter::onMusicStop(PlaylistPtr playlist, const MetaPtr meta)
{
    Q_D(Presenter);
    emit coverSearchFinished(meta, SearchMeta(), "");
    emit lyricSearchFinished(meta, SearchMeta(), "");
    d->player->stop();
    emit this->musicStoped(playlist, meta);
}

void Presenter::onMusicPrev(PlaylistPtr playlist, const MetaPtr meta)
{
    Q_D(Presenter);
    if (playlist->isEmpty()) {
        emit coverSearchFinished(meta, SearchMeta(), "");
        emit lyricSearchFinished(meta, SearchMeta(), "");
        d->player->stop();
        emit this->musicStoped(playlist, meta);
    }
    emit d->playPrev(playlist, meta);
}

void Presenter::onMusicNext(PlaylistPtr playlist, const MetaPtr meta)
{
    Q_D(Presenter);
    if (playlist->isEmpty()) {
        emit coverSearchFinished(meta, SearchMeta(), "");
        emit lyricSearchFinished(meta, SearchMeta(), "");
        d->player->stop();
        emit this->musicStoped(playlist, meta);
    }
    qDebug() << "play next";
    emit d->playNext(playlist, meta);
}

void Presenter::onToggleFavourite(const MetaPtr meta)
{
    Q_D(Presenter);
    if (d->playlistMgr->playlist(FavMusicListID)->contains(meta)) {
        d->playlistMgr->playlist(FavMusicListID)->removeMusicList(MetaPtrList() << meta);
    } else {
        emit notifyAddToPlaylist(d->playlistMgr->playlist(FavMusicListID), MetaPtrList() << meta);
        d->playlistMgr->playlist(FavMusicListID)->appendMusicList(MetaPtrList() << meta);
    }
}

void Presenter::onChangeProgress(qint64 value, qint64 range)
{
    Q_D(Presenter);
//    auto position = value * d->player->duration() / range;
//    if (position < 0) {
//        qCritical() << "invaild position:" << d->player->media().canonicalUrl() << position;
//    }
//    d->player->setPosition(position);
    auto position = value * d->player->duration() / range;
    d->player->setPosition(position);
}

void Presenter::onVolumeChanged(int volume)
{
    Q_D(Presenter);
//    qDebug() << "change play volumr" << volume;
    d->player->setVolume(volume);
    if (volume > 0 && d->player->muted()) {
        d->player->setMuted(false);
        d->settings->setOption("base.play.mute", false);
    }
    d->settings->setOption("base.play.volume", volume);
}

void Presenter::onPlayModeChanged(int mode)
{
    Q_D(Presenter);
    d->player->setMode(static_cast<Player::PlaybackMode>(mode));
    d->settings->setOption("base.play.playmode", mode);
    d->settings->sync();
    emit this->modeChanged(mode);
}

void Presenter::onToggleMute()
{
    Q_D(Presenter);
    d->player->setMuted(! d->player->muted());
    d->settings->setOption("base.play.mute", d->player->muted());
}

void Presenter::onUpdateMetaCodec(const MetaPtr meta)
{
    Q_D(Presenter);
    emit musicMetaUpdate(d->player->activePlaylist(), meta);
}

void Presenter::onPlayall(PlaylistPtr playlist)
{
    onMusicPlay(playlist, playlist->first());
}

void Presenter::onResort(PlaylistPtr playlist, int sortType)
{
    playlist->sortBy(static_cast<Playlist::SortType>(sortType));
    //store
    emit this->musicListResorted(playlist);
}

void Presenter::onImportFiles(const QStringList &filelist)
{
    Q_D(Presenter);
    PlaylistPtr playlist = d->currentPlaylist;
    requestImportPaths(playlist, filelist);
    return;
}

void Presenter::onScanMusicDirectory()
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

    auto player = d->player;

    connect(player, &Player::playbackStatusChanged,
    this, [ = ](Player::PlaybackStatus playbackStatus) {
        switch (playbackStatus) {
        case Player::InvalidPlaybackStatus:
        case Player::Stopped:
            mprisPlayer->setPlaybackStatus(Mpris::Stopped);
            break;
        case Player::Playing:
            mprisPlayer->setPlaybackStatus(Mpris::Playing);
            break;
        case Player::Paused:
            mprisPlayer->setPlaybackStatus(Mpris::Paused);
            break;
        }
    });

    connect(this, &Presenter::musicPlayed, this, [ = ](PlaylistPtr playlist, const MetaPtr meta) {
        if (meta.isNull()) {
            return;
        }

        QVariantMap metadata;
        metadata.insert(Mpris::metadataToString(Mpris::Title), meta->title);
        metadata.insert(Mpris::metadataToString(Mpris::Artist), meta->artist);
        metadata.insert(Mpris::metadataToString(Mpris::Album), meta->album);
        metadata.insert(Mpris::metadataToString(Mpris::Length), meta->length / 1000);
//        mprisPlayer->setCanSeek(true);
        mprisPlayer->setMetadata(metadata);
        mprisPlayer->setLoopStatus(Mpris::Playlist);
        mprisPlayer->setPlaybackStatus(Mpris::Stopped);
        mprisPlayer->setVolume(double(player->volume()) / 100.0);
    });

    connect(mprisPlayer, &MprisPlayer::playPauseRequested,
    this, [ = ]() {
        qCritical() << "it seems not call playbackStatusChanged";
        switch (d->player->status()) {
        case Player::InvalidPlaybackStatus:
        case Player::Stopped:
            onMusicPlay(player->activePlaylist(), player->activeMeta());
            mprisPlayer->setPlaybackStatus(Mpris::Playing);
            break;

        case Player::Playing:
            onMusicPause(player->activePlaylist(), player->activeMeta());
            mprisPlayer->setPlaybackStatus(Mpris::Paused);
            break;
        case Player::Paused:
            onMusicResume(player->activePlaylist(), player->activeMeta());
            mprisPlayer->setPlaybackStatus(Mpris::Playing);
            break;
        }
    });

    connect(mprisPlayer, &MprisPlayer::playRequested,
    this, [ = ]() {
        if (d->player->status() == Player::Paused) {
            onMusicResume(player->activePlaylist(), player->activeMeta());
        } else {
            onMusicPlay(player->activePlaylist(), player->activeMeta());
        }
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
    this, [ = ](const MetaPtr  meta, const DMusic::SearchMeta & song, const QByteArray & coverData) {
        if (player->activeMeta().isNull() || meta.isNull()) {
            return;
        }
        if (player->activeMeta()->hash != meta->hash) {
            return;
        }

        QVariantMap metadata = mprisPlayer->metadata();
        metadata.insert(Mpris::metadataToString(Mpris::ArtUrl), MetaSearchService::coverUrl(meta));
        mprisPlayer->setMetadata(metadata);
    });
}
