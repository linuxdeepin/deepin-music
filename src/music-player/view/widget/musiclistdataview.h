/*
 * Copyright (C) 2016 ~ 2018 Wuhan Deepin Technology Co., Ltd.
 *
 * Author:     Iceyer <me@iceyer.net>
 *
 * Maintainer: Iceyer <me@iceyer.net>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <DListView>

#include "../../core/playlist.h"

DWIDGET_USE_NAMESPACE

class MusicListDataViewPrivate;
class MusicListDataView : public DListView
{
    Q_OBJECT
public:
    explicit MusicListDataView(QWidget *parent = Q_NULLPTR);
    ~MusicListDataView();

    PlaylistPtr playlist() const;

    int rowCount();

    int listSize();

    void setViewModeFlag(QListView::ViewMode mode);
    void setPlaying(const MetaPtr);
    MetaPtr playing() const;
    MetaPtr hoverin() const;

    void showContextMenu(const QPoint &pos,
                         PlaylistPtr selectedPlaylist,
                         PlaylistPtr favPlaylist,
                         QList<PlaylistPtr > newPlaylists);

    PlayMusicTypePtrList playMusicTypePtrList() const;

    void setThemeType(int type);
    int getThemeType() const;

    void setPlayPixmap(QPixmap pixmap, QPixmap sidebarPixmap);
    QPixmap getPlayPixmap() const;
    QPixmap getSidebarPixmap() const;

    void updateList();

signals:
    void requestCustomContextMenu(const QPoint &pos);
    void playMedia(const MetaPtr meta);
    void resume(const MetaPtr meta);
    void pause(PlaylistPtr playlist, const MetaPtr meta);
    void addToPlaylist(PlaylistPtr playlist, const MetaPtrList  &metalist);
    void musiclistRemove(const MetaPtrList  &metalist);
    void musiclistDelete(const MetaPtrList  &metalist);
    void modeChanged(int);
    void addMetasFavourite(const MetaPtrList  &metalist);
    void removeMetasFavourite(const MetaPtrList  &metalist);

protected:
    virtual void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

public:
    void onMusiclistChanged(PlaylistPtr playlist);

private:
    QScopedPointer<MusicListDataViewPrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), MusicListDataView)
};
