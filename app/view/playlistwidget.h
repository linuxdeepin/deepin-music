/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef PLAYLISTWIDGET_H
#define PLAYLISTWIDGET_H

#include <QFrame>

class MusicListInfo;
class Playlist;
class PlayListView;
class PlaylistWidget : public QFrame
{
    Q_OBJECT
public:
    explicit PlaylistWidget(QWidget *parent = 0);

    void updatePlaylist(QList<QSharedPointer<Playlist> > playlists);

public slots:
    void onPlaylistAdded(QSharedPointer<Playlist>);
    void onCurrentPlaylistChanded(QSharedPointer<Playlist> playlist);

signals:
    void addPlaylist(bool editmode);
    void selectPlaylist(QSharedPointer<Playlist>);

private:
    PlayListView *m_listview;
};

#endif // PLAYLISTWIDGET_H
