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

#include <DFrame>

#include "../core/playlist.h"

DWIDGET_USE_NAMESPACE

class MusicListView;
class MusicListDataWidget;
class MusicListWidget : public DWidget
{
    Q_OBJECT
public:
    explicit MusicListWidget(QWidget *parent = Q_NULLPTR);
    void initData(QList<PlaylistPtr > playlists, PlaylistPtr last);

signals:
    void addPlaylist(bool editmode);
    void selectPlaylist(PlaylistPtr playlist);
    void hidePlaylist();
    void playall(PlaylistPtr playlist);
    void customResort(const QStringList &uuids);
    void playMedia(PlaylistPtr playlist, const MetaPtr meta);
    void resort(PlaylistPtr playlist, int sortType);

    void requestCustomContextMenu(const QPoint &pos, char type);
    void addToPlaylist(PlaylistPtr playlist, const MetaPtrList  &metalist);
    void musiclistRemove(PlaylistPtr playlist, const MetaPtrList  &metalist);
    void musiclistDelete(PlaylistPtr playlist, const MetaPtrList  &metalist);
    void showInfoDialog(const MetaPtr meta);
    void updateMetaCodec(const MetaPtr meta);

public slots:
    void onMusicPlayed(PlaylistPtr playlist, const MetaPtr);
    void onPlaylistAdded(PlaylistPtr);
    void onCurrentChanged(PlaylistPtr playlist);
    void onMusiclistChanged(PlaylistPtr playlist);
    void onMusicListAdded(PlaylistPtr playlist, const MetaPtrList metalist);
    void onMusicListRemoved(PlaylistPtr playlist, const MetaPtrList metalist);
    void onMusiclistUpdate();

    void onCustomContextMenuRequest(const QPoint &pos,
                                    PlaylistPtr selectedlist,
                                    PlaylistPtr favlist,
                                    QList<PlaylistPtr >newlists);

protected:
    virtual void focusOutEvent(QFocusEvent *event) Q_DECL_OVERRIDE;

private:
    MusicListView         *m_dataBaseListview;
    MusicListView         *m_customizeListview;
    MusicListDataWidget   *m_dataListView;
    bool                   addFlag = false;
};

