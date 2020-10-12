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

#pragma once

#include "../core/playlist.h"
#include "../core/musicsettings.h"
#include "searchmeta.h"
class QTimer;
class Presenter;
class MetaSearchService;
class PlaylistManager;
class Player;
class MediaLibrary;
class MetaBufferDetector;
class Transfer;
class PresenterPrivate: public QObject
{
    Q_OBJECT
public:
    explicit PresenterPrivate(Presenter *parent = nullptr);

    void initBackend();
    void quickReadSql();
    void notifyMusicPlayed(PlaylistPtr playlist, const MetaPtr meta);

    PlaylistPtr         playlistBeforeSearch;
    PlaylistPtr         currentPlaylist;

    bool                syncPlayerResult    = false;
    int                 continueErrorCount  = 0;
    qint64              lastPlayPosition    = 0;
    MetaPtr             syncPlayerMeta;

    MetaSearchService   *lyricService   = nullptr;
    PlaylistManager     *playlistMgr    = nullptr;
    Player              *player         = nullptr;
    MediaLibrary        *library        = nullptr;
    MusicSettings         *settings       = nullptr;
    Transfer            *transfer       = nullptr;
    QTimer              *pdbusinterval  = nullptr; //to forbid mult times dbus invoke
    MetaBufferDetector  *metaBufferDetector = nullptr;

    Presenter *q_ptr;
    Q_DECLARE_PUBLIC(Presenter)

signals:
    void requestInitPlugin();
    void requestMetaSearch(const MetaPtr);
    void requestChangeMetaCache(const MetaPtr meta, const DMusic::SearchMeta &search);

    void updateMprisVolume(int volume);

    void play(PlaylistPtr playlist, const MetaPtr info);
    void resume(PlaylistPtr playlist, const MetaPtr meta);
    void playNext(PlaylistPtr playlist, const MetaPtr info);
    void playPrev(PlaylistPtr playlist, const MetaPtr info);
    void stop();
    void pause();
};
