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

#include <DDialog>
#include <mediameta.h>

#include "../core/playlist.h"

DWIDGET_USE_NAMESPACE

class MusicListDialogPrivate;
class MusicListDialog : public DAbstractDialog
{
    Q_OBJECT

public:
    explicit MusicListDialog(QWidget *parent = Q_NULLPTR);
    ~MusicListDialog();

    void setPlayMusicData(PlaylistPtr playlist, PlayMusicTypePtr playMusicType);
    void setPlaying(const MetaPtr);

    void showContextMenu(const QPoint &pos,
                         PlaylistPtr selectedPlaylist,
                         PlaylistPtr favPlaylist,
                         QList<PlaylistPtr > newPlaylists);

    void setThemeType(int type);
    void setPlayPixmap(QPixmap pixmap, QPixmap sidebarPixmap);

signals:
    void requestCustomContextMenu(const QPoint &pos);
    void playMedia(const MetaPtr meta);
    void resume(const MetaPtr meta);
    void addToPlaylist(PlaylistPtr playlist, const MetaPtrList  &metalist);
    void musiclistRemove(const MetaPtrList  &metalist);
    void musiclistDelete(const MetaPtrList  &metalist);
    void modeChanged(int);
    void pause(const MetaPtr meta);
    void addMetasFavourite(const MetaPtrList  &metalist);
    void removeMetasFavourite(const MetaPtrList  &metalist);

private:
    QScopedPointer<MusicListDialogPrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), MusicListDialog)
};

