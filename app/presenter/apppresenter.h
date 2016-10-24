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

class Playlist;
class AppPresenterPrivate;
class QMediaPlayer;
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

signals:
    // change ui signal
    void requestImportFiles();
    void showPlaylist();
    void showMusiclist();

    //! request import dialog
    void importMediaFiles(QSharedPointer<Playlist> playlist, const QStringList &filelist);

    // to playlist
    void playlistAdded(QSharedPointer<Playlist>);
    void playlistRemove(QSharedPointer<Playlist>);
    void selectedPlaylistChanged(QSharedPointer<Playlist>);
    void playingPlaylistChanged(QSharedPointer<Playlist>);

    // to music list
    void musicAdded(QSharedPointer<Playlist> palylist, const MusicInfo &info);
    void musicPlayed(QSharedPointer<Playlist> palylist, const MusicInfo &info);
    void musicPaused(QSharedPointer<Playlist> palylist, const MusicInfo &info);
    void musicRemoved(QSharedPointer<Playlist> palylist, const MusicInfo &info);

public slots:
    //! UI: music control interface
    void onMusicPlay(QSharedPointer<Playlist> playlist, const MusicInfo &info);
    void onMusicPause(QSharedPointer<Playlist> playlist, const MusicInfo &info);
//    void onMusicStop(QSharedPointer<Playlist> playlist, const MusicInfo &info);
    void onMusicPrev(QSharedPointer<Playlist> playlist, const MusicInfo &info);
    void onMusicNext(QSharedPointer<Playlist> playlist, const MusicInfo &info);
    void onToggleFavourite(const MusicInfo &info);

    //! UI: playlist manager interface
    //! TODO: refactor
    void onMusicAdd(const QString &id, const MusicInfo &info);
//    void onMusicAdd(QSharedPointer<Playlist> playlist, const MusicInfo &info);
    void onMusicRemove(QSharedPointer<Playlist> playlist, const MusicInfo &info);

    void onPlaylistAdd(bool edit);
    void onSelectedPlaylistChanged(QSharedPointer<Playlist> playlist);

    //! UI: menu interface
//    void onRequestPlaylistMenuData(QSharedPointer<Playlist> playlist);
//    void onRequestMusiclistMenuData(QSharedPointer<Playlist> playlist, const MusicInfo &selected);


public slots:
    void onImportMusicDirectory();
    void onImportFiles(const QStringList &filelist);

private:
    void prepareData();

    QScopedPointer<AppPresenterPrivate> d;
};

#endif // APPPRESENTER_H
