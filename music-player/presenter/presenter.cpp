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
#include <DUtil>

#include <plugininterface.h>
#include <metadetector.h>

#include "../musicapp.h"
#include "../core/player.h"
#include "../core/playlist.h"
#include "../core/playlistmanager.h"
#include "../core/metasearchservice.h"
#include "../core/mediadatabase.h"
#include "../core/settings.h"
#include "../core/medialibrary.h"
#include "../core/pluginmanager.h"
#include "../core/util/threadpool.h"

using namespace DMusic;

PresenterPrivate::PresenterPrivate(Presenter *parent)
    : QObject(parent), q_ptr(parent)
{

}

void PresenterPrivate::initBackend()
{
    Q_Q(Presenter);

    MetaDetector::init();

    auto pm = PluginManager::instance();
    connect(this, &PresenterPrivate::requestInitPlugin,
            pm, &PluginManager::init);
    ThreadPool::instance()->moveToNewThread(pm);

    MediaDatabase::instance()->init();
    ThreadPool::instance()->moveToNewThread(MediaDatabase::instance());

    qDebug() << "TRACE:" << "database init finished";

    player = Player::instance();
    player->init();
    qDebug() << "TRACE:" << "player init finished";

    settings = AppSettings::instance();

    library = MediaLibrary::instance();
    library->init();
    ThreadPool::instance()->moveToNewThread(MediaLibrary::instance());
    qDebug() << "TRACE:" << "library init finished";

    playlistMgr = new PlaylistManager;
    playlistMgr->load();
    qDebug() << "TRACE:" << "playlistMgr init finished";

    currentPlaylist = playlistMgr->playlist(AllMusicListID);

    connect(this, &PresenterPrivate::play, player, &Player::playMeta);
    connect(this, &PresenterPrivate::resume, player, &Player::resume);
    connect(this, &PresenterPrivate::playNext, player, &Player::playNextMeta);
    connect(this, &PresenterPrivate::playPrev, player, &Player::playPrevMusic);
    connect(this, &PresenterPrivate::pause, player, &Player::pause);
    connect(this, &PresenterPrivate::stop, player, &Player::stop);

    connect(pm, &PluginManager::onPluginLoaded,
    this, [ = ](const QString & objectName, DMusic::Plugin::PluginInterface * instance) {
        if (instance && instance->pluginType() == DMusic::Plugin::PluginType::TypeMetaSearchEngine) {
            qDebug() << "load plugins" << objectName;
            lyricService = MetaSearchService::instance();
            qDebug() << "TRACE:" << "lyricService init finished";
            lyricService->init();

            connect(lyricService, &MetaSearchService::coverSearchFinished,
            this, [ = ](const MetaPtr meta, const DMusic::SearchMeta & search, const QByteArray & coverData) {
                if (search.id != meta->searchID) {
                    meta->searchID = search.id;
                    meta->updateSearchIndex();
                    emit MediaDatabase::instance()->updateMediaMeta(meta);
                }
                emit q->coverSearchFinished(meta, search, coverData);
            });

            connect(this, &PresenterPrivate::requestMetaSearch,
                    lyricService, &MetaSearchService::searchMeta);
            connect(this, &PresenterPrivate::requestChangeMetaCache,
                    lyricService, &MetaSearchService::onChangeMetaCache);
            connect(lyricService, &MetaSearchService::lyricSearchFinished,
                    q, &Presenter::lyricSearchFinished);
            connect(lyricService, &MetaSearchService::contextSearchFinished,
                    q, &Presenter::contextSearchFinished);
            connect(q, &Presenter::requestContextSearch,
                    lyricService, &MetaSearchService::searchContext);

            ThreadPool::instance()->moveToNewThread(MetaSearchService::instance());

            auto activeMeta = Player::instance()->activeMeta();
            if (activeMeta) {
                emit requestMetaSearch(activeMeta);
            }
        }
    });
}

QDataStream &operator<<(QDataStream &dataStream, const MetaPtr &objectA)
{
    auto ptr = objectA.data();
    auto ptrval = reinterpret_cast<qulonglong>(ptr);
    auto var = QVariant::fromValue(ptrval);
    dataStream << var;
    return  dataStream;
}

QDataStream &operator>>(QDataStream &dataStream, MetaPtr &objectA)
{
    QVariant var;
    dataStream >> var;
    qulonglong ptrval = var.toULongLong();
    auto ptr = reinterpret_cast<MediaMeta *>(ptrval);
    objectA = MetaPtr(ptr);
    return dataStream;
}

Presenter::Presenter(QObject *parent)
    : QObject(parent), d_ptr(new PresenterPrivate(this))
{
    qRegisterMetaType<MetaPtr>();
    qRegisterMetaTypeStreamOperators<MetaPtr>();
    qRegisterMetaType<MetaPtrList>();
    qRegisterMetaType<QList<MediaMeta>>();
    qRegisterMetaType<PlaylistMeta>();
    qRegisterMetaType<PlaylistPtr>();
    qRegisterMetaType<QList<PlaylistPtr>>();

    qRegisterMetaType<QList<SearchMeta>>();
    qRegisterMetaType<SearchMeta>();
}

Presenter::~Presenter()
{
    Q_D(Presenter);
    qDebug() << "destroy Presenter";
    // close gstreamer
    d->player->stop();
    qDebug() << "Presenter destroyed";
}


void Presenter::prepareData()
{
    Q_D(Presenter);

//    QThread::sleep(10);
    d->initBackend();
    qDebug() << "TRACE:" << "initBackend finished";

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
    this, [ = ](const QString & playlistId, int mediaCount) {
        qDebug() << "scanFinished";
        if (d->playlistMgr->playlist(AllMusicListID)->isEmpty()) {
            qDebug() << "scanFinished: meta library clean";
            emit metaLibraryClean();
        }

        if (0 == mediaCount) {
            emit scanFinished(playlistId, mediaCount);
        }
    });

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
        d->settings->setOption("base.play.last_meta", meta->hash);
        d->settings->setOption("base.play.last_playlist", playlist->id());

        MetaPtr favInfo(meta);
        favInfo->favourite = d->playlistMgr->playlist(FavMusicListID)->contains(meta);
        emit this->musicPlayed(playlist, favInfo);
        d->requestMetaSearch(meta);
    });

    connect(d->player, &Player::mediaError,
    this, [ = ](PlaylistPtr playlist, const MetaPtr meta, Player::Error error) {
        Q_D(Presenter);
        Q_ASSERT(!playlist.isNull());
        emit musicError(playlist, meta, error);

        if (error == Player::NoError) {
//            qDebug() << "set d->syncPlayerResult false " << meta->title;
            d->syncPlayerResult = false;
            if (meta->invalid) {
                meta->invalid = false;
                emit musicMetaUpdate(playlist, meta);
            }
            return;
        }

        if (!meta->invalid) {
            meta->invalid = true;
            emit musicMetaUpdate(playlist, meta);
        }

//        qDebug() << "check d->syncPlayerResult false " << d->syncPlayerResult << meta->title;
        if (d->syncPlayerResult) {
//            qDebug() << "set d->syncPlayerResult false " << meta->title;
            d->syncPlayerResult = false;
            emit notifyMusciError(playlist, meta, error);
        } else {
//            qDebug() << "next" << playlist->displayName() << playlist->canNext();
            if (playlist->canNext()) {
                DUtil::TimerSingleShot(800, [d, playlist, meta]() {
                    d->playNext(playlist, meta);
                });
            }
        }
    });

    connect(this, &Presenter::musicMetaUpdate,
    this, [ = ](PlaylistPtr /*playlist*/,  MetaPtr meta) {
        qDebug() << "update" << meta->invalid << meta->length;
        for (auto playlist : allplaylist()) {
            playlist->updateMeta(meta);
        }
        // update database
        meta->updateSearchIndex();
        emit MediaDatabase::instance()->updateMediaMeta(meta);
    });

    connect(this, &Presenter::playNext, this, &Presenter::onMusicNext);

    emit dataLoaded();
}

void Presenter::postAction()
{
    Q_D(Presenter);
    emit d->requestInitPlugin();

    auto volume = d->settings->value("base.play.volume").toInt();
    d->player->setVolume(volume);
    emit this->volumeChanged(d->player->volume());

    auto mute = d->settings->value("base.play.mute").toBool();
    d->player->setMuted(mute);
    emit this->mutedChanged(mute);

    auto playmode = d->settings->value("base.play.playmode").toInt();
    d->player->setMode(static_cast<Player::PlaybackMode>(playmode));
    emit this->modeChanged(playmode);

    auto allplaylist = d->playlistMgr->playlist(AllMusicListID);
    auto lastPlaylist = allplaylist;
    if (lastPlaylist.isNull()) {
        lastPlaylist = allplaylist;
    }

    auto lastMeta = lastPlaylist->first();
    auto position = 0;
    auto isMetaLibClear = MediaLibrary::instance()->isEmpty();
    isMetaLibClear |= allplaylist->isEmpty();

    if (d->settings->value("base.play.remember_progress").toBool() && !isMetaLibClear) {
        d->syncPlayerResult = true;

        auto lastPlaylistId = d->settings->value("base.play.last_playlist").toString();
        if (!d->playlistMgr->playlist(lastPlaylistId).isNull()) {
            lastPlaylist = d->playlistMgr->playlist(lastPlaylistId);
        }
        Q_ASSERT(!lastPlaylist.isNull());
        if (lastPlaylist->id() == SearchMusicListID) {
            lastPlaylist = allplaylist;
        }

        auto lastMetaId = d->settings->value("base.play.last_meta").toString();
        lastMeta = MediaLibrary::instance()->meta(lastMetaId);

        if (lastPlaylist->contains(lastMeta)) {
            lastMeta = lastPlaylist->music(lastMetaId);
        } else {
            lastMeta = lastPlaylist->first();
        }

        if (!lastMeta.isNull()) {
            position = d->settings->value("base.play.last_position").toInt();
            onCurrentPlaylistChanged(lastPlaylist);
            emit locateMusic(lastPlaylist, lastMeta);
            emit musicPlayed(lastPlaylist, lastMeta);

            d->player->setPlayOnLoaded(false);
            d->player->setFadeInOut(false);
            d->player->loadMedia(lastPlaylist, lastMeta);
            d->player->pause();
            d->player->setPosition(position);

            emit musicPaused(lastPlaylist, lastMeta);
            if (d->lyricService) {
                emit d->requestMetaSearch(lastMeta);
            }
        }
    }

    QString toOpenUri = d->settings->value("base.play.to_open_uri").toString();
    if (!toOpenUri.isEmpty()) {
        AppSettings::instance()->setOption("base.play.to_open_uri", "");
        AppSettings::instance()->sync();
        openUri(QUrl(toOpenUri));
    } else {
        if (d->settings->value("base.play.auto_play").toBool() && !lastPlaylist->isEmpty() && !isMetaLibClear) {
            qDebug() << lastPlaylist->id() << lastPlaylist->displayName();
            onCurrentPlaylistChanged(lastPlaylist);
            onMusicResume(lastPlaylist, lastMeta);
        }
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

void Presenter::openUri(const QUrl &uri)
{
    Q_D(Presenter);
    auto localfile = uri.toLocalFile();
    // open url
    qDebug() << "open url" << localfile;
    auto metas = MediaLibrary::instance()->importFile(localfile);
    if (0 == metas.length()) {
        qCritical() << "openUriRequested" << uri;
        return;
    }
    auto list = d->playlistMgr->playlist(AllMusicListID);
    emit MediaLibrary::instance()->meidaFileImported(AllMusicListID, metas);
    onAddToPlaylist(list, metas);
    onSyncMusicPlay(list, metas.first());
    onCurrentPlaylistChanged(list);
}

void Presenter::onSyncMusicPlay(PlaylistPtr playlist, const MetaPtr meta)
{
    Q_D(Presenter);
    d->syncPlayerResult = true;
    d->syncPlayerMeta = meta;
    onMusicPlay(playlist, meta);
}

void Presenter::onSyncMusicResume(PlaylistPtr playlist, const MetaPtr meta)
{
    Q_D(Presenter);
    d->syncPlayerResult = true;
    d->syncPlayerMeta = meta;
    onMusicResume(playlist, meta);
}

void Presenter::onSyncMusicPrev(PlaylistPtr playlist, const MetaPtr meta)
{
    Q_D(Presenter);
    d->syncPlayerResult = true;
    d->syncPlayerMeta = meta;
    onMusicPrev(playlist, meta);
}

void Presenter::onSyncMusicNext(PlaylistPtr playlist, const MetaPtr meta)
{
    Q_D(Presenter);
    d->syncPlayerResult = true;
    d->syncPlayerMeta = meta;
    onMusicNext(playlist, meta);
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

void Presenter::pause()
{
    Q_D(Presenter);
    auto activeList = d->player->activePlaylist();
    auto activeMeta = d->player->activeMeta();
    if (activeList && activeMeta) {
        onMusicPause(activeList, activeMeta);
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
        d->library->removeMediaMetaList(metalist);
    } else {
        next = playlist->removeMusicList(metalist);
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

void Presenter::onMusiclistDelete(PlaylistPtr playlist, const MetaPtrList metalist)
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
    d->library->removeMediaMetaList(metalist);
}

void Presenter::onAddToPlaylist(PlaylistPtr playlist,
                                const MetaPtrList metalist)
{
    Q_D(Presenter);

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
    d->currentPlaylist = playlist;
    emit currentMusicListChanged(d->currentPlaylist);
}

void Presenter::onCustomResort(const QStringList &uuids)
{
    Q_D(Presenter);
    d->playlistMgr->onCustomResort(uuids);
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

void Presenter::onChangeSearchMetaCache(const MetaPtr meta, const DMusic::SearchMeta &search)
{
    Q_D(Presenter);

    if (meta->searchID != search.id) {
        qDebug() << "update search id " << search.id;
        meta->searchID = search.id;
        meta->updateSearchIndex();
        emit MediaDatabase::instance()->updateMediaMeta(meta);
    }

    emit d->requestChangeMetaCache(meta, search);
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

    auto toPlayMeta = meta;
    if (playlist.isNull()) {
        playlist = d->playlistMgr->playlist(AllMusicListID);
    }

    d->player->setPlayOnLoaded(true);
    if (0 == d->playlistMgr->playlist(AllMusicListID)->length()) {
        emit requestImportFiles();
        return;
    }

    auto oldPlayinglist = d->player->activePlaylist();
    if (!oldPlayinglist.isNull() && oldPlayinglist != playlist) {
        qDebug() << "stop old list" << oldPlayinglist->id() << playlist->id();
        oldPlayinglist->play(MetaPtr());
    }
//    if (oldPlayinglist.isNull()) {
//        d->player->playMeta()
//    }

    if (0 == playlist->length()) {
        qCritical() << "empty playlist" << playlist->displayName();
        return;
    }

    if (toPlayMeta.isNull()) {
        toPlayMeta = playlist->first();
    }

    if (!playlist->contains(toPlayMeta)) {
        toPlayMeta = playlist->next(meta);
    }

    Q_ASSERT(!toPlayMeta.isNull());
    Q_ASSERT(!playlist.isNull());

    qDebug() << "play" << playlist->displayName()
             << "( count:" << playlist->length() << ")"
             << toPlayMeta->title << toPlayMeta->hash;
    emit d->play(playlist, toPlayMeta);
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
    if (playlist.isNull()) {
        return;
    }

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
    if (playlist.isNull()) {
        return;
    }

    if (playlist->isEmpty()) {
        emit coverSearchFinished(meta, SearchMeta(), "");
        emit lyricSearchFinished(meta, SearchMeta(), "");
        d->player->stop();
        emit this->musicStoped(playlist, meta);
    }
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

    d->player->setVolume(volume);
    qDebug() << "change play volume" << d->player->volume();
    if (volume > 0 && d->player->muted()) {
        d->player->setMuted(false);
        d->settings->setOption("base.play.mute", false);
    }
    d->settings->setOption("base.play.volume", volume);
    emit d->updateMprisVolume(volume);
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

    if (d->player->muted()) {
        emit d->updateMprisVolume(0);
    } else {
        emit d->updateMprisVolume(d->player->volume());
    }
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
    if (playlist->sortType() != Playlist::SortByCustom) {
        emit this->musicListResorted(playlist);
    }
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

    auto player = Player::instance();

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
        // TODO: support mpris playlist
        Q_UNUSED(playlist);

        QVariantMap metadata;
        metadata.insert(Mpris::metadataToString(Mpris::Title), meta->title);
        metadata.insert(Mpris::metadataToString(Mpris::Artist), meta->artist);
        metadata.insert(Mpris::metadataToString(Mpris::Album), meta->album);
        metadata.insert(Mpris::metadataToString(Mpris::Length), meta->length / 1000);
        //mprisPlayer->setCanSeek(true);
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

    connect(mprisPlayer, &MprisPlayer::openUriRequested,
    this, [ = ](const QUrl & uri) {
        openUri(uri);
    });

    connect(mprisPlayer, &MprisPlayer::playRequested,
    this, [ = ]() {
        if (d->player->activePlaylist().isNull()) {
            return;
        }

        if (d->player->status() == Player::Paused) {
            onMusicResume(player->activePlaylist(), player->activeMeta());
        } else {
            onMusicPlay(player->activePlaylist(), player->activeMeta());
        }
        mprisPlayer->setPlaybackStatus(Mpris::Playing);
    });

    connect(mprisPlayer, &MprisPlayer::pauseRequested,
    this, [ = ]() {
        if (d->player->activePlaylist().isNull()) {
            return;
        }

        onMusicPause(player->activePlaylist(), player->activeMeta());
        mprisPlayer->setPlaybackStatus(Mpris::Paused);
    });

    connect(mprisPlayer, &MprisPlayer::nextRequested,
    this, [ = ]() {
        if (d->player->activePlaylist().isNull()) {
            return;
        }

        onMusicNext(player->activePlaylist(), player->activeMeta());
        mprisPlayer->setPlaybackStatus(Mpris::Playing);
    });

    connect(mprisPlayer, &MprisPlayer::previousRequested,
    this, [ = ]() {
        if (d->player->activePlaylist().isNull()) {
            return;
        }

        onMusicPrev(player->activePlaylist(), player->activeMeta());
        mprisPlayer->setPlaybackStatus(Mpris::Playing);
    });

    connect(mprisPlayer, &MprisPlayer::volumeRequested,
    this, [ = ](double volume) {
        onVolumeChanged(volume * 100);
        emit this->volumeChanged(volume * 100);
    });

    connect(d, &PresenterPrivate::updateMprisVolume,
    this, [ = ](int volume) {
        mprisPlayer->setVolume(volume / 100.0);
    });

    connect(this, &Presenter::progrossChanged,
    this, [ = ](qint64 pos, qint64) {
        mprisPlayer->setPosition(pos);
    });

    connect(this, &Presenter::coverSearchFinished,
    this, [ = ](const MetaPtr meta, const DMusic::SearchMeta &, const QByteArray &) {
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
