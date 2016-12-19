/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef APPPRESENTER_H
#define APPPRESENTER_H

#include <QObject>
#include <QScopedPointer>

#include "../core/music.h"
#include "../core/playlist.h"

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

    void prepareData();

    //! action signal
signals:
    //! ui: request import dialog
    void importMediaFiles(PlaylistPtr playlist, const QStringList &filelist);

    //! notify signal
signals:
    void dataPrepared();

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
    void selectedPlaylistChanged(PlaylistPtr);
    void playingPlaylistChanged(PlaylistPtr);

    //! from playlist manager
    void musicAdded(PlaylistPtr playlist, const MusicMeta &info);
    void musiclistAdded(PlaylistPtr playlist, const MusicMetaList &metalist);
    void musicRemoved(PlaylistPtr playlist, const MusicMeta &info);

    //! from control
    void musicPlayed(PlaylistPtr playlist, const MusicMeta &info);
    void musicPaused(PlaylistPtr playlist, const MusicMeta &info);
    void musicStoped(PlaylistPtr playlist, const MusicMeta &info);
    void progrossChanged(qint64 pos, qint64 length);

    //! from lyricservice
    void lyricSearchFinished(const MusicMeta &, const QByteArray &lyricData);
    void coverSearchFinished(const MusicMeta &, const QByteArray &coverData);

    //! meta info
    void metaInfoClean();

public slots:
    //! music control interface
    void onMusicPlay(PlaylistPtr playlist, const MusicMeta &meta);
    void onMusicPause(PlaylistPtr playlist, const MusicMeta &info);
    void onMusicResume(PlaylistPtr playlist, const MusicMeta &info);
    //void onMusicStop(PlaylistPtr playlist, const MusicInfo &info);
    void onMusicPrev(PlaylistPtr playlist, const MusicMeta &info);
    void onMusicNext(PlaylistPtr playlist, const MusicMeta &info);
    void onToggleFavourite(const MusicMeta &info);
    void onChangeProgress(qint64 value, qint64 range);
    void onPlayModeChanged(int mode);

    //! music list
    void onPlayall(PlaylistPtr playlist);
    void onResort(PlaylistPtr playlist, int sortType);

    //! UI: playlist manager interface
    void onMusicAdd(PlaylistPtr playlist, const MusicMetaList &metalist);
    void onMusicRemove(PlaylistPtr playlist, const MusicMetaList &metalist);
    void onMusicDelete(PlaylistPtr, const MusicMetaList &metalist);
    void onPlaylistAdd(bool edit);
    void onSelectedPlaylistChanged(PlaylistPtr playlist);

    //! ui: menu interface
    void onRequestMusiclistMenu(const QPoint &pos);
    void onSearchText(const QString text);
    void onLocateMusic(const QString &hash);

    void onImportMusicDirectory();

public slots:
    void onImportFiles(const QStringList &filelist);

    //! private
signals:
    //! to player backend
    void changeProgress(qint64 value, qint64 range);
    void play(PlaylistPtr playlist, const MusicMeta &info);
    void resume(PlaylistPtr playlist, const MusicMeta &meta);
    void playNext(PlaylistPtr playlist, const MusicMeta &info);
    void playPrev(PlaylistPtr playlist, const MusicMeta &info);
    void stop();
    void pause();

    // to lyric backend
    void requestLyricCoverSearch(const MusicMeta &);

private:
    QScopedPointer<PresenterPrivate> d;
};

#endif // APPPRESENTER_H
