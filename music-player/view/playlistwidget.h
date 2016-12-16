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

#include "../core/music.h"
#include "../core/playlist.h"

class PlayListView;
class PlaylistWidget : public QFrame
{
    Q_OBJECT
public:
    explicit PlaylistWidget(QWidget *parent = 0);

    void initPlaylist(QList<PlaylistPtr > playlists, PlaylistPtr last);

    virtual void focusOutEvent(QFocusEvent *event) Q_DECL_OVERRIDE;
public slots:
    void onPlaylistAdded(PlaylistPtr);
    void onCurrentChanged(PlaylistPtr playlist);

signals:
    void playall(PlaylistPtr playlist);
    void addPlaylist(bool editmode);
    void deletePlaylist(PlaylistPtr playlist);
    void selectPlaylist(PlaylistPtr);

private:
    PlayListView *m_listview;
};

#endif // PLAYLISTWIDGET_H
