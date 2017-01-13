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

#include <MprisPlayer>

#include "../core/playlist.h"

class Playlist;

class PresenterPrivate;
class Presenter : public QObject
{
    Q_OBJECT
public:
    explicit Presenter(QObject *parent = 0);
    ~Presenter();

    void initMpris(MprisPlayer* mprisPlayer);
    void prepareData();
    void postAction();

    QList<PlaylistPtr > allplaylist();

public slots:
    void volumeUp();
    void volumeDown();
    void togglePaly();
    void next();
    void prev();

    void requestImportPaths(PlaylistPtr playlist, const QStringList &filelist);

signals:
    void dataLoaded();

signals:
    //! ui: request import dialog
    void meidaFilesImported(PlaylistPtr playlist, MetaPtrList metalist);

    //! ui: control
    // TODO: need path
    void requestImportFiles();
    void showPlaylist(bool visible);
    void showMusicList(PlaylistPtr playlist);

    //! from playlist manager
    void playlistAdded(PlaylistPtr);
    void playlistRemove(PlaylistPtr);
    void activePlaylistChanged(PlaylistPtr);

    //! from music manager
    void locateMusic(PlaylistPtr, const MetaPtr);
    void currentMusicListChanged(PlaylistPtr);
    void musicListResorted(PlaylistPtr);
    void musicListAdded(PlaylistPtr playlist, const MetaPtrList metalist);
    void musicListRemoved(PlaylistPtr playlist, const MetaPtrList metalist);
    void requestMusicListMenu(const QPoint &pos,
                                PlaylistPtr selectedlist,
                                PlaylistPtr favlist,
                                QList<PlaylistPtr >newlists);

    //! from control
    void musicPlayed(PlaylistPtr playlist, const MetaPtr meta);
    void musicError(PlaylistPtr playlist, const MetaPtr meta, int error);
    void musicPaused(PlaylistPtr playlist, const MetaPtr meta);
    void musicStoped(PlaylistPtr playlist, const MetaPtr meta);
    void musicMetaUpdate(PlaylistPtr playlist, const MetaPtr meta);
    void progrossChanged(qint64 pos, qint64 length);
    void volumeChanged(int volume);
    void mutedChanged(bool muted);

    //! from lyricservice
    void requestContextSearch(const QString &context);
    void lyricSearchFinished(const MetaPtr , const QByteArray &lyricData);
    void coverSearchFinished(const MetaPtr , const QByteArray &coverData);
    void contextSearchFinished(const QString &context, const MetaPtrList metalist);

    //! meta info
    void metaLibraryClean();
    void notifyMusciError(PlaylistPtr playlist, const MetaPtr meta, int error);
    void notifyAddToPlaylist(PlaylistPtr playlist, const MetaPtrList info);

public slots:
    //! music control interface
    void onSyncMusicPlay(PlaylistPtr playlist, const MetaPtr meta);
    void onMusicPlay(PlaylistPtr playlist, const MetaPtr meta);
    void onMusicPause(PlaylistPtr playlist, const MetaPtr info);
    void onMusicResume(PlaylistPtr playlist, const MetaPtr info);
    void onMusicStop(PlaylistPtr playlist, const MetaPtr info);
    void onMusicPrev(PlaylistPtr playlist, const MetaPtr meta);
    void onMusicNext(PlaylistPtr playlist, const MetaPtr meta);

    void onToggleFavourite(const MetaPtr meta);
    void onChangeProgress(qint64 value, qint64 range);
    void onVolumeChanged(int volume);
    void onPlayModeChanged(int mode);
    void onToggleMute();

    void onUpdateMetaCodec(const MetaPtr meta);

    //! music list
    void onPlayall(PlaylistPtr playlist);
    void onResort(PlaylistPtr playlist, int sortType);

    //! UI: playlist manager interface
    void onMusiclistRemove(PlaylistPtr playlist, const MetaPtrList metalist);
    void onMusiclistDelete(PlaylistPtr playlist, const MetaPtrList metalist);
    void onAddToPlaylist(PlaylistPtr playlist, const MetaPtrList metalist);
    void onPlaylistAdd(bool edit);
    void onCurrentPlaylistChanged(PlaylistPtr playlist);

    //! ui: menu interface
    void onRequestMusiclistMenu(const QPoint &pos);
    void onSearchText(const QString text);
    void onExitSearch();
    void onLocateMusicAtAll(const QString &hash);
    void onChangeSearchMetaCache(const MetaPtr meta);

    void onScanMusicDirectory();
    void onImportFiles(const QStringList &filelist);

private:
    QScopedPointer<PresenterPrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), Presenter)
};

