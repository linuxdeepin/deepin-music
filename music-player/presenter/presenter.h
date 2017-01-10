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
#include <QScopedPointer>

#include "../core/music.h"
#include "../core/playlist.h"

#include <MprisPlayer>

class Playlist;

class PresenterPrivate;
class Presenter : public QObject
{
    Q_OBJECT
public:
    explicit Presenter(QObject *parent = 0);
    ~Presenter();

    PlaylistPtr allMusicPlaylist();
    PlaylistPtr favMusicPlaylist();
    PlaylistPtr lastPlaylist();
    QList<PlaylistPtr > allplaylist();
    int playMode();
    void volumeup();
    void volumedown();
    void togglePaly();
    void next();
    void prev();


    void initMpris(MprisPlayer* mprisPlayer);

    void prepareData();

    void loadConfig();

    void postAction();
    //! action signal
signals:
    //! ui: request import dialog
    void importMediaFiles(PlaylistPtr playlist, const QStringList &filelist);
    void meidaFilesImported(PlaylistPtr playlist, MusicMetaList metalist);

    //! notify signal
signals:
    void dataLoaded();

    //! ui: control
    void requestImportFiles();
    void setPlaylistVisible(bool visible);
    void showMusiclist();

    //! ui: menu
    void musiclistMenuRequested(const QPoint &pos,
                                PlaylistPtr selectedlist,
                                PlaylistPtr favlist,
                                QList<PlaylistPtr >newlists);

    //! from playlist manager
    void playlistAdded(PlaylistPtr);
    void playlistRemove(PlaylistPtr);
    void currentPlaylistChanged(PlaylistPtr);
    void playlistResorted(PlaylistPtr);
    void playingPlaylistChanged(PlaylistPtr);

    //! from playlist manager
    void musicAdded(PlaylistPtr playlist, const MusicMeta &info);
    void musiclistAdded(PlaylistPtr playlist, const MusicMetaList &metalist);
    void musicRemoved(PlaylistPtr playlist, const MusicMeta &info);

    //! from control
    void musicPlayed(PlaylistPtr playlist, const MusicMeta &meta);
    void musicError(PlaylistPtr playlist, const MusicMeta &meta, int error);
    void musicPaused(PlaylistPtr playlist, const MusicMeta &meta);
    void musicStoped(PlaylistPtr playlist, const MusicMeta &meta);
    void musicMetaUpdate(PlaylistPtr playlist, const MusicMeta &meta);
    void notifyMusciError(PlaylistPtr playlist, const MusicMeta &meta, int error);
    void progrossChanged(qint64 pos, qint64 length);
    void volumeChanged(int volume);
    void mutedChanged(bool muted);

    void locateMusic(PlaylistPtr playlist, const MusicMeta &info);
    //! from lyricservice
    void requestContextSearch(const QString &context);
    void lyricSearchFinished(const MusicMeta &, const QByteArray &lyricData);
    void coverSearchFinished(const MusicMeta &, const QByteArray &coverData);
    void contextSearchFinished(const QString &context, const MusicMetaList &metalist);
    void changeMetaCache(const MusicMeta &meta);

    //! meta info
    void metaInfoClean();
    void notifyAddToPlaylist(PlaylistPtr playlist, const MusicMetaList &info);

public slots:
    //! music control interface
    void onSyncMusicPlay(PlaylistPtr playlist, const MusicMeta &meta);
    void onMusicPlay(PlaylistPtr playlist, const MusicMeta &meta);
    void onMusicPause(PlaylistPtr playlist, const MusicMeta &info);
    void onMusicResume(PlaylistPtr playlist, const MusicMeta &info);
    void onMusicStop(PlaylistPtr playlist, const MusicMeta &info);
    void onMusicPrev(PlaylistPtr playlist, const MusicMeta &meta);
    void onMusicNext(PlaylistPtr playlist, const MusicMeta &meta);
    void onToggleFavourite(const MusicMeta &info);
    void onChangeProgress(qint64 value, qint64 range);
    void onVolumeChanged(int volume);
    void onPlayModeChanged(int mode);
    void onToggleMute();

    void onUpdateMetaCodec(const MusicMeta &meta);

    //! music list
    void onPlayall(PlaylistPtr playlist);
    void onResort(PlaylistPtr playlist, int sortType);

    //! UI: playlist manager interface
    void onAddToPlaylist(PlaylistPtr playlist, const MusicMetaList &metalist);
    void onMusiclistRemove(PlaylistPtr playlist, const MusicMetaList &metalist);
    void onMusiclistDelete(PlaylistPtr playlist, const MusicMetaList &metalist);
    void onPlaylistAdd(bool edit);
    void onSelectedPlaylistChanged(PlaylistPtr playlist);

    //! ui: menu interface
    void onRequestMusiclistMenu(const QPoint &pos);
    void onSearchText(const QString text);
    void onExitSearch();
    void onLocateMusicAtAll(const QString &hash);

    void onImportMusicDirectory();
    void onImportFiles(const QStringList &filelist);

private:
    QScopedPointer<PresenterPrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), Presenter)
};

