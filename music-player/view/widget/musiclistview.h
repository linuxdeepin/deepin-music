/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#pragma once

#include <QListView>

#include "../../core/playlist.h"

class QStandardItemModel;
class MusicListViewPrivate;
class MusicListView : public QListView
{
    Q_OBJECT
public:
    explicit MusicListView(QWidget *parent = 0);
    ~MusicListView();

signals:
    void play(const MusicMeta &meta);
    void updateMetaCodec(const MusicMeta &meta);
    void addToPlaylist(PlaylistPtr playlist, const MusicMetaList metalist);
    void removeMusicList(const MusicMetaList &metalist);
    void deleteMusicList(const MusicMetaList &metalist);
    void requestCustomContextMenu(const QPoint &pos);

public:
    PlaylistPtr playlist();
    const MusicMeta activeMeta();

    void onMusicPlayed(PlaylistPtr playlist, const MusicMeta &meta);
    void onMusicPause(PlaylistPtr playlist, const MusicMeta &meta);
    void onMusicRemoved(PlaylistPtr playlist, const MusicMeta &meta);
    void onMusicAdded(PlaylistPtr playlist, const MusicMeta &meta);
    void onMusicError(PlaylistPtr playlist, const MusicMeta &meta, int error);
    void onMusicListAdded(PlaylistPtr playlist, const MusicMetaList &metalist);
    void onLocate(PlaylistPtr playlist, const MusicMeta &meta);
    void onMusiclistChanged(PlaylistPtr playlist);

    void showContextMenu(const QPoint &pos,
                         PlaylistPtr selectedlist,
                         PlaylistPtr favlist,
                         QList<PlaylistPtr >newlist);


protected:
    virtual void paintEvent(QPaintEvent *e) Q_DECL_OVERRIDE;
    virtual void wheelEvent(QWheelEvent *event) Q_DECL_OVERRIDE;
    virtual void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;

private:
    QScopedPointer<MusicListViewPrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), MusicListView)

};
