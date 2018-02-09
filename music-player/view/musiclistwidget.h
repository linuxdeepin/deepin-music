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

#include <QFrame>

#include "../core/playlist.h"

class MusicListWidgetPrivate;
class MusicListWidget : public QFrame
{
    Q_OBJECT
public:
    explicit MusicListWidget(QWidget *parent = 0);
    ~MusicListWidget();

    void setCustomSortType();
public slots:
    void onMusiclistChanged(PlaylistPtr playlist);
    void onMusicPlayed(PlaylistPtr playlist, const MetaPtr info);
    void onMusicPause(PlaylistPtr playlist, const MetaPtr meta);
    void onMusicError(PlaylistPtr playlist, const MetaPtr meta, int error);
    void onMusicListAdded(PlaylistPtr playlist, const MetaPtrList metalist);
    void onMusicListRemoved(PlaylistPtr playlist, const MetaPtrList metalist);
    void onLocate(PlaylistPtr playlist, const MetaPtr meta);
    void onCustomContextMenuRequest(const QPoint &pos,
                                    PlaylistPtr selectedlist,
                                    PlaylistPtr favlist,
                                    QList<PlaylistPtr >newlists);

signals:
    void playall(PlaylistPtr playlist);
    void resort(PlaylistPtr playlist, int sortType);
    void playMedia(PlaylistPtr playlist, const MetaPtr meta);
    void showInfoDialog(const MetaPtr meta);
    void updateMetaCodec(const MetaPtr meta);

    void requestCustomContextMenu(const QPoint &pos);
    void addToPlaylist(PlaylistPtr playlist, const MetaPtrList  &metalist);
    void musiclistRemove(PlaylistPtr playlist, const MetaPtrList  &metalist);
    void musiclistDelete(PlaylistPtr playlist, const MetaPtrList  &metalist);
    void importSelectFiles(PlaylistPtr playlist, QStringList urllist);

protected:
    virtual void dragEnterEvent(QDragEnterEvent *event) Q_DECL_OVERRIDE;
//    virtual void dragMoveEvent(QDragMoveEvent *event) Q_DECL_OVERRIDE;
//    virtual void dragLeaveEvent(QDragLeaveEvent *event) Q_DECL_OVERRIDE;
    virtual void dropEvent(QDropEvent *event) Q_DECL_OVERRIDE;
    virtual void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;

private:
    QScopedPointer<MusicListWidgetPrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), MusicListWidget)
};

