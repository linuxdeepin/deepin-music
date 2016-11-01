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

#include "model/musiclistmodel.h"

class MusicItem;
class Playlist;
class AppPresenterPrivate;

class AppPresenter : public QObject
{
    Q_OBJECT
public:
    explicit AppPresenter(QObject *parent = 0);
    ~AppPresenter();

    QSharedPointer<Playlist> allMusicPlaylist();
    QSharedPointer<Playlist> favMusicPlaylist();
    QSharedPointer<Playlist> lastPlaylist();
    QList<QSharedPointer<Playlist> > allplaylist();
    int playMode();

    void prepareData();

    //! action signal
signals:
    //! ui: request import dialog
    void importMediaFiles(QSharedPointer<Playlist> playlist, const QStringList &filelist);

    //! notify signal
signals:
    void dataPrepared();

    //! ui: control
    void requestImportFiles();
    void showPlaylist();
    void showMusiclist();

    //! ui: menu
    void musiclistMenuRequested(const QPoint &pos,
                                QSharedPointer<Playlist> selectedlist,
                                QSharedPointer<Playlist> favlist,
                                QList<QSharedPointer<Playlist> >newlists);

    //! from playlist manager
    void playlistAdded(QSharedPointer<Playlist>);
    void playlistRemove(QSharedPointer<Playlist>);
    void selectedPlaylistChanged(QSharedPointer<Playlist>);
    void playingPlaylistChanged(QSharedPointer<Playlist>);

    //! from playlist manager
    void musicAdded(QSharedPointer<Playlist> palylist, const MusicMeta &info);
    void musiclistAdded(QSharedPointer<Playlist> palylist, const MusicMetaList &metalist);
    void musicRemoved(QSharedPointer<Playlist> palylist, const MusicMeta &info);

    //! from control
    void musicPlayed(QSharedPointer<Playlist> palylist, const MusicMeta &info);
    void musicPaused(QSharedPointer<Playlist> palylist, const MusicMeta &info);
    void progrossChanged(qint64 value, qint64 range);

    //! from lyricservice
    void lyricSearchFinished(const MusicMeta &, const QString &lyricPath);
    void coverSearchFinished(const MusicMeta &, const QString &coverPath);

public slots:
    //! music control interface
    void onMusicPlay(QSharedPointer<Playlist> playlist, const MusicMeta &info);
    void onMusicPause(QSharedPointer<Playlist> playlist, const MusicMeta &info);
    //void onMusicStop(QSharedPointer<Playlist> playlist, const MusicInfo &info);
    void onMusicPrev(QSharedPointer<Playlist> playlist, const MusicMeta &info);
    void onMusicNext(QSharedPointer<Playlist> playlist, const MusicMeta &info);
    void onToggleFavourite(const MusicMeta &info);
    void onChangeProgress(qint64 value, qint64 range);
    void onPlayModeChanged(int mode);

    //! music list
    void onPlayall(QSharedPointer<Playlist> playlist);
    void onResort(QSharedPointer<Playlist> palylist, int sortType);

    //! UI: playlist manager interface
    void onMusicAdd(QSharedPointer<Playlist> playlist, const MusicMetaList &metalist);
    void onMusicRemove(QSharedPointer<Playlist> playlist, const MusicMeta &info);
    void onPlaylistAdd(bool edit);
    void onSelectedPlaylistChanged(QSharedPointer<Playlist> playlist);

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
    void play(QSharedPointer<Playlist> playlist, const MusicMeta &info);
    void playNext(QSharedPointer<Playlist> playlist, const MusicMeta &info);
    void playPrev(QSharedPointer<Playlist> playlist, const MusicMeta &info);
    void stop();
    void pause();

    // to lyric backend
    void requestLyricCoverSearch(const MusicMeta &);

private:
    QScopedPointer<AppPresenterPrivate> d;
};

#endif // APPPRESENTER_H
