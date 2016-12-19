/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#pragma once

#include <QObject>

#include "../core/util/singleton.h"

#include "../core/playlist.h"

class Presenter;
class ViewPresenter : public QObject, public Singleton<ViewPresenter>
{
    Q_OBJECT
    friend class Singleton<ViewPresenter>;
public:
    explicit ViewPresenter(QObject *parent = 0);
    void binding(Presenter *presenter);

signals:
    void play(PlaylistPtr playlist, const MusicMeta &meta);
    void resume(PlaylistPtr playlist, const MusicMeta &meta);
    void pause(PlaylistPtr playlist, const MusicMeta &meta);
    void next(PlaylistPtr playlist, const MusicMeta &meta);
    void prev(PlaylistPtr playlist, const MusicMeta &meta);
    void changeProgress(qint64 value, qint64 duration);

    void modeChanged(int mode);
    void changePlayMode(int);
    void toggleFavourite(const MusicMeta &meta);
    void togglePlaylist();


signals:
    void playall(PlaylistPtr playlist);
    void addPlaylist(bool editmode);
    void deletePlaylist(PlaylistPtr playlist);
    void selectPlaylist(PlaylistPtr playlist);

signals:
    void musicListRemove(PlaylistPtr playlist, const MusicMetaList &metalist);
    void musicListDelete(PlaylistPtr playlist, const MusicMetaList &metalist);
    void musicClicked(PlaylistPtr playlist, const MusicMeta &info);
    void musicAdd(PlaylistPtr playlist, const MusicMetaList &metalist);
    void resort(PlaylistPtr playlist, int sortType);
    void requestCustomContextMenu(const QPoint &pos);

signals:

    // view controller
    void showImportDialog(const QUrl &path);

    void showImportView();
    void showMusiclistView();
    void toggleLyricView();
    void hidePlaylist();
    void showPlaylist();

    // musiclist controller
    void locateMusic(PlaylistPtr playlist, const MusicMeta &info);
    void musicPlayed(PlaylistPtr playlist, const MusicMeta &info);
    void musicPaused(PlaylistPtr playlist, const MusicMeta &info);
    void musicStoped(PlaylistPtr playlist, const MusicMeta &info);

    void progrossChanged(qint64 pos, qint64 length);
    void lyricChanged(const MusicMeta &meta, const QByteArray &lyricData);
    void coverChanged(const MusicMeta &meta, const QByteArray &coverData);

    void musiclistAdded(PlaylistPtr playlist, const MusicMetaList &metalist);

    void playlistChanged(PlaylistPtr playlist);
    void playlistAdded(PlaylistPtr playlist);
    void playlistRemove(PlaylistPtr playlist);
    void selectedPlaylistChanged(PlaylistPtr playlist);
    void playingPlaylistChanged(PlaylistPtr playlist);

    void musicAdded(PlaylistPtr playlist, const MusicMeta &info);
    void musicRemoved(PlaylistPtr playlist, const MusicMeta &info);

    void lyricSearchFinished(const MusicMeta &, const QByteArray &lyricData);
    void coverSearchFinished(const MusicMeta &, const QByteArray &coverData);
};
