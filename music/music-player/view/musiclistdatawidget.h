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

class ActionBar;
class MusicListDataWidgetPrivate;
class MusicListDataWidget : public DWidget
{
    Q_OBJECT
public:
    explicit MusicListDataWidget(QWidget *parent = Q_NULLPTR);
    ~MusicListDataWidget() override;

    //void setCustomSortType(PlaylistPtr playlist);
    void tabwidgetInfo(PlaylistPtr infoPlaylist);
    PlaylistPtr curPlaylist();

public slots:
    void onSearchText(QString str);
    void selectMusiclistChanged(PlaylistPtr playlist);
    void onMusiclistChanged(PlaylistPtr playlist);
    void onMusicListAdded(PlaylistPtr playlist, const MetaPtrList metalist);
    void onMusicListRemoved(PlaylistPtr playlist, const MetaPtrList metalist);
    void onMusiclistUpdate();
    void onMusicPlayed(PlaylistPtr playlist, const MetaPtr);
    void slotTheme(int type);
    void changePicture(QPixmap pixmap, QPixmap sidebarPixmap, QPixmap albumPixmap);
    void retResult(QString searchText, QList<PlaylistPtr> resultlist);
    void resultTabwidget(int index);
    void setCustomSortType(PlaylistPtr playlist);
    void CloseSearch();
    void onCustomContextMenuRequest(const QPoint &pos,
                                    PlaylistPtr selectedlist,
                                    PlaylistPtr favlist,
                                    QList<PlaylistPtr >newlists,
                                    char type);

signals:
    void playall(PlaylistPtr playlist);
    void resort(PlaylistPtr playlist, int sortType);
    void playMedia(PlaylistPtr playlist, const MetaPtr meta);
    void resume(PlaylistPtr playlist, const MetaPtr meta);
    void pause(PlaylistPtr playlist, const MetaPtr meta);
    void modeChanged(int);
    void importSelectFiles(PlaylistPtr playlist, QStringList urllist);

    void requestCustomContextMenu(const QPoint &pos, char type);
    void addToPlaylist(PlaylistPtr playlist, const MetaPtrList  &metalist);
    void musiclistRemove(PlaylistPtr playlist, const MetaPtrList  &metalist);
    void musiclistDelete(PlaylistPtr playlist, const MetaPtrList  &metalist);
    void showInfoDialog(const MetaPtr meta);
    void updateMetaCodec(const QString &preTitle, const QString &preArtist, const QString &preAlbum, const MetaPtr meta);
    void addMetasFavourite(const MetaPtrList  &metalist);
    void removeMetasFavourite(const MetaPtrList  &metalist);

    void changeFocus(QString type);

protected:
    bool eventFilter(QObject *o, QEvent *e) Q_DECL_OVERRIDE;

protected:
    virtual void dragEnterEvent(QDragEnterEvent *event) Q_DECL_OVERRIDE;
    virtual void dropEvent(QDropEvent *event) Q_DECL_OVERRIDE;

private:
    PlaylistPtr  MusicPlaylists;
    PlaylistPtr ArtistPlaylists;
    PlaylistPtr  AlbumPlaylists;

    QScopedPointer<MusicListDataWidgetPrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), MusicListDataWidget)
};

class ActionBar : public DWidget
{
    Q_OBJECT
public:
    explicit ActionBar(QWidget *parent = Q_NULLPTR);
    ~ActionBar() {}
private:
    bool MoveFlag = false;

protected:
    //Filter the mousemoveevent
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
};

