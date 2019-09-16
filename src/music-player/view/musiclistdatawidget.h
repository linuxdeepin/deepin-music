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

class MusicListDataWidgetPrivate;
class MusicListDataWidget : public DWidget
{
    Q_OBJECT
public:
    explicit MusicListDataWidget(QWidget *parent = Q_NULLPTR);
    ~MusicListDataWidget();

    void setCustomSortType(PlaylistPtr playlist);

public slots:
    void onMusiclistChanged(PlaylistPtr playlist);
    void onMusicListAdded(PlaylistPtr playlist, const MetaPtrList metalist);
    void onMusiclistUpdate();
    void onMusicPlayed(PlaylistPtr playlist, const MetaPtr);
    void slotTheme(int type);
    void changePicture(QPixmap pixmap);

    void onCustomContextMenuRequest(const QPoint &pos,
                                    PlaylistPtr selectedlist,
                                    PlaylistPtr favlist,
                                    QList<PlaylistPtr >newlists,
                                    char type);

signals:
    void playall(PlaylistPtr playlist);
    void resort(PlaylistPtr playlist, int sortType);
    void playMedia(PlaylistPtr playlist, const MetaPtr meta);
    void pause(PlaylistPtr playlist, const MetaPtr meta);
    void modeChanged(int);

    void requestCustomContextMenu(const QPoint &pos, char type);
    void addToPlaylist(PlaylistPtr playlist, const MetaPtrList  &metalist);
    void musiclistRemove(PlaylistPtr playlist, const MetaPtrList  &metalist);
    void musiclistDelete(PlaylistPtr playlist, const MetaPtrList  &metalist);
    void showInfoDialog(const MetaPtr meta);
    void updateMetaCodec(const MetaPtr meta);

private:
    QScopedPointer<MusicListDataWidgetPrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), MusicListDataWidget)
};

