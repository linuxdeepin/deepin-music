/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef MUSICLISTVIEW_H
#define MUSICLISTVIEW_H

#include <QTableView>

#include <musicmeta.h>
#include "../../core/playlist.h"

class Playlist;
class QStandardItemModel;
class QScrollBar;
class MusicListView : public QTableView
{
    Q_OBJECT
public:
    explicit MusicListView(QWidget *parent = 0);

    QStandardItemModel *model() const {return m_model;}

signals:
    void play(const MusicMeta &meta);
    void addToPlaylist(PlaylistPtr playlist, const MusicMetaList metalist);
    void removeMusicList(const MusicMetaList &metalist);
    void deleteMusicList(const MusicMetaList &metalist);
    void requestCustomContextMenu(const QPoint &pos);

public slots:
    void showContextMenu(const QPoint &pos,
                         PlaylistPtr selectedlist,
                         PlaylistPtr favlist,
                         QList<PlaylistPtr >newlist);


protected:
    virtual void wheelEvent(QWheelEvent *event) Q_DECL_OVERRIDE;
    virtual void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;

private:
    // TODO: remove
    QStandardItemModel  *m_model        = nullptr;
    QScrollBar          *m_scrollBar    = nullptr;
};

#endif // MUSICLISTVIEW_H
