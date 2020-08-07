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
#include <DDialog>

#include <DSettingsOption>
#include <DDesktopServices>
#include <DUtil>

#include <plugininterface.h>
#include <metadetector.h>

#include "../musicapp.h"
#include "util/pinyinsearch.h"
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
    });

    metaBufferDetector = new MetaBufferDetector(this);
    connect(metaBufferDetector, &MetaBufferDetector::metaBuffer, q, [ = ](const QVector<float> &buffer, const QString & hash) {
        Q_EMIT q->metaBuffer(buffer, hash);
    }, Qt::QueuedConnection);

    settings = MusicSettings::instance();

    pdbusinterval =  new QTimer;
    connect(pdbusinterval, &QTimer::timeout,
    this, [ = ]() {
        pdbusinterval->stop();
    });

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
        if (curPlaylist->id() != playlist->id()) {
            auto curAllMetas = playlist->allmusic();
            for (int i = curAllMetas.size() - 1; i >= 0; i--) {
                if (meta == curAllMetas[i])
                    continue;
                if (curAllMetas[i]->invalid && !QFile::exists(curAllMetas[i]->localPath))
                    curAllMetas.removeAt(i);
                else {
                    curAllMetas[i]->invalid = false;
                }
            }
            auto allMsics = curPlaylist->allmusic();
            bool same = true;
            if (allMsics.size() != curAllMetas.size()) {
                same = false;
            } else if (!curAllMetas.isEmpty() && allMsics.size() == curAllMetas.size()) {
                if (allMsics.first()->hash != curAllMetas.first()->hash || allMsics.last()->hash != curAllMetas.last()->hash) {
                    same = false;
                } else {
                    for (int i = 1; i < allMsics.size() - 1; ++i) {
                        if (allMsics[i]->hash != curAllMetas[i]->hash) {
                            same = false;
                            break;
                        }
                    }
                }
            }
            if (!same) {
                curPlaylist->removeMusicList(curPlaylist->allmusic());
                this->thread()->msleep(50);
                curPlaylist->appendMusicList(curAllMetas);
            }
        } else {
            auto curPlaylist = player->curPlaylist();
            auto curAllMetas = curPlaylist->allmusic();
            MetaPtrList curRemoveMetas;
            for (int i = curAllMetas.size() - 1; i >= 0; i--) {
                if (curAllMetas[i]->invalid && !QFile::exists(curAllMetas[i]->localPath))
                    curRemoveMetas.append(curAllMetas[i]);
                else {
                    curAllMetas[i]->invalid = false;
                }
            }
            if (!curRemoveMetas.isEmpty()) {
                curPlaylist->removeMusicList(curRemoveMetas);
            }
        }
        player->playMeta(playlist, meta);
    });
    connect(this, &PresenterPrivate::resume,
    this, [ = ](PlaylistPtr playlist, const MetaPtr meta) {
        auto curPlaylist = player->curPlaylist();
        if (curPlaylist->id() != playlist->id()) {
            auto curAllMetas = playlist->allmusic();
            for (int i = curAllMetas.size() - 1; i >= 0; i--) {
                if (meta == curAllMetas[i])
                    continue;
                if (curAllMetas[i]->invalid && !QFile::exists(curAllMetas[i]->localPath))
                    curAllMetas.removeAt(i);
                else {
                    curAllMetas[i]->invalid = false;
                }
            }
            auto allMsics = curPlaylist->allmusic();
            bool same = true;
            if (allMsics.size() != curAllMetas.size()) {
                same = false;
            } else if (!curAllMetas.isEmpty() && allMsics.size() == curAllMetas.size()) {
                if (allMsics.first()->hash != curAllMetas.first()->hash || allMsics.last()->hash != curAllMetas.last()->hash) {
                    same = false;
                } else {
                    for (int i = 1; i < allMsics.size() - 1; ++i) {
                        if (allMsics[i]->hash != curAllMetas[i]->hash) {
                            same = false;
                            break;
                        }
                    }
                }
            }
            if (!same) {
                curPlaylist->removeMusicList(curPlaylist->allmusic());
                curPlaylist->appendMusicList(curAllMetas);
            }
        } else {
            auto curPlaylist = player->curPlaylist();
            auto curAllMetas = curPlaylist->allmusic();
            MetaPtrList curRemoveMetas;
            for (int i = curAllMetas.size() - 1; i >= 0; i--) {
                if (curAllMetas[i]->invalid && !QFile::exists(curAllMetas[i]->localPath))
                    curRemoveMetas.append(curAllMetas[i]);
                else {
                    curAllMetas[i]->invalid = false;
                }
            }
            if (!curRemoveMetas.isEmpty()) {
                curPlaylist->removeMusicList(curRemoveMetas);
            }
        }
        player->resume(playlist, meta);
    });
    connect(this, &PresenterPrivate::playNext,
    this, [ = ](PlaylistPtr playlist, const MetaPtr meta) {
        auto curPlaylist = player->curPlaylist();
        if (curPlaylist->id() != playlist->id()) {
            auto curAllMetas = playlist->allmusic();
            for (int i = curAllMetas.size() - 1; i >= 0; i--) {
                if (meta == curAllMetas[i])
                    continue;
                if (curAllMetas[i]->invalid && !QFile::exists(curAllMetas[i]->localPath))
                    curAllMetas.removeAt(i);
                else {
                    curAllMetas[i]->invalid = false;
                }
            }
            auto allMsics = curPlaylist->allmusic();
            bool same = true;
            if (allMsics.size() != curAllMetas.size()) {
                same = false;
            } else if (!curAllMetas.isEmpty() && allMsics.size() == curAllMetas.size()) {
                if (allMsics.first()->hash != curAllMetas.first()->hash || allMsics.last()->hash != curAllMetas.last()->hash) {
                    same = false;
                } else {
                    for (int i = 1; i < allMsics.size() - 1; ++i) {
                        if (allMsics[i]->hash != curAllMetas[i]->hash) {
                            same = false;
                            break;
                        }
                    }
                }
            }
            if (!same) {
                curPlaylist->removeMusicList(curPlaylist->allmusic());
                curPlaylist->appendMusicList(curAllMetas);
            }
        } else {
            auto curPlaylist = player->curPlaylist();
            auto curAllMetas = curPlaylist->allmusic();
            MetaPtrList curRemoveMetas;
            for (int i = curAllMetas.size() - 1; i >= 0; i--) {
                if (curAllMetas[i]->invalid && !QFile::exists(curAllMetas[i]->localPath))
                    curRemoveMetas.append(curAllMetas[i]);
                else {
                    curAllMetas[i]->invalid = false;
                }
            }
            if (!curRemoveMetas.isEmpty()) {
                curPlaylist->removeMusicList(curRemoveMetas);
            }
        }
        player->playNextMeta(playlist, meta);
    });
    connect(this, &PresenterPrivate::playPrev,
    this, [ = ](PlaylistPtr playlist, const MetaPtr meta) {
        auto curPlaylist = player->curPlaylist();
        if (curPlaylist->id() != playlist->id()) {
            auto curAllMetas = playlist->allmusic();
            for (int i = curAllMetas.size() - 1; i >= 0; i--) {
                if (meta == curAllMetas[i])
                    continue;
                if (curAllMetas[i]->invalid && !QFile::exists(curAllMetas[i]->localPath))
                    curAllMetas.removeAt(i);
                else {
                    curAllMetas[i]->invalid = false;
                }
            }
            auto allMsics = curPlaylist->allmusic();
            bool same = true;
            if (allMsics.size() != curAllMetas.size()) {
                same = false;
            } else if (!curAllMetas.isEmpty() && allMsics.size() == curAllMetas.size()) {
                if (allMsics.first()->hash != curAllMetas.first()->hash || allMsics.last()->hash != curAllMetas.last()->hash) {
                    same = false;
                } else {
                    for (int i = 1; i < allMsics.size() - 1; ++i) {
                        if (allMsics[i]->hash != curAllMetas[i]->hash) {
                            same = false;
                            break;
                        }
                    }
                }
            }
            if (!same) {
                curPlaylist->removeMusicList(curPlaylist->allmusic());
                curPlaylist->appendMusicList(curAllMetas);
            }
        } else {
            auto curPlaylist = player->curPlaylist();
            auto curAllMetas = curPlaylist->allmusic();
            MetaPtrList curRemoveMetas;
            for (int i = curAllMetas.size() - 1; i >= 0; i--) {
                if (curAllMetas[i]->invalid && !QFile::exists(curAllMetas[i]->localPath))
                    curRemoveMetas.append(curAllMetas[i]);
                else {
                    curAllMetas[i]->invalid = false;
                }
            }
            if (!curRemoveMetas.isEmpty()) {
                curPlaylist->removeMusicList(curRemoveMetas);
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

    if (playlist.isNull() || meta.isNull())
        return;

    MetaPtr favInfo(meta);
    favInfo->favourite = playlistMgr->playlist(FavMusicListID)->contains(meta);
    //    qDebug() << FavMusicListID << meta->title << favInfo->favourite;
    Q_EMIT q->musicPlayed(playlist, favInfo);
}

bool Presenter::containsStr(QString searchText, QString text)
{
    //filter
    text = QString(text).remove("\r").remove("\n");
    bool chineseFlag = false;
    for (auto ch : searchText) {
        if (DMusic::PinyinSearch::isChinese(ch)) {
            chineseFlag = true;
            break;
        }
    }
    if (chineseFlag) {
        return text.contains(searchText);
    } else {
        auto curTextList = DMusic::PinyinSearch::simpleChineseSplit(text);
        QString curTextListStr = "";
        if (!curTextList.isEmpty()) {
            for (auto mText : curTextList) {
                if (mText.contains(searchText, Qt::CaseInsensitive)) {
                    return true;
                }
                curTextListStr += mText;
            }
//            curTextListStr = QString(curTextListStr.remove(" "));
            if (curTextListStr.contains(searchText, Qt::CaseInsensitive)) {
                return true;
            }
        }
        return text.contains(searchText, Qt::CaseInsensitive);
    }
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
        auto allMetas = playlist->allmusic();
        int count = 0;
        for (auto meta : metalist) {
            for (auto curMeta : allMetas) {
                if (curMeta->hash == meta->hash) {
                    count++;
                    break;
                }
            }
        }
        Q_EMIT notifyAddToPlaylist(playlist, metalist, count);

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
        //qDebug() << playlist << playlist->id();
        Q_EMIT musicListRemoved(playlist, metalist);
    });

    connect(d->player, &Player::positionChanged,
    this, [ = ](qint64 position, qint64 duration, qint64 coefficient) {
        d->lastPlayPosition = position;
        Q_EMIT progrossChanged(position, duration, coefficient);
    });

    connect(d->player, &Player::volumeChanged,
            this, &Presenter::volumeChanged);
    connect(d->player, &Player::mutedChanged,
            this, &Presenter::mutedChanged);
//    connect(d->player, &Player::localMutedChanged,
//            this, &Presenter::localMutedChanged);
    connect(this, &Presenter::musicFileMiss,
            d->player, &Player::musicFileMiss);

    connect(d->player, &Player::mediaPlayed,
    this, [ = ](PlaylistPtr playlist, const MetaPtr meta) {
        if (!meta.isNull()) {
            d->settings->setOption("base.play.last_meta", meta->hash);
            if (!playlist.isNull() || playlist->id() != PlayMusicListID)
                d->settings->setOption("base.play.last_playlist", playlist->id());
            d->notifyMusicPlayed(playlist, meta);
            d->requestMetaSearch(meta);
            d->metaBufferDetector->onBufferDetector(meta->localPath, meta->hash);
        }
    });

    connect(d->player, &Player::mediaError,
    this, [ = ](PlaylistPtr playlist, const MetaPtr meta, Player::Error error) {
        Q_D(Presenter);
        Q_ASSERT(!playlist.isNull());

        Q_EMIT musicError(playlist, meta, error);

        if (error == Player::NoError) {

            d->syncPlayerResult = false;
            if (meta->invalid) {
                meta->invalid = false;
                Q_EMIT musicMetaUpdate(playlist, meta);
            }
            d->continueErrorCount = 0;
            return;
        }

        if (meta != nullptr) {
            if (!meta->invalid) {
                meta->invalid = true;
                Q_EMIT musicMetaUpdate(playlist, meta);
            }
        }

        if (d->syncPlayerResult) {

            d->syncPlayerResult = false;
            Q_EMIT notifyMusciError(playlist, meta, error);
        } else {

            Q_EMIT notifyMusciError(playlist, meta, error);
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

    int volume = d->settings->value("base.play.volume").toInt();
    d->player->setVolume(volume);
    Q_EMIT this->volumeChanged(volume);

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

    if (/*d->settings->value("base.play.remember_progress").toBool() && */!isMetaLibClear) {
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
        } else if (lastMetaId == "") {
            lastMeta = nullptr;
        } else {
            lastMeta = lastPlaylist->first();
        }

        if (!lastMeta.isNull()) {
            position = 0;
            if (d->settings->value("base.play.remember_progress").toBool()) {
                position = d->settings->value("base.play.last_position").toInt();
            }
            d->lastPlayPosition = position;
            if (d->settings->value("base.play.remember_progress").toBool())
                onCurrentPlaylistChanged(lastPlaylist);
            Q_EMIT locateMusic(lastPlaylist, lastMeta);
            d->notifyMusicPlayed(lastPlaylist, lastMeta);

            d->player->setPlayOnLoaded(false);
            d->player->setFadeInOut(false);
            d->player->loadMedia(lastPlaylist, lastMeta, position);
            d->metaBufferDetector->onBufferDetector(lastMeta->localPath, lastMeta->hash);

            Q_EMIT musicPaused(lastPlaylist, lastMeta);
            if (d->lyricService) {
                Q_EMIT d->requestMetaSearch(lastMeta);
            }
        }
    }

    auto curPlaylist = d->playlistMgr->playlist(PlayMusicListID);
    QString toOpenUri = d->settings->value("base.play.to_open_uri").toString();
    if (!toOpenUri.isEmpty()) {
        MusicSettings::setOption("base.play.to_open_uri", "");
        MusicSettings::sync();
        openUri(QUrl(toOpenUri));
    } else {
        connect(d->player, &Player::readyToResume, this, [ = ]() {
            if (d->settings->value("base.play.auto_play").toBool() && !curPlaylist->isEmpty() && !lastPlaylist->isEmpty() && !isMetaLibClear) {
                qDebug() << lastPlaylist->id() << lastPlaylist->displayName();
                if (d->settings->value("base.play.remember_progress").toBool() && !isMetaLibClear) {
                    onCurrentPlaylistChanged(lastPlaylist);
                    QTimer::singleShot(50, [ = ]() {
                        onMusicResume(lastPlaylist, lastMeta);
                    });
                } else {
                    d->lastPlayPosition = 0;
                    onCurrentPlaylistChanged(lastPlaylist);
                    Q_EMIT locateMusic(lastPlaylist, lastMeta);
                    d->notifyMusicPlayed(lastPlaylist, lastMeta);

                    d->player->setPlayOnLoaded(false);
                    d->player->setFadeInOut(false);
                    d->player->loadMedia(lastPlaylist, lastMeta);

                    QTimer::singleShot(50, [ = ]() {
                        onMusicResume(lastPlaylist, lastMeta);
                    });
                }
            } else {
                Q_EMIT d->pause();
                d->player->setReady();
            }
        });
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
        Q_EMIT scanFinished(localfile, 0);
        return ;
    }
    auto list = d->playlistMgr->playlist(AllMusicListID);
    onAddMetaToPlaylist(list, metas);
    Q_EMIT MediaLibrary::instance()->meidaFileImported(AllMusicListID, metas);

    if (!d->player->isReady() && d->player->status() != Player::Stopped) {
        auto lastMetaId = d->settings->value("base.play.last_meta").toString();
        MetaPtr lastMeta = MediaLibrary::instance()->meta(lastMetaId);
        bool bsame = false;
        //last_meta maybe null
        if (lastMeta.isNull()) {
            d->player->setDoubleClickStartType(2);
        } else {
            if (lastMeta->hash == metas.first()->hash) {
                bsame = true;
                d->player->setDoubleClickStartType(3);
            } else {
                d->player->setDoubleClickStartType(2);
            }
        }
        connect(d->player, &Player::playerReady,
        this, [ = ]() {
            d->player->setReady();
            QTimer::singleShot(50, [ = ]() {
                if (bsame) {
                    onMusicResume(list, metas.first());
                } else {
                    onSyncMusicPlay(list, metas.first());
                }
            });
        });
    } else {
        onSyncMusicPlay(list, metas.first());
        d->player->setReady();
    }
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
        onMusicPauseNow(activeList, activeMeta);
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
    if (d->player->curPlaylist().isNull() || d->player->curPlaylist()->isEmpty())
        return;
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
    if (d->player->curPlaylist().isNull() || d->player->curPlaylist()->isEmpty())
        return;
    auto alllist = d->playlistMgr->playlist(AllMusicListID);
    auto activeList = d->player->activePlaylist();
    auto activeMeta = d->player->activeMeta();
    if (activeList.isNull()) {
        onPlayall(alllist);
        return;
    }
    onMusicPrev(activeList, activeMeta);
}

void Presenter::onHandleQuit()
{
    handleQuit();
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
            if (!d->player->activeMeta().isNull() && meta->hash == d->player->activeMeta()->hash && playlist->isLast(meta)) {
                t_isLastMeta = true;
            }
        }
    }

    // TODO: do better;
    if (playlist->id() == AllMusicListID || playlist->id() == "musicResult") {
        for (auto &playlist : allplaylist()) {
            auto meta =  playlist->removeMusicList(metalist);
            if (playlist == playinglist) {
                next = meta;
            }
        }

        /*-----Import song interface----*/
        if (playlist->isEmpty()  && playinglist->allmusic().isEmpty()) {

            qDebug() << "meta library clean";
            onMusicStop(playlist, next);
            if (d->playlistMgr->playlist(AllMusicListID)->isEmpty()) {

                qDebug() << "Presenter::onMusiclistRemove Q_EMIT 1";
                Q_EMIT metaLibraryClean();
            }
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
            if (d->playlistMgr->playlist(AllMusicListID)->isEmpty()) {
                qDebug() << "Presenter::onMusiclistRemove Q_EMIT 2";
                Q_EMIT metaLibraryClean();
            }
        }

        MediaDatabase::instance()->removeMediaMetaList(metalist);
        d->library->removeMediaMetaList(metalist);
    } else if (playlist->id() == PlayMusicListID) {
        next = playlist->removeMusicList(metalist);
        if (playlist->isEmpty()) {
            qDebug() << "meta library clean";
            onMusicStop(playlist, next);
            d->settings->setOption("base.play.last_meta", "");
            if (!d->player->activePlaylist().isNull())
                d->player->activePlaylist()->play(nullptr);
        }
    } else {
        next = playlist->removeMusicList(metalist);
    }

    /*-----Judge the condition to remove the song playback switch -----*/
    for (auto &meta : metalist) {
        if (d->player->isActiveMeta(meta) && (playinglist == playlist || playlist->id() == AllMusicListID
                                              || playlist->id() == AlbumMusicListID  || playlist->id() == ArtistMusicListID)) {
            if (playinglist->isEmpty() || t_isLastMeta || next.isNull()) { /*新建歌单清空时停止播放*/
                onMusicStop(playinglist, next);
            } else {
                onMusicPlay(playinglist, next);
            }
        }
    }

    if (playlist->allmusic().size() == 0 &&  playlist->id() != "play") {

        qDebug() << "Presenter::onMusiclistRemove Q_EMIT 3";
        Q_EMIT musicListClear();
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
        if (!d->player->activeMeta().isNull() && meta->hash == d->player->activeMeta()->hash && playlist->isLast(meta)) {
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

    if (!playlist.isNull() && playlist->id() == FavMusicListID) {
        onAddMetasFavourite(metalist);
        return;
    }

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
        auto allMetas = modifiedPlaylist->allmusic();
        int count = 0;
        for (auto meta : metalist) {
            for (auto curMeta : allMetas) {
                if (curMeta->hash == meta->hash) {
                    count++;
                    break;
                }
            }

        }
        Q_EMIT notifyAddToPlaylist(modifiedPlaylist, metalist, count);
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
    newlists.removeAll(d->playlistMgr->playlist(AlbumCandListID));
    newlists.removeAll(d->playlistMgr->playlist(MusicCandListID));
    newlists.removeAll(d->playlistMgr->playlist(ArtistCandListID));
    newlists.removeAll(d->playlistMgr->playlist(AlbumResultListID));
    newlists.removeAll(d->playlistMgr->playlist(MusicResultListID));
    newlists.removeAll(d->playlistMgr->playlist(ArtistResultListID));

    auto selectedlist = d->currentPlaylist;
    auto favlist = d->playlistMgr->playlist(FavMusicListID);

    Q_EMIT this->requestMusicListMenu(pos, selectedlist, favlist, newlists, type);
}

void Presenter::removeListSame(QStringList *list)
{
    for (int i = 0; i < list->count(); i++) {
        for (int k = i + 1; k <  list->count(); k++) {
            if (list->at(i) == list->at(k)) {
                list->removeAt(k);
                k--;
            }
        }
    }
}

void Presenter::onSearchText(const QString &id, const QString &text)
{
    Q_D(Presenter);
    QList<PlaylistPtr> resultlist;
    resultlist.clear();
    if (id == "") {//搜索栏enter按键
        //搜索歌曲候选:<=5个
        auto musicList = d->playlistMgr->playlist(AllMusicListID);;

        auto searchList = d->playlistMgr->playlist(MusicResultListID);
        MetaPtrList musicMetaDataList;

        for (auto &metaData : musicList->allmusic()) {
            if (containsStr(text, metaData->title)) {
                musicMetaDataList.append(metaData);
            }
        }
        searchList->reset(musicMetaDataList);
        //    Q_EMIT searchResult(text, searchList);
        resultlist.push_back(searchList);

        //搜索演唱者候选：<=3
        PlaylistPtr artistList = d->playlistMgr->playlist(ArtistMusicListID);
        auto searchArtistList = d->playlistMgr->playlist(ArtistResultListID);
        MetaPtrList artistMetaDataList;
        searchArtistList->clearTypePtr();

        for (auto &metaData : artistList->playMusicTypePtrList()) {
            if (containsStr(text, metaData->name)) {
                searchArtistList->appendMusicTypePtrListData(metaData);
            }
        }
        //    Q_EMIT searchResult(text, searchArtistList);
        resultlist.push_back(searchArtistList);

        //搜索专辑候选：<=3
        PlaylistPtr albumList = d->playlistMgr->playlist(AlbumMusicListID);
        auto searchAlbumList = d->playlistMgr->playlist(AlbumResultListID);
        MetaPtrList albumMetaDataList;
        searchAlbumList->clearTypePtr();

        for (auto &metaData : albumList->playMusicTypePtrList()) {
            if (containsStr(text, metaData->name)) {
                searchAlbumList->appendMusicTypePtrListData(metaData);
            }
        }
        resultlist.push_back(searchAlbumList);
        Q_EMIT searchResult(text, resultlist, "");
        return;
    }
    if (id == MusicResultListID) { //点击歌曲
        resultlist.clear();
        //搜索歌曲
        auto musicList = d->playlistMgr->playlist(AllMusicListID);;

        auto searchList = d->playlistMgr->playlist(MusicResultListID);
        MetaPtrList musicMetaDataList;
        //该音乐的歌手列表
        QStringList artist, album;
        artist.clear();
        album.clear();
        for (auto &metaData : musicList->allmusic()) {
            if (containsStr(text, metaData->title)) {
                musicMetaDataList.append(metaData);
                if (metaData->album == "") {
                    album.append("未知专辑");
                } else {

                    album.append(metaData->album);
                }
                if (metaData->artist == "") {
                    album.append("未知歌手");
                } else {
                    artist.append(metaData->artist);
                }
            }
        }
        searchList->reset(musicMetaDataList);
        //    Q_EMIT searchResult(text, searchList);
        resultlist.push_back(searchList);

        removeListSame(&artist);
        removeListSame(&album);

        //搜索该音乐的专辑
        PlaylistPtr albumList = d->playlistMgr->playlist(AlbumMusicListID);
        auto searchAlbumList = d->playlistMgr->playlist(AlbumResultListID);
        MetaPtrList albumMetaDataList;
        searchAlbumList->clearTypePtr();

        for (auto &metaData : albumList->playMusicTypePtrList()) {
            for (int i = 0; i < album.length(); i++) {
                if (metaData->name.contains(album.at(i))) {
                    searchAlbumList->appendMusicTypePtrListData(metaData);
                }
            }
        }
        resultlist.push_back(searchAlbumList);

        //搜索该音乐的歌手
        PlaylistPtr artistList = d->playlistMgr->playlist(ArtistMusicListID);
        auto searchArtistList = d->playlistMgr->playlist(ArtistResultListID);
        MetaPtrList artistMetaDataList;
        searchArtistList->clearTypePtr();

        for (auto &metaData : artistList->playMusicTypePtrList()) {
            for (int i = 0; i < artist.length(); i++) {
                if (metaData->name.contains(artist.at(i))) {
                    searchArtistList->appendMusicTypePtrListData(metaData);
                }
            }
        }
        resultlist.push_back(searchArtistList);

        Q_EMIT searchResult(text, resultlist, MusicResultListID);
        return;
    }

    if (id == ArtistResultListID) {
        resultlist.clear();

        //搜索该歌手
        PlaylistPtr artistList = d->playlistMgr->playlist(ArtistMusicListID);
        auto searchArtistList = d->playlistMgr->playlist(ArtistResultListID);
        MetaPtrList artistMetaDataList;
        searchArtistList->clearTypePtr();

        for (auto &metaData : artistList->playMusicTypePtrList()) {
            if (containsStr(text, metaData->name)) {
                searchArtistList->appendMusicTypePtrListData(metaData);
            }
        }
        resultlist.push_back(searchArtistList);

        //搜索该歌手的音乐
        auto musicList = d->playlistMgr->playlist(AllMusicListID);;
        auto searchList = d->playlistMgr->playlist(MusicResultListID);
        MetaPtrList musicMetaDataList;
        //该歌手的专辑列表
        QStringList albumlist;
        albumlist.clear();
        for (auto &metaData : musicList->allmusic()) {
            if (metaData->artist == "") {
                metaData->artist = "未知歌手";
            }
            if (containsStr(text, metaData->artist)) {
                musicMetaDataList.append(metaData);
                if (metaData->album == "") {
                    albumlist.append("未知专辑");
                } else {
                    albumlist.append(metaData->album);
                }
            }
        }
        searchList->reset(musicMetaDataList);
        resultlist.push_back(searchList);
        //去除相同的专辑
        removeListSame(&albumlist);


        //该歌手的专辑
        PlaylistPtr albumList = d->playlistMgr->playlist(AlbumMusicListID);
        auto searchAlbumList = d->playlistMgr->playlist(AlbumResultListID);
        MetaPtrList albumMetaDataList;
        searchAlbumList->clearTypePtr();

        for (auto &metaData : albumList->playMusicTypePtrList()) {
            for (int i = 0; i < albumlist.length(); i++) {
                if (metaData->name.contains(albumlist.at(i))) {
                    searchAlbumList->appendMusicTypePtrListData(metaData);
                }
            }
        }
        resultlist.push_back(searchAlbumList);

        Q_EMIT searchResult(text, resultlist, ArtistResultListID);
        return;
    }

    if (id == AlbumResultListID) {
        resultlist.clear();

        //搜索该专辑
        PlaylistPtr albumList = d->playlistMgr->playlist(AlbumMusicListID);
        auto searchAlbumList = d->playlistMgr->playlist(AlbumResultListID);
        MetaPtrList albumMetaDataList;
        searchAlbumList->clearTypePtr();

        for (auto &metaData : albumList->playMusicTypePtrList()) {
            if (containsStr(text, metaData->name)) {
                searchAlbumList->appendMusicTypePtrListData(metaData);
            }
        }
        resultlist.push_back(searchAlbumList);

        //搜索该专辑的音乐
        auto musicList = d->playlistMgr->playlist(AllMusicListID);;
        auto searchList = d->playlistMgr->playlist(MusicResultListID);
        MetaPtrList musicMetaDataList;
        //该专辑的歌手列表
        QStringList artist;
        artist.clear();
        for (auto &metaData : musicList->allmusic()) {
            if (metaData->album == "") {
                metaData->album = "未知专辑";
            }
            if (containsStr(text, metaData->album)) {
                musicMetaDataList.append(metaData);
                if (metaData->artist == "") {
                    artist.append("未知歌手");
                } else {
                    artist.append(metaData->artist);
                }
            }
        }
        searchList->reset(musicMetaDataList);
        resultlist.push_back(searchList);
        //去除相同的歌手
        removeListSame(&artist);

        //搜索该专辑的歌手
        auto artistList = d->playlistMgr->playlist(ArtistMusicListID);
        auto searchArtistList = d->playlistMgr->playlist(ArtistResultListID);
        MetaPtrList artistMetaDataList;
        searchArtistList->clearTypePtr();

        for (auto &metaData : artistList->playMusicTypePtrList()) {
            for (int i = 0; i < artist.length(); i++) {
                if (metaData->name.contains(artist.at(i))) {
                    searchArtistList->appendMusicTypePtrListData(metaData);
                }
            }
        }
        resultlist.push_back(searchArtistList);

        Q_EMIT searchResult(text, resultlist, AlbumResultListID);
        return;
    }

}

void Presenter::onSearchCand(const QString text)
{
    Q_D(Presenter);
    //搜索歌曲候选:<=5个
    auto musicList = d->playlistMgr->playlist(AllMusicListID);
    PlaylistPtr musicListResult = nullptr;
    int count = 0;
    MetaPtrList musicMetaDataList;
    for (auto &metaData : musicList->allmusic()) {
        if (containsStr(text, metaData->title)) {
            musicMetaDataList.append(metaData);
            count ++;
        }
        if (count >= 5) {
            count = 0;
            break;
        }
    }
    auto searchList = d->playlistMgr->playlist(MusicCandListID);
    searchList->reset(musicMetaDataList);
    Q_EMIT searchCand(text, searchList);

    //搜索演唱者候选：<=3
    count = 0;
    MetaPtrList artistMetaDataList;
    PlaylistPtr artistList = d->playlistMgr->playlist(ArtistMusicListID);
    auto searchArtistList = d->playlistMgr->playlist(ArtistCandListID);
    searchArtistList->clearTypePtr();
    for (auto &metaData : artistList->playMusicTypePtrList()) {
        if (containsStr(text, metaData->name)) {
            searchArtistList->appendMusicTypePtrListData(metaData);
            count ++;
        }
        if (count >= 3) {
            break;
        }
    }
    Q_EMIT searchCand(text, searchArtistList);

    //搜索专辑候选：<=3
    count = 0;
    MetaPtrList albumMetaDataList;
    PlaylistPtr albumList = d->playlistMgr->playlist(AlbumMusicListID);
    auto searchAlbumList = d->playlistMgr->playlist(AlbumCandListID);
    searchAlbumList->clearTypePtr();
    for (auto &metaData : albumList->playMusicTypePtrList()) {
        if (containsStr(text, metaData->name)) {
            searchAlbumList->appendMusicTypePtrListData(metaData);
            count ++;
        }
        if (count >= 3) {
            break;
        }
    }
    Q_EMIT searchCand(text, searchAlbumList);
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
        //为空则播放所有音乐
        playlist = d->playlistMgr->playlist(AllMusicListID);
    }

    d->player->setPlayOnLoaded(true);
    if (0 == d->playlistMgr->playlist(AllMusicListID)->length()) {
        //所有音乐为空则导入音乐
        Q_EMIT requestImportFiles();
        return;
    }

    auto oldPlayinglist = d->player->activePlaylist();
    if (!oldPlayinglist.isNull() && oldPlayinglist != playlist) {
        qDebug() << "stop old list" << oldPlayinglist->id() << playlist->id();
        oldPlayinglist->play(MetaPtr());
    }

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
    auto alllists = d->playlistMgr->allplaylist();
    for (auto curList : alllists) {
        if (!curList.isNull())
            curList->setPlayingStatus(true);
    }
    Q_EMIT d->play(playlist, toPlayMeta);
}

void Presenter::onMusicPause(PlaylistPtr playlist, const MetaPtr info)
{
    Q_D(Presenter);
    auto alllists = d->playlistMgr->allplaylist();
    for (auto curList : alllists) {
        if (!curList.isNull())
            curList->setPlayingStatus(false);
    }
    Q_EMIT d->pause();
    Q_EMIT musicPaused(playlist, info);
}

void Presenter::onMusicPauseNow(PlaylistPtr playlist, const MetaPtr meta)
{
    Q_D(Presenter);
    d->player->pauseNow();
    auto alllists = d->playlistMgr->allplaylist();
    for (auto curList : alllists) {
        if (!curList.isNull())
            curList->setPlayingStatus(false);
    }
    Q_EMIT musicPaused(playlist, meta);
}

void Presenter::onMusicResume(PlaylistPtr playlist, const MetaPtr info)
{
    Q_D(Presenter);
    if (!d->player->isReady()) {
        return;
    }
    auto alllists = d->playlistMgr->allplaylist();
    for (auto curList : alllists) {
        if (!curList.isNull())
            curList->setPlayingStatus(true);
    }
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
    auto alllists = d->playlistMgr->allplaylist();
    for (auto curList : alllists) {
        if (!curList.isNull())
            curList->setPlayingStatus(false);
    }
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
        Q_EMIT notifyAddToPlaylist(d->playlistMgr->playlist(FavMusicListID), MetaPtrList() << meta, 0);
        d->playlistMgr->playlist(FavMusicListID)->appendMusicList(MetaPtrList() << meta);
    }
}

void Presenter::onAddMetasFavourite(const MetaPtrList metalist)
{
    Q_D(Presenter);
    auto favMusicList = d->playlistMgr->playlist(FavMusicListID);
    auto favAllMetas = favMusicList->allmusic();
    int count = 0;
    for (auto meta : metalist) {
        for (auto fMeta : favAllMetas) {
            if (meta->hash == fMeta->hash) {
                count++;
                break;
            }
        }
    }
    d->playlistMgr->playlist(FavMusicListID)->appendMusicList(metalist);
    Q_EMIT notifyAddToPlaylist(d->playlistMgr->playlist(FavMusicListID), metalist, count);
}

void Presenter::onRemoveMetasFavourite(const MetaPtrList metalist)
{
    Q_D(Presenter);
    d->playlistMgr->playlist(FavMusicListID)->removeMusicList(metalist);
}

void Presenter::onChangeProgress(qint64 value, qint64 range)
{
    Q_D(Presenter);

    if (range <= 0)
        return;
    if (value > range) {
        //d->player->playNextMeta();
        onMusicNext(d->player->activePlaylist(), d->player->activeMeta());
        //mprisPlayer->setPlaybackStatus(Mpris::Playing);
        return;
    }
    d->player->setIOPosition(value, range);

    auto position = value * d->player->duration() / range;
    d->player->setPosition(position);
}

void Presenter::onChangePosition(qint64 value, qint64 range)
{
    Q_D(Presenter);

    if (range <= 0)
        return;

    if (value > range) {
        //d->player->playNextMeta();
        onMusicNext(d->player->activePlaylist(), d->player->activeMeta());
        return;
    }

    d->player->setIOPosition(value, range);
    auto position = value * d->player->duration() / range;
    d->player->setPosition(position);
}

void Presenter::onVolumeChanged(int volume)
{
    Q_D(Presenter);
    d->player->setVolume(volume);
    Q_EMIT d->updateMprisVolume(volume);
}

void Presenter::UpdateVolume(int volume)
{
    Q_D(Presenter);
    d->player->updateVolume(volume);
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
//    d->player->setMuted(!d->player->muted());
//    if (d->player->muted()) {
//        Q_EMIT d->updateMprisVolume(0);
//    } else {
//        Q_EMIT d->updateMprisVolume(d->player->volume());
//    }
    if (d->player->status() == Player::Paused ||
            d->player->status() == Player::Playing) {
        if (d->player->isValidDbusMute())
            d->player->setMuted(!d->player->muted());

        if (d->player->muted()) {
            Q_EMIT d->updateMprisVolume(0);
        } else {
            Q_EMIT d->updateMprisVolume(d->player->volume());
        }
    } else {
        //local toggle
        //Q_EMIT d->player->localMutedChanged();
        Q_EMIT localMutedChanged(0);
    }

}

void Presenter::onLocalToggleMute()
{
    Q_D(Presenter);
    if (d->player->isValidDbusMute()) {
        d->player->setMuted(!d->player->muted());
    } else {
        //Q_EMIT d->player->localMutedChanged();
        Q_EMIT localMutedChanged(1);
    }
}

void Presenter::localMuteChanged(bool mute)
{
    Q_D(Presenter);
    d->player->setLocalMuted(mute);
}

void Presenter::onFadeInOut()
{
    Q_D(Presenter);
    auto fadeInOut = d->settings->value("base.play.fade_in_out").toBool();
    d->player->setFadeInOut(fadeInOut);
}

void Presenter::onUpdateMetaCodec(const QString &preTitle, const QString &preArtist, const QString &preAlbum, const MetaPtr meta)
{
    Q_D(Presenter);
    if (meta.isNull() || (preTitle == meta->title && preArtist == meta->artist && preAlbum == meta->album))
        return;
    Q_EMIT musicMetaUpdate(d->player->activePlaylist(), meta);

    if ((!preArtist.isEmpty() && preArtist != meta->artist) || (!preAlbum.isEmpty() && preAlbum != meta->album)) {
        auto artistPlaylist = d->playlistMgr->playlist(ArtistMusicListID);
        auto artistTypePtrList = artistPlaylist->playMusicTypePtrList();
        for (auto curType : artistTypePtrList) {
            if (curType->name == preArtist) {
                curType->name = meta->artist;
                break;
            }
        }
        auto albumPlaylist = d->playlistMgr->playlist(AlbumMusicListID);
        auto albumTypePtrList = albumPlaylist->playMusicTypePtrList();
        for (auto curType : albumTypePtrList) {
            if (curType->name == preAlbum) {
                curType->name = meta->album;
                curType->extraName = meta->artist;
                break;
            }
        }
    }
}

void Presenter::onPlayall(PlaylistPtr playlist)
{
    Q_D(Presenter);

    if (playlist->id() == AlbumMusicListID || playlist->id() == ArtistMusicListID) {

        PlaylistPtr curPlaylist = playlist;
        auto playMusicTypePtrList = curPlaylist->playMusicTypePtrList();
        auto PlayMusicTypePtr = playMusicTypePtrList[0];
        QString name = PlayMusicTypePtr->name;

        if (curPlaylist.isNull()) {
            qWarning() << "can not player emptry playlist";
            return;
        }

        MetaPtr curMeta;
        for (auto TypePtr : curPlaylist->playMusicTypePtrList()) {
            if (TypePtr->name == name) {

                auto metaHash  =  TypePtr->playlistMeta.sortMetas.at(0);
                if (TypePtr->playlistMeta.metas.contains(metaHash)) {
                    curMeta = TypePtr->playlistMeta.metas[metaHash];
                }
            }
        }
        onMusicPlay(playlist, curMeta);

    }  else {

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
}

void Presenter::onResort(PlaylistPtr playlist, int sortType)
{
    playlist->sortBy(static_cast<Playlist::SortType>(sortType));
    if (playlist->sortType() != Playlist::SortByCustom) {
        Q_EMIT this->musicListResorted(playlist);
    }
}

void Presenter::onImportFiles(const QStringList &filelist, PlaylistPtr playlist)
{
    Q_D(Presenter);
    //PlaylistPtr playlist = d->currentPlaylist;
    PlaylistPtr curPlaylist = playlist;
//    bool flag = false;
    if (playlist == nullptr) {
        curPlaylist = d->playlistMgr->playlist(AllMusicListID);
//        flag = true;
    }
    requestImportPaths(curPlaylist, filelist);
    auto curPlayerlist = d->player->curPlaylist();
    if (curPlayerlist->isEmpty()/* && flag*/) {
        d->player->setActivePlaylist(curPlaylist);
    }
    curPlayerlist->appendMusicList(curPlaylist->allmusic());
    return;
}

void Presenter::onSpeechPlayMusic(const QString music)
{
    Q_D(Presenter);
    PlaylistPtr musicList = d->playlistMgr->playlist(AllMusicListID);
    PlaylistPtr curPlayList  = d->playlistMgr->playlist(PlayMusicListID);
    bool find = false;
    MetaPtrList musicMetaDataList;
    MetaPtr playMetaData;
    for (auto &metaData : musicList->allmusic()) {
        if (containsStr(music, metaData->title)) {
            musicMetaDataList.append(metaData);
            playMetaData = metaData;
            find = true;
        }
    }
    if (find) {
        curPlayList->reset(musicMetaDataList);
//        d->player->setActivePlaylist(curPlayList);
        onSyncMusicPlay(curPlayList, playMetaData);
        Q_EMIT sigSpeedResult(1, true);
    } else {
        Q_EMIT sigSpeedResult(1, false);
    }
}

void Presenter::onSpeechPlayArtist(const QString artist)
{
    Q_D(Presenter);
    MetaPtrList artistMetaDataList;
    PlaylistPtr musicList = d->playlistMgr->playlist(AllMusicListID);
    PlaylistPtr curPlayList = d->playlistMgr->playlist(PlayMusicListID);
    bool find = false;
    MetaPtrList musicMetaDataList;
    MetaPtr playMetaData;
    for (auto &metaData : musicList->allmusic()) {
        if (containsStr(artist, metaData->artist)) {
            musicMetaDataList.append(metaData);
            find = true;
        }
    }
    if (find) {
        playMetaData = musicMetaDataList.first();
        curPlayList->reset(musicMetaDataList);
        d->player->setActivePlaylist(curPlayList);
        onSyncMusicPlay(curPlayList, playMetaData);
        Q_EMIT sigSpeedResult(2, true);
    } else {
        Q_EMIT sigSpeedResult(2, false);
    }
}

void Presenter::onSpeechPlayArtistMusic(const QString artist, const QString music)
{
    Q_D(Presenter);
    MetaPtrList artistMetaDataList;
    PlaylistPtr musicList = d->playlistMgr->playlist(AllMusicListID);
    PlaylistPtr curPlayList = d->playlistMgr->playlist(PlayMusicListID);
    bool find = false;
    MetaPtrList musicMetaDataList;
    MetaPtr playMetaData;
    for (auto &metaData : musicList->allmusic()) {
        if (containsStr(artist, metaData->artist)) {
            if (containsStr(music, metaData->title)) {
                musicMetaDataList.append(metaData);
            }
            find = true;
        }
    }
    if (find) {
        playMetaData = musicMetaDataList.first();
        curPlayList->reset(musicMetaDataList);
//        d->player->setActivePlaylist(curPlayList);
        onSyncMusicPlay(curPlayList, playMetaData);
        Q_EMIT sigSpeedResult(3, true);
    } else {
        Q_EMIT sigSpeedResult(3, false);
    }
}

void Presenter::onSpeechPlayFaverite()
{
    Q_D(Presenter);
    MetaPtrList artistMetaDataList;
    PlaylistPtr musicList = d->playlistMgr->playlist(FavMusicListID);
    PlaylistPtr curPlayList = d->playlistMgr->playlist(PlayMusicListID);
    MetaPtrList musicMetaDataList;
    MetaPtr playMetaData;
    if (musicList->allmusic().size() == 0) {
        Q_EMIT sigSpeedResult(4, false);
    } else {
        playMetaData = musicList->allmusic().first();
        curPlayList->reset(musicList->allmusic());
        d->player->setActivePlaylist(curPlayList);
        onSyncMusicPlay(curPlayList, playMetaData);
        Q_EMIT sigSpeedResult(4, true);
    }
}

void Presenter::onSpeechPlayCustom(const QString listName)
{
    Q_D(Presenter);
    MetaPtrList artistMetaDataList;
    PlaylistPtr musicList ;
    PlaylistPtr curPlayList = d->playlistMgr->playlist(PlayMusicListID);
    bool find = false;
    for (auto mList : d->playlistMgr->allplaylist()) {
        if (containsStr(mList->displayName(), listName)) {
            musicList = mList;
            find = true;
        }
    }
    if (find) {
        MetaPtrList musicMetaDataList;
        MetaPtr playMetaData;
        playMetaData = musicList->allmusic().first();
        curPlayList->reset(musicList->allmusic());
        d->player->setActivePlaylist(curPlayList);
        onSyncMusicPlay(curPlayList, playMetaData);
        Q_EMIT sigSpeedResult(5, true);
    } else {
        Q_EMIT sigSpeedResult(5, false);
    }
}

void Presenter::onSpeechPlayRadom()
{
    Q_D(Presenter);
    PlaylistPtr musicList = d->playlistMgr->playlist(AllMusicListID);
    PlaylistPtr curPlayList  = d->playlistMgr->playlist(PlayMusicListID);
    MetaPtrList musicMetaDataList;
    MetaPtr playMetaData;
    int count = musicList->allmusic().size();
    if (count == 0) {
        Q_EMIT sigSpeedResult(6, false);
    } else {
        int index = qrand() % count;
        playMetaData = musicList->allmusic().at(index);
        musicMetaDataList.append(playMetaData);
        curPlayList->reset(musicMetaDataList);
        onSyncMusicPlay(curPlayList, playMetaData);
        Q_EMIT sigSpeedResult(6, true);
    }

}

void Presenter::onSpeechPause()
{
    Q_D(Presenter);
    onMusicPause(d->currentPlaylist, nullptr);
}

void Presenter::onSpeechStop()
{
    Q_D(Presenter);
    onMusicStop(d->currentPlaylist, d->currentPlaylist->playing());
}

void Presenter::onSpeechResume()
{
    Q_D(Presenter);
    onSyncMusicResume(d->currentPlaylist, d->currentPlaylist->playing());
}

void Presenter::onSpeechPrevious()
{
    Q_D(Presenter);
    onSyncMusicPrev(d->currentPlaylist, d->currentPlaylist->playing());
}

void Presenter::onSpeechNext()
{
    Q_D(Presenter);
    onSyncMusicNext(d->currentPlaylist, d->currentPlaylist->playing());
}

void Presenter::onSpeechFavorite()
{
    Q_D(Presenter);
    onToggleFavourite(d->currentPlaylist->playing());
}

void Presenter::onSpeechunFaverite()
{
    Q_D(Presenter);
    onToggleFavourite(d->currentPlaylist->playing());
}

void Presenter::onSpeechsetMode(const int mode)
{
//    Q_D(Presenter);
    onPlayModeChanged(mode);
}

void Presenter::onScanMusicDirectory()
{
    auto musicDir =  QStandardPaths::standardLocations(QStandardPaths::MusicLocation);
    qWarning() << "scan" << musicDir;
    PlaylistPtr playlist = nullptr;
    onImportFiles(musicDir, playlist);

    Q_D(Presenter);
    auto curPlaylist = d->playlistMgr->playlist(AllMusicListID);
    d->player->setActivePlaylist(curPlaylist);
    auto curPlayerlist = d->player->curPlaylist();
    curPlayerlist->appendMusicList(curPlaylist->allmusic());
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
//        mprisPlayer->setPlaybackStatus(Mpris::Stopped);
        mprisPlayer->setPlaybackStatus(Mpris::Playing);
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
        if (!d->pdbusinterval->isActive()) {
            d->pdbusinterval->start(50);
        } else
            return;
        /************************************************************
         * if no song in music,do not import songs when dbus msg comes
         * ***********************************************************/
        if (d->playlistMgr->playlist(AllMusicListID)->length() == 0) {
            return;
        }
        //set player ready when dbus msg comes
        d->player->setReady();

        if (d->player->status() == Player::Paused) {
            onMusicResume(player->activePlaylist(), player->activeMeta());
        } else {
            if (d->player->status() != Player::Playing) {
                onMusicPlay(player->activePlaylist(), player->activeMeta());
            }
        }
        mprisPlayer->setPlaybackStatus(Mpris::Playing);
    });

    connect(mprisPlayer, &MprisPlayer::pauseRequested,
    this, [ = ]() {
        if (!d->pdbusinterval->isActive()) {
            d->pdbusinterval->start(50);
        } else
            return;

        if (d->player->activePlaylist().isNull() &&  d->player != nullptr) {
            d->player->pauseNow();
            return;
        }
        if (d->player->status() == Player::Playing)
            onMusicPauseNow(player->activePlaylist(), player->activeMeta());
        mprisPlayer->setPlaybackStatus(Mpris::Paused);
    });

    connect(mprisPlayer, &MprisPlayer::nextRequested,
    this, [ = ]() {
        if (d->player->activePlaylist().isNull()) {
            return;
        }
        if (!d->pdbusinterval->isActive()) {
            d->pdbusinterval->start(50);
        } else
            return;
        /************************************************************
         * if no song in music,do not play songs when dbus msg comes
         * ***********************************************************/
        if (d->playlistMgr->playlist(AllMusicListID)->length() == 0) {
            return;
        }
        //set player ready when dbus msg comes
        d->player->setReady();
        onMusicNext(player->activePlaylist(), player->activeMeta());
        mprisPlayer->setPlaybackStatus(Mpris::Playing);
    });

    connect(mprisPlayer, &MprisPlayer::previousRequested,
    this, [ = ]() {
        if (d->player->activePlaylist().isNull()) {
            return;
        }
        if (!d->pdbusinterval->isActive()) {
            d->pdbusinterval->start(50);
        } else
            return;
        /************************************************************
         * if no song in music,do not play songs when dbus msg comes
         * ***********************************************************/
        if (d->playlistMgr->playlist(AllMusicListID)->length() == 0) {
            return;
        }
        //set player ready when dbus msg comes
        d->player->setReady();
        onMusicPrev(player->activePlaylist(), player->activeMeta());
        mprisPlayer->setPlaybackStatus(Mpris::Playing);
    });

    connect(mprisPlayer, &MprisPlayer::volumeRequested,
    this, [ = ](double volume) {
        Q_UNUSED(volume)
//        onVolumeChanged(volume * 100);
//        Q_EMIT this->volumeChanged(volume * 100);
    });

    connect(d, &PresenterPrivate::updateMprisVolume,
    this, [ = ](int volume) {
        mprisPlayer->setVolume(volume / 100);
    });

    connect(this, &Presenter::progrossChanged,
    this, [ = ](qint64 pos, qint64) {
        mprisPlayer->setPosition(pos);
    });

    connect(mprisPlayer, &MprisPlayer::seekRequested,
    this, [ = ](qlonglong offset) {
        if (!d->pdbusinterval->isActive()) {
            d->pdbusinterval->start(50);
        } else
            return;
        onChangeProgress(d->player->position() + offset, d->player->duration());
    });

    connect(mprisPlayer, &MprisPlayer::setPositionRequested,
    this, [ = ](const QDBusObjectPath & trackId, qlonglong offset) {
        Q_UNUSED(trackId)
        onChangePosition(offset, d->player->duration());
    });

    /*********************************************
     *  set dbus PlaybackStatus as pause state
     * *******************************************/
    connect(d, &PresenterPrivate::pause,
    this, [ = ]() {
        mprisPlayer->setPlaybackStatus(Mpris::Paused);
    });

    connect(this, &Presenter::musicStoped,
    this, [ = ]() {
        mprisPlayer->setPlaybackStatus(Mpris::Stopped);
        mprisPlayer->setMetadata(QVariantMap());
    });

    connect(d->player, &Player::playbackStatusChanged,
    this, [ = ](Player::PlaybackStatus stat) {
        mprisPlayer->setPlaybackStatus(static_cast<Mpris::PlaybackStatus>(stat));
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

