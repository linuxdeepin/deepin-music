/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#pragma once

#include <QFrame>

#include "../core/playlist.h"

class PlayListView;
class PlaylistWidget : public QFrame
{
    Q_OBJECT
public:
    explicit PlaylistWidget(QWidget *parent = 0);
    void initData(QList<PlaylistPtr > playlists, PlaylistPtr last);

signals:
    void addPlaylist(bool editmode);
    void selectPlaylist(PlaylistPtr playlist);
    void hidePlaylist();
    void playall(PlaylistPtr playlist);
    void customResort(const QStringList &uuids);

public slots:
    void onMusicPlayed(PlaylistPtr playlist, const MetaPtr);
    void onPlaylistAdded(PlaylistPtr);
    void onCurrentChanged(PlaylistPtr playlist);

protected:
    virtual void focusOutEvent(QFocusEvent *event) Q_DECL_OVERRIDE;

private:
    PlayListView *m_listview;
};

