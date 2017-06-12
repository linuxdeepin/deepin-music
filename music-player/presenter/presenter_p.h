/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#pragma once

#include "../core/playlist.h"
#include "../core/settings.h"
#include "searchmeta.h"

class Presenter;
class MetaSearchService;
class PlaylistManager;
class Player;
class MediaLibrary;
class PresenterPrivate: public QObject
{
    Q_OBJECT
public:
    PresenterPrivate(Presenter *parent = nullptr);

    void initBackend();

    PlaylistPtr         playlistBeforeSearch;
    PlaylistPtr         currentPlaylist;

    bool                syncPlayerResult    = false;
    MetaPtr             syncPlayerMeta;

    MetaSearchService   *lyricService   = nullptr;
    PlaylistManager     *playlistMgr    = nullptr;
    Player              *player         = nullptr;
    MediaLibrary        *library        = nullptr;
    AppSettings *settings;

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
