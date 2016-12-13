/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef MUSICLISTWIDGET_H
#define MUSICLISTWIDGET_H

#include <QFrame>

#include <dcombobox.h>

#include "../core/music.h"
#include "../core/playlist.h"

class MusicItem;
class MusicMeta;
class QLabel;
class MusicListView;
class QStandardItemModel;

class MusicListWidget : public QFrame
{
    Q_OBJECT
public:
    explicit MusicListWidget(QWidget *parent = 0);

    void setCurrentList(PlaylistPtr playlist);
    virtual void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;

signals:
    void musicListRemove(PlaylistPtr playlist, const MusicMetaList &metalist);
    void musicListDelete(PlaylistPtr playlist, const MusicMetaList &metalist);

    void musicClicked(PlaylistPtr playlist, const MusicMeta &info);
    void musicAdd(PlaylistPtr playlist, const MusicMetaList &metalist);
    void playall(PlaylistPtr playlist);
    void resort(PlaylistPtr playlist, int sortType);
    void requestCustomContextMenu(const QPoint &pos);

public slots:
    void onMusicPlayed(PlaylistPtr playlist, const MusicMeta &info);
    void onMusicRemoved(PlaylistPtr playlist, const MusicMeta &info);
    void onMusicAdded(PlaylistPtr playlist, const MusicMeta &info);
    void onMusicListAdded(PlaylistPtr playlist, const MusicMetaList &infolist);

    void onLocate(PlaylistPtr playlist, const MusicMeta &info);

    void onMusiclistChanged(PlaylistPtr playlist);
    void onCustomContextMenuRequest(const QPoint &pos,
                                    PlaylistPtr selectedlist,
                                    PlaylistPtr favlist,
                                    QList<PlaylistPtr >newlists);
signals:
    void showEmptyHits(bool empty);
private:
    void addMusicInfo(MusicListView *m_musiclist, const MusicMeta &info);

    PlaylistPtr    m_playlist;
    QLabel                      *m_emptyHits    = nullptr;
    MusicListView               *m_musiclist    = nullptr;
    QStandardItemModel          *m_model        = nullptr;
    Dtk::Widget::DComboBox      *m_sortCombo    = nullptr;
};


#endif // MUSICLISTWIDGET_H
