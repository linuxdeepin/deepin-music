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

#include <DWidget>

#include "../core/playlist.h"

DWIDGET_USE_NAMESPACE

class PlayListWidgetPrivate;
class PlayListWidget : public DWidget
{
    Q_OBJECT
public:
    explicit PlayListWidget(QWidget *parent = Q_NULLPTR);
    ~PlayListWidget() override;

    void updateInfo(PlaylistPtr playlist);
    void setCurPlaylist(PlaylistPtr playlist);
    PlaylistPtr curPlaylist();

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

    void slotTheme(int type);
    void changePicture(QPixmap pixmap, QPixmap sidebarPixmap, QPixmap albumPixmap);

signals:
    void playall(PlaylistPtr playlist);
    void resort(PlaylistPtr playlist, int sortType);
    void playMedia(PlaylistPtr playlist, const MetaPtr meta);
    void resume(PlaylistPtr playlist, const MetaPtr meta);
    void pause(PlaylistPtr playlist, const MetaPtr meta);
    void showInfoDialog(const MetaPtr meta);
    void updateMetaCodec(const QString &preTitle, const QString &preArtist, const QString &preAlbum, const MetaPtr meta);

    void requestCustomContextMenu(const QPoint &pos, char type);
    void addToPlaylist(PlaylistPtr playlist, const MetaPtrList  &metalist);
    void musiclistRemove(PlaylistPtr playlist, const MetaPtrList  &metalist);
    void musiclistDelete(PlaylistPtr playlist, const MetaPtrList  &metalist);
    void importSelectFiles(PlaylistPtr playlist, QStringList urllist);

    void addMetasFavourite(const MetaPtrList  &metalist);
    void removeMetasFavourite(const MetaPtrList  &metalist);
    void musicFileMiss();

protected:
    virtual void dragEnterEvent(QDragEnterEvent *event) Q_DECL_OVERRIDE;
//    virtual void dragMoveEvent(QDragMoveEvent *event) Q_DECL_OVERRIDE;
//    virtual void dragLeaveEvent(QDragLeaveEvent *event) Q_DECL_OVERRIDE;
    virtual void dropEvent(QDropEvent *event) Q_DECL_OVERRIDE;
    virtual void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;

private:
    QScopedPointer<PlayListWidgetPrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), PlayListWidget)
};

extern const QString AlbumMusicListID;
extern const QString ArtistMusicListID;
extern const QString AllMusicListID;
extern const QString FavMusicListID;
extern const QString SearchMusicListID;
extern const QString PlayMusicListID;
extern const QString AlbumCandListID;
extern const QString MusicCandListID;
extern const QString ArtistCandListID;
