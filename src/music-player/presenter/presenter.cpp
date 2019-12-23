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

#include "presenter.h"
#include "presenter_p.h"

#include <QDebug>
#include <QDir>
#include <QUrl>
#include <QThread>
#include <QProcess>
#include <QApplication>
#include <QStandardPaths>

#include <DSettingsOption>
#include <DDesktopServices>
#include <DUtil>

#include <plugininterface.h>
#include <metadetector.h>

#include "../musicapp.h"
#include "../core/player.h"
#include "../core/playlist.h"
#include "../core/playlistmanager.h"
#include "../core/metasearchservice.h"
#include "../core/mediadatabase.h"
#include "../core/musicsettings.h"
#include "../core/medialibrary.h"
#include "../core/pluginmanager.h"
#include "../core/util/threadpool.h"
#include "../core/metabufferdetector.h"

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
    qDebug() << "TRACE:" << "player init finished";
    connect(player, &Player::audioBufferProbed, q, [ = ](const QAudioBuffer & buffer) {
        Q_EMIT q->audioBufferProbed(buffer);
    } );

    metaBufferDetector = new MetaBufferDetector(this);
    connect(metaBufferDetector, &MetaBufferDetector::metaBuffer, q, [ = ](const QVector<float> &buffer, const QString & hash) {
        Q_EMIT q->metaBuffer(buffer, hash);
    }, Qt::QueuedConnection);

    settings = MusicSettings::instance();

    library = MediaLibrary::instance();
    library->init();
    ThreadPool::instance()->moveToNewThread(MediaLibrary::instance());
    qDebug() << "TRACE:" << "library init finished";

    playlistMgr = new PlaylistManager;
    playlistMgr->load();
    qDebug() << "TRACE:" << "playlistMgr init finished";

    currentPlaylist = playlistMgr->playlist(AllMusicListID);
    PlaylistPtr albumPlaylist = playlistMgr->playlist(AlbumMusicListID);
    albumPlaylist->metaListToPlayMusicTypePtrList(Playlist::SortByAblum, currentPlaylist->allmusic());
    PlaylistPtr artistPlaylist = playlistMgr->playlist(ArtistMusicListID);
    artistPlaylist->metaListToPlayMusicTypePtrList(Playlist::SortByArtist, currentPlaylist->allmusic());

    player->setCurPlaylist(playlistMgr->playlist(PlayMusicListID));

    connect(this, &PresenterPrivate::play,
    this, [ = ](PlaylistPtr playlist, const MetaPtr meta) {
        auto curPlaylist = player->curPlaylist();
        if (curPlaylist != playlist) {
            curPlaylist->removeMusicList(curPlaylist->allmusic());
            auto curAllMetas = playlist->allmusic();
            for (int i = curAllMetas.size() - 1; i >= 0; i--) {
                if (meta == curAllMetas[i])
                    continue;
                if (curAllMetas[i]->invalid)
                    curAllMetas.removeAt(i);
            }
            curPlaylist->appendMusicList(curAllMetas);
        } else {
            auto curPlaylist = player->curPlaylist();
            auto curAllMetas = curPlaylist->allmusic();
            for (int i = curAllMetas.size() - 1; i >= 0; i--) {
                if (!curAllMetas[i]->invalid)
                    curAllMetas.removeAt(i);
            }
            if (!curAllMetas.isEmpty()) {
                curPlaylist->removeMusicList(curAllMetas);
            }
        }
        player->playMeta(playlist, meta);
    });
    connect(this, &PresenterPrivate::resume,
    this, [ = ](PlaylistPtr playlist, const MetaPtr meta) {
        auto curPlaylist = player->curPlaylist();
        if (curPlaylist != playlist) {
            curPlaylist->removeMusicList(curPlaylist->allmusic());
            auto curAllMetas = playlist->allmusic();
            for (int i = curAllMetas.size() - 1; i >= 0; i--) {
                if (meta == curAllMetas[i])
                    continue;
                if (curAllMetas[i]->invalid)
                    curAllMetas.removeAt(i);
            }
            curPlaylist->appendMusicList(curAllMetas);
        } else {
            auto curPlaylist = player->curPlaylist();
            auto curAllMetas = curPlaylist->allmusic();
            for (int i = curAllMetas.size() - 1; i >= 0; i--) {
                if (!curAllMetas[i]->invalid)
                    curAllMetas.removeAt(i);
            }
            if (!curAllMetas.isEmpty()) {
                curPlaylist->removeMusicList(curAllMetas);
            }
        }
        player->resume(playlist, meta);
    });
    connect(this, &PresenterPrivate::playNext,
    this, [ = ](PlaylistPtr playlist, const MetaPtr meta) {
        auto curPlaylist = player->curPlaylist();
        if (curPlaylist != playlist) {
            curPlaylist->removeMusicList(curPlaylist->allmusic());
            auto curAllMetas = playlist->allmusic();
            for (int i = curAllMetas.size() - 1; i >= 0; i--) {
                if (meta == curAllMetas[i])
                    continue;
                if (curAllMetas[i]->invalid)
                    curAllMetas.removeAt(i);
            }
            curPlaylist->appendMusicList(curAllMetas);
        } else {
            auto curPlaylist = player->curPlaylist();
            auto curAllMetas = curPlaylist->allmusic();
            for (int i = curAllMetas.size() - 1; i >= 0; i--) {
                if (!curAllMetas[i]->invalid)
                    curAllMetas.removeAt(i);
            }
            if (!curAllMetas.isEmpty()) {
                curPlaylist->removeMusicList(curAllMetas);
            }
        }
        player->playNextMeta(playlist, meta);
    });
    connect(this, &PresenterPrivate::playPrev,
    this, [ = ](PlaylistPtr playlist, const MetaPtr meta) {
        auto curPlaylist = player->curPlaylist();
        if (curPlaylist != playlist) {
            curPlaylist->removeMusicList(curPlaylist->allmusic());
            auto curAllMetas = playlist->allmusic();
            for (int i = curAllMetas.size() - 1; i >= 0; i--) {
                if (meta == curAllMetas[i])
                    continue;
                if (curAllMetas[i]->invalid)
                    curAllMetas.removeAt(i);
            }
            curPlaylist->appendMusicList(curAllMetas);
        } else {
            auto curPlaylist = player->curPlaylist();
            auto curAllMetas = curPlaylist->allmusic();
            for (int i = curAllMetas.size() - 1; i >= 0; i--) {
                if (!curAllMetas[i]->invalid)
                    curAllMetas.removeAt(i);
            }
            if (!curAllMetas.isEmpty()) {
                curPlaylist->removeMusicList(curAllMetas);
            }
        }
        player->playPrevMusic(playlist, meta);
    });
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
                    Q_EMIT MediaDatabase::instance()->updateMediaMeta(meta);
                }
                meta->coverUrl = MetaSearchService::coverUrl(meta);
                Q_EMIT q->coverSearchFinished(meta, search, coverData);
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
                Q_EMIT requestMetaSearch(activeMeta);
            }
        }
    });
}

void PresenterPrivate::notifyMusicPlayed(PlaylistPtr playlist, const MetaPtr meta)
{
    Q_Q(Presenter);

    MetaPtr favInfo(meta);
    favInfo->favourite = playlistMgr->playlist(FavMusicListID)->contains(meta);
//    qDebug() << FavMusicListID << meta->title << favInfo->favourite;
    Q_EMIT q->musicPlayed(playlist, favInfo);
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
    qDebug() << "Presenter destroyed";
}

void Presenter::handleQuit()
{
    Q_D(Presenter);
    d->settings->setOption("base.play.last_position", d->lastPlayPosition);
    d->player->stop();
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
        PlaylistPtr allplaylist = d->playlistMgr->playlist(AllMusicListID);
        if (!allplaylist.isNull()) {
            PlaylistPtr albumPlaylist = d->playlistMgr->playlist(AlbumMusicListID);
            PlaylistPtr artistPlaylist = d->playlistMgr->playlist(ArtistMusicListID);
            if (albumPlaylist) {
                albumPlaylist->metaListToPlayMusicTypePtrList(Playlist::SortByAblum, allplaylist->allmusic());
            }
            if (artistPlaylist) {
                artistPlaylist->metaListToPlayMusicTypePtrList(Playlist::SortByArtist, allplaylist->allmusic());
            }
        }
        Q_EMIT meidaFilesImported(playlist, metalist);
    });

    connect(d->library, &MediaLibrary::scanFinished,
    this, [ = ](const QString & playlistId, int mediaCount) {
        qDebug() << "scanFinished";
        if (d->playlistMgr->playlist(AllMusicListID)->isEmpty()) {
            qDebug() << "scanFinished: meta library clean";
            Q_EMIT metaLibraryClean();
        }

        if (0 == mediaCount) {
            Q_EMIT scanFinished(playlistId, mediaCount);
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
                Q_EMIT musicListAdded(playlist, slice);
                slice.clear();
                QThread::msleep(50);
            }
        }
        if (slice.length() > 0) {
            Q_EMIT musicListAdded(playlist, slice);
            slice.clear();
        }
    });

    connect(d->playlistMgr, &PlaylistManager::musiclistRemoved,
    this, [ = ](PlaylistPtr playlist, const MetaPtrList metalist) {
        qDebug() << playlist << playlist->id();
        Q_EMIT musicListRemoved(playlist, metalist);
    });

    connect(d->player, &Player::positionChanged,
    this, [ = ](qint64 position, qint64 duration) {
        d->lastPlayPosition = position;
        Q_EMIT progrossChanged(position, duration);
    });

    connect(d->player, &Player::volumeChanged,
            this, &Presenter::volumeChanged);

    connect(d->player, &Player::mutedChanged,
    this, [ = ](bool mute) {
        if (mute) {
            Q_EMIT this->mutedChanged(mute);
        } else {
            Q_EMIT this->volumeChanged(d->player->volume());
        }
    });

    connect(d->player, &Player::mediaPlayed,
    this, [ = ](PlaylistPtr playlist, const MetaPtr meta) {
        d->settings->setOption("base.play.last_meta", meta->hash);
        d->settings->setOption("base.play.last_playlist", playlist->id());
        d->notifyMusicPlayed(playlist, meta);
        d->requestMetaSearch(meta);
        d->metaBufferDetector->onBufferDetector(meta->localPath, meta->hash);
    });

    connect(d->player, &Player::mediaError,
    this, [ = ](PlaylistPtr playlist, const MetaPtr meta, Player::Error error) {
        Q_D(Presenter);
        Q_ASSERT(!playlist.isNull());
        Q_EMIT musicError(playlist, meta, error);

        if (error == Player::NoError) {
//            qDebug() << "set d->syncPlayerResult false " << meta->title;
            d->syncPlayerResult = false;
            if (meta->invalid) {
                meta->invalid = false;
                Q_EMIT musicMetaUpdate(playlist, meta);
            }
            d->continueErrorCount = 0;
            return;
        }

        if (!meta->invalid) {
            meta->invalid = true;
            Q_EMIT musicMetaUpdate(playlist, meta);
        }

//        qDebug() << "check d->syncPlayerResult" << d->syncPlayerResult << meta->title;
        if (d->syncPlayerResult) {
//            qDebug() << "set d->syncPlayerResult false " << meta->title;
            d->syncPlayerResult = false;
            Q_EMIT notifyMusciError(playlist, meta, error);
        } else {
//            qDebug() << "next" << playlist->displayName() << playlist->canNext();
            Q_EMIT notifyMusciError(playlist, meta, error);
//            if (playlist->canNext() && d->continueErrorCount < 5) {
//                DUtil::TimerSingleShot(800, [d, playlist, meta]() {
//                    ++d->continueErrorCount;
//                    d->playNext(playlist, meta);
//                });
//            }
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
        Q_EMIT MediaDatabase::instance()->updateMediaMeta(meta);
    });

    connect(this, &Presenter::playNext, this, &Presenter::onMusicNext);

    Q_EMIT dataLoaded();
}

void Presenter::postAction()
{
    Q_D(Presenter);
    Q_EMIT d->requestInitPlugin();

    auto volume = d->settings->value("base.play.volume").toInt();
    d->player->setVolume(volume);
    Q_EMIT this->volumeChanged(d->player->volume());

    auto mute = d->settings->value("base.play.mute").toBool();
    d->player->setMuted(mute);
    Q_EMIT this->mutedChanged(mute);

    auto playmode = d->settings->value("base.play.playmode").toInt();
    d->player->setMode(static_cast<Player::PlaybackMode>(playmode));
    Q_EMIT this->modeChanged(playmode);

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
            d->lastPlayPosition = position;
            onCurrentPlaylistChanged(lastPlaylist);
            Q_EMIT locateMusic(lastPlaylist, lastMeta);
            d->notifyMusicPlayed(lastPlaylist, lastMeta);

            d->player->setPlayOnLoaded(false);
            d->player->setFadeInOut(false);
            d->player->loadMedia(lastPlaylist, lastMeta);
            d->player->pause();
            QTimer::singleShot(10, [ = ]() {
                d->player->setPosition(position);
            });

            Q_EMIT musicPaused(lastPlaylist, lastMeta);
            if (d->lyricService) {
                Q_EMIT d->requestMetaSearch(lastMeta);
            }
        }
    }

    QString toOpenUri = d->settings->value("base.play.to_open_uri").toString();
    if (!toOpenUri.isEmpty()) {
        MusicSettings::setOption("base.play.to_open_uri", "");
        MusicSettings::sync();
        openUri(QUrl(toOpenUri));
    } else {
        if (d->settings->value("base.play.auto_play").toBool() && !lastPlaylist->isEmpty() && !isMetaLibClear) {
            qDebug() << lastPlaylist->id() << lastPlaylist->displayName();
            if (d->settings->value("base.play.remember_progress").toBool() && !isMetaLibClear) {
                onCurrentPlaylistChanged(lastPlaylist);

//                d->player->setPosition(position);
                onMusicResume(lastPlaylist, lastMeta);

            } else {
                d->lastPlayPosition = 0;
                onCurrentPlaylistChanged(lastPlaylist);
                Q_EMIT locateMusic(lastPlaylist, lastMeta);
                d->notifyMusicPlayed(lastPlaylist, lastMeta);

                d->player->setPlayOnLoaded(false);
                d->player->setFadeInOut(false);
                d->player->loadMedia(lastPlaylist, lastMeta);

                onMusicResume(lastPlaylist, lastMeta);
            }

        }
    }

    auto fadeInOut = d->settings->value("base.play.fade_in_out").toBool();
    d->player->setFadeInOut(fadeInOut);

    if (!isMetaLibClear) {
        Q_EMIT showMusicList(allplaylist);
    }

    // Add playlist
    for (auto playlist : d->playlistMgr->allplaylist()) {
        Q_EMIT playlistAdded(playlist);
    }

    Q_EMIT currentMusicListChanged(lastPlaylist);
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
    onAddMetaToPlaylist(list, metas);
    Q_EMIT MediaLibrary::instance()->meidaFileImported(AllMusicListID, metas);

    onSyncMusicPlay(list, metas.first());
    onCurrentPlaylistChanged(list);
}

void Presenter::onSyncMusicPlay(PlaylistPtr playlist, const MetaPtr meta)
{
    Q_D(Presenter);
    d->syncPlayerResult = true;
    d->continueErrorCount = 0;
    d->syncPlayerMeta = meta;
    onMusicPlay(playlist, meta);
}

void Presenter::onSyncMusicResume(PlaylistPtr playlist, const MetaPtr meta)
{
    Q_D(Presenter);
    d->syncPlayerResult = true;
    d->continueErrorCount = 0;
    d->syncPlayerMeta = meta;
    onMusicResume(playlist, meta);
}

void Presenter::onSyncMusicPrev(PlaylistPtr playlist, const MetaPtr meta)
{
    Q_D(Presenter);
    d->syncPlayerResult = true;
    d->continueErrorCount = 0;
    d->syncPlayerMeta = meta;
    onMusicPrev(playlist, meta);
}

void Presenter::onSyncMusicNext(PlaylistPtr playlist, const MetaPtr meta)
{
    Q_D(Presenter);
    d->syncPlayerResult = true;
    d->continueErrorCount = 0;
    d->syncPlayerMeta = meta;
    onMusicNext(playlist, meta);
}

QList<PlaylistPtr > Presenter::allplaylist()
{
    Q_D(Presenter);
    return d->playlistMgr->allplaylist();
}

PlaylistPtr Presenter::playlist(const QString &id)
{
    Q_D(Presenter);
    return d->playlistMgr->playlist(id);
}

void Presenter::volumeUp()
{
    Q_D(Presenter);
    onVolumeChanged(d->player->volume() + Player::VolumeStep);
    Q_EMIT volumeChanged(d->player->volume());
}

void Presenter::volumeDown()
{
    Q_D(Presenter);
    onVolumeChanged(d->player->volume() - Player::VolumeStep);
    Q_EMIT volumeChanged(d->player->volume());
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
    if (playlist == nullptr)
        return;
    auto playinglist = d->player->curPlaylist();
    MetaPtr next;
    bool t_isLastMeta = false;

    //检查当前播放的是否包含最后一首
    if (playinglist != nullptr) {
        for (auto meta : metalist) {
            if (meta->hash == d->player->activeMeta()->hash && playlist->isLast(meta)) {
                t_isLastMeta = true;
            }
        }
    }

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
            Q_EMIT metaLibraryClean();
        }

        MediaDatabase::instance()->removeMediaMetaList(metalist);
        d->library->removeMediaMetaList(metalist);
    } else if (playlist->id() == AlbumMusicListID || playlist->id() == ArtistMusicListID) {
        auto curPlaylist = d->playlistMgr->playlist(AllMusicListID);
        for (auto &autoPlaylist : allplaylist()) {
            auto meta =  autoPlaylist->removeMusicList(metalist);
            if (autoPlaylist == playinglist) {
                next = meta;
            }
        }

        if (curPlaylist->isEmpty()) {
            qDebug() << "meta library clean";
            onMusicStop(playlist, next);
            Q_EMIT metaLibraryClean();
        }

        MediaDatabase::instance()->removeMediaMetaList(metalist);
        d->library->removeMediaMetaList(metalist);
    } else if (playlist->id() == PlayMusicListID) {
        next = playlist->removeMusicList(metalist);
        if (playlist->isEmpty()) {
            qDebug() << "meta library clean";
            onMusicStop(playlist, next);
            d->player->activePlaylist()->play(nullptr);
        }
    } else {
        next = playlist->removeMusicList(metalist);
    }

    if (playlist == d->player->curPlaylist()
            || playlist->id() == AllMusicListID) {
        //stop music
        for (auto &meta : metalist) {
            if (d->player->isActiveMeta(meta)) {
                if (playinglist->isEmpty() || t_isLastMeta) {
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
    bool t_isLastMeta = false;
    auto playinglist = d->player->curPlaylist();

    //检查当前播放的是否包含最后一首
    for (auto meta : metalist) {
        if (meta->hash == d->player->activeMeta()->hash && playlist->isLast(meta)) {
            t_isLastMeta = true;
        }
    }

    for (auto &playlist : allplaylist()) {
        auto meta = playlist->removeMusicList(metalist);
        if (playlist == playinglist) {
            next = meta;
        }
    }
    if (playinglist->isEmpty()) {
        playinglist->play(nullptr);
    }

    auto allMusicList = d->playlistMgr->playlist(AllMusicListID);

    if (allMusicList->isEmpty()) {
        qDebug() << "meta library clean";
        onMusicStop(playlist, MetaPtr());
        Q_EMIT metaLibraryClean();
    }

    MediaDatabase::instance()->removeMediaMetaList(metalist);

    QMap<QString, QString> trashFiles;
    for (auto &meta : metalist) {
        if (d->player->activeMeta() &&
                (meta->hash == d->player->activeMeta()->hash)) {
            if (playinglist->isEmpty() || t_isLastMeta) {
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
// FIXME:        Q_EMIT d->moniter->fileRemoved(file);
    }

    if (!qEnvironmentVariableIsEmpty("FLATPAK_APPID")) {
        Dtk::Widget::DDesktopServices::trash(trashFiles.keys());
    } else {
        QProcess::startDetached("gvfs-trash", trashFiles.keys());
    }

    d->library->removeMediaMetaList(metalist);
}

void Presenter::onAddToPlaylist(PlaylistPtr playlist,
                                const MetaPtrList metalist)
{
    Q_D(Presenter);

    PlaylistPtr modifiedPlaylist = playlist;
    if (playlist.isNull()) {
        Q_EMIT showPlaylist(true);

        PlaylistMeta info;
        info.editmode = true;
        info.readonly = false;
        info.uuid = d->playlistMgr->newID();
        info.displayName = d->playlistMgr->newDisplayName();
        modifiedPlaylist = d->playlistMgr->addPlaylist(info);
        Q_EMIT playlistAdded(d->playlistMgr->playlist(info.uuid), true);
    } else {
        bool existFlag = true;
        auto allMetas = modifiedPlaylist->allmusic();
        for (auto meta : metalist) {
            bool curExistFlag = false;
            for (auto curMeta : allMetas) {
                if (curMeta->hash == meta->hash) {
                    curExistFlag = true;
                    break;
                }
            }
            if (!curExistFlag) {
                existFlag = false;
                break;
            }
        }
        if (!existFlag)
            Q_EMIT notifyAddToPlaylist(modifiedPlaylist, metalist);
    }

    if (d->playlistMgr->playlist(modifiedPlaylist->id()).isNull()) {
        qCritical() << "no list" << modifiedPlaylist->id();
        return;
    }
    modifiedPlaylist->appendMusicList(metalist);
}

void Presenter::onAddMetaToPlaylist(PlaylistPtr playlist, const MetaPtrList metalist)
{
    Q_D(Presenter);

    PlaylistPtr modifiedPlaylist = playlist;
    if (playlist.isNull()) {
        Q_EMIT showPlaylist(true);

        PlaylistMeta info;
        info.editmode = true;
        info.readonly = false;
        info.uuid = d->playlistMgr->newID();
        info.displayName = d->playlistMgr->newDisplayName();
        modifiedPlaylist = d->playlistMgr->addPlaylist(info);
        Q_EMIT playlistAdded(d->playlistMgr->playlist(info.uuid));
    } else {
        //Q_EMIT notifyAddToPlaylist(modifiedPlaylist, metalist);
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
    Q_EMIT currentMusicListChanged(d->currentPlaylist);
}

void Presenter::onCustomResort(const QStringList &uuids)
{
    Q_D(Presenter);
    d->playlistMgr->onCustomResort(uuids);
}

void Presenter::onRequestMusiclistMenu(const QPoint &pos, char type)
{
    Q_D(Presenter);
    QList<PlaylistPtr > newlists = d->playlistMgr->allplaylist();
    // remove all and fav and search
    newlists.removeAll(d->playlistMgr->playlist(AlbumMusicListID));
    newlists.removeAll(d->playlistMgr->playlist(ArtistMusicListID));
    newlists.removeAll(d->playlistMgr->playlist(AllMusicListID));
    newlists.removeAll(d->playlistMgr->playlist(FavMusicListID));
    newlists.removeAll(d->playlistMgr->playlist(SearchMusicListID));
    newlists.removeAll(d->playlistMgr->playlist(PlayMusicListID));

    auto selectedlist = d->currentPlaylist;
    auto favlist = d->playlistMgr->playlist(FavMusicListID);

    Q_EMIT this->requestMusicListMenu(pos, selectedlist, favlist, newlists, type);
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
    Q_EMIT this->currentMusicListChanged(searchList);
}

void Presenter::onExitSearch()
{
    qDebug() << "exit search";
    Q_D(Presenter);
    qDebug() << d->playlistBeforeSearch;
    if (!d->playlistBeforeSearch.isNull()) {
        d->currentPlaylist = d->playlistBeforeSearch;
        Q_EMIT this->currentMusicListChanged(d->playlistBeforeSearch);
    }
}

void Presenter::onLocateMusicAtAll(const QString &hash)
{
    Q_D(Presenter);
    auto allList = d->playlistMgr->playlist(AllMusicListID);
    d->currentPlaylist = allList;
    Q_EMIT locateMusic(allList, allList->music(hash));
    //    onMusicPlay(allList, allList->music(hash));
}

void Presenter::onChangeSearchMetaCache(const MetaPtr meta, const DMusic::SearchMeta &search)
{
    Q_D(Presenter);

    if (meta->searchID != search.id) {
        qDebug() << "update search id " << search.id;
        meta->searchID = search.id;
        meta->updateSearchIndex();
        Q_EMIT MediaDatabase::instance()->updateMediaMeta(meta);
    }

    Q_EMIT d->requestChangeMetaCache(meta, search);
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

    Q_EMIT playlistAdded(d->playlistMgr->playlist(info.uuid), edit);
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
        Q_EMIT requestImportFiles();
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
    playlist->setPlayingStatus(true);
    Q_EMIT d->play(playlist, toPlayMeta);
}

void Presenter::onMusicPause(PlaylistPtr playlist, const MetaPtr info)
{
    Q_D(Presenter);
    playlist->setPlayingStatus(false);
    Q_EMIT d->pause();
    Q_EMIT musicPaused(playlist, info);
}

void Presenter::onMusicResume(PlaylistPtr playlist, const MetaPtr info)
{
    Q_D(Presenter);
    playlist->setPlayingStatus(true);
    Q_EMIT d->resume(playlist, info);
    d->notifyMusicPlayed(playlist, info);
}

void Presenter::onMusicStop(PlaylistPtr playlist, const MetaPtr meta)
{
    Q_D(Presenter);
    Q_EMIT coverSearchFinished(meta, SearchMeta(), "");
    Q_EMIT lyricSearchFinished(meta, SearchMeta(), "");
    d->player->stop();
    d->metaBufferDetector->onClearBufferDetector();
    playlist->setPlayingStatus(false);
    Q_EMIT this->musicStoped(playlist, meta);
}

void Presenter::onMusicPrev(PlaylistPtr playlist, const MetaPtr meta)
{
    Q_D(Presenter);
    if (playlist.isNull()) {
        return;
    }

    MetaPtr curMeta = meta;
    if (curMeta == nullptr)
        curMeta = playlist->first();
    if (playlist->isEmpty()) {
        Q_EMIT coverSearchFinished(curMeta, SearchMeta(), "");
        Q_EMIT lyricSearchFinished(curMeta, SearchMeta(), "");
        d->player->stop();
        Q_EMIT this->musicStoped(playlist, curMeta);
        return;
    }
    Q_EMIT d->playPrev(playlist, curMeta);
}

void Presenter::onMusicNext(PlaylistPtr playlist, const MetaPtr meta)
{
    Q_D(Presenter);
    if (playlist.isNull()) {
        return;
    }

    MetaPtr curMeta = meta;
    if (curMeta == nullptr)
        curMeta = playlist->first();
    if (playlist->isEmpty()) {
        Q_EMIT coverSearchFinished(curMeta, SearchMeta(), "");
        Q_EMIT lyricSearchFinished(curMeta, SearchMeta(), "");
        d->player->stop();
        Q_EMIT this->musicStoped(playlist, curMeta);
        return;
    }
    Q_EMIT d->playNext(playlist, curMeta);
}

void Presenter::onToggleFavourite(const MetaPtr meta)
{
    Q_D(Presenter);
    if (d->playlistMgr->playlist(FavMusicListID)->contains(meta)) {
        d->playlistMgr->playlist(FavMusicListID)->removeMusicList(MetaPtrList() << meta);
    } else {
        Q_EMIT notifyAddToPlaylist(d->playlistMgr->playlist(FavMusicListID), MetaPtrList() << meta);
        d->playlistMgr->playlist(FavMusicListID)->appendMusicList(MetaPtrList() << meta);
    }
}

void Presenter::onAddMetasFavourite(const MetaPtrList metalist)
{
    Q_D(Presenter);
    auto favMusicList = d->playlistMgr->playlist(FavMusicListID);
    auto favAllMetas = favMusicList->allmusic();
    bool existFlag = false;
    for (auto meta : metalist) {
        for (auto fMeta : favAllMetas) {
            if (meta->hash == fMeta->hash) {
                existFlag = true;
                break;
            }
        }
        if (existFlag)
            break;
    }
    d->playlistMgr->playlist(FavMusicListID)->appendMusicList(metalist);
    if (!existFlag)
        Q_EMIT notifyAddToPlaylist(d->playlistMgr->playlist(FavMusicListID), metalist);
}

void Presenter::onRemoveMetasFavourite(const MetaPtrList metalist)
{
    Q_D(Presenter);
    d->playlistMgr->playlist(FavMusicListID)->removeMusicList(metalist);
}

void Presenter::onChangeProgress(qint64 value, qint64 range)
{
    Q_D(Presenter);
//    auto position = value * d->player->duration() / range;
//    if (position < 0) {
//        qCritical() << "invalid position:" << d->player->media().canonicalUrl() << position;
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
    Q_EMIT d->updateMprisVolume(volume);
}

void Presenter::onPlayModeChanged(int mode)
{
    Q_D(Presenter);
    d->player->setMode(static_cast<Player::PlaybackMode>(mode));
    d->settings->setOption("base.play.playmode", mode);
    d->settings->sync();
    Q_EMIT this->modeChanged(mode);
}

void Presenter::onToggleMute()
{
    Q_D(Presenter);
    d->player->setMuted(! d->player->muted());
    d->settings->setOption("base.play.mute", d->player->muted());

    if (d->player->muted()) {
        Q_EMIT d->updateMprisVolume(0);
    } else {
        Q_EMIT d->updateMprisVolume(d->player->volume());
    }
}

void Presenter::onUpdateMetaCodec(const MetaPtr meta)
{
    Q_D(Presenter);
    Q_EMIT musicMetaUpdate(d->player->activePlaylist(), meta);
}

void Presenter::onPlayall(PlaylistPtr playlist)
{
    Q_D(Presenter);
    int size = playlist->length();
    if (size < 1)
        return;
    MetaPtr meta = playlist->playing();
    if (meta == nullptr)
        meta = playlist->first();
    if (d->player->mode() == Player::Shuffle) {
        int n = qrand() % size;
        meta = playlist->allmusic()[n];
    }

    onMusicPlay(playlist, meta);
}

void Presenter::onResort(PlaylistPtr playlist, int sortType)
{
    playlist->sortBy(static_cast<Playlist::SortType>(sortType));
    if (playlist->sortType() != Playlist::SortByCustom) {
        Q_EMIT this->musicListResorted(playlist);
    }
}

void Presenter::onImportFiles(const QStringList &filelist)
{
    Q_D(Presenter);
    //PlaylistPtr playlist = d->currentPlaylist;
    PlaylistPtr playlist = d->playlistMgr->playlist(AllMusicListID);
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
        metadata.insert(Mpris::metadataToString(Mpris::ArtUrl), meta->coverUrl);

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

    connect(mprisPlayer, &MprisPlayer::stopRequested,
    this, [ = ]() {
        onMusicStop(player->activePlaylist(), player->activeMeta());
        mprisPlayer->setPlaybackStatus(Mpris::Stopped);
        mprisPlayer->setMetadata(QVariantMap());
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
        Q_EMIT this->volumeChanged(volume * 100);
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
        if (!meta) {
            return;
        }
        meta->coverUrl = MetaSearchService::coverUrl(meta);
        if (player->activeMeta().isNull() || meta.isNull()) {
            return;
        }
        if (player->activeMeta()->hash != meta->hash) {
            return;
        }

        QVariantMap metadata = mprisPlayer->metadata();
        metadata.insert(Mpris::metadataToString(Mpris::ArtUrl), meta->coverUrl);
        mprisPlayer->setMetadata(metadata);
    });
}
