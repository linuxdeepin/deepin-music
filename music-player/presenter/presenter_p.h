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

class Presenter;
class LyricService;
class PlaylistManager;
class MediaFileMonitor;
class QSettings;
class PresenterPrivate: public QObject
{
    Q_OBJECT
public:
    PresenterPrivate(Presenter *parent = nullptr);

    void initData();

    PlaylistPtr         playlistBeforeSearch;

    LyricService        *lyricService;
    PlaylistManager     *playlistMgr;
    MediaFileMonitor    *moniter;
    QSettings           *settings;

    Presenter *q_ptr;
    Q_DECLARE_PUBLIC(Presenter);

signals:
    void requestMetaSearch(const MusicMeta&);

    void play(PlaylistPtr playlist, const MusicMeta &info);
    void resume(PlaylistPtr playlist, const MusicMeta &meta);
    void playNext(PlaylistPtr playlist, const MusicMeta &info);
    void playPrev(PlaylistPtr playlist, const MusicMeta &info);
    void stop();
    void pause();
};
