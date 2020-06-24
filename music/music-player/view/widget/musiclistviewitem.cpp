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

#include "musiclistviewitem.h"

#include <QDebug>
#include <QTimer>
#include <QMouseEvent>
#include <QHBoxLayout>
#include <QMessageBox>

#include <DLineEdit>
#include <DLabel>
#include <DMenu>

#include <DDialog>

#include "core/playlist.h"


MusicListViewItem::MusicListViewItem(PlaylistPtr playlist, QListWidget *parent) : QListWidgetItem(parent)
{
    m_data = playlist;
    Q_ASSERT(playlist);

    QIcon icon(":/mpimage/light/normal/famous_ballad_normal.svg");
    if (playlist->id() == AlbumMusicListID) {
        icon = QIcon(":/mpimage/light/normal/album_normal.svg");
    } else if (playlist->id() == ArtistMusicListID) {
        icon = QIcon(":/mpimage/light/normal/singer_normal.svg");
    } else if (playlist->id() == AllMusicListID) {
        icon = QIcon(":/mpimage/light/normal/all_music_normal.svg");
    } else if (playlist->id() == FavMusicListID) {
        icon = QIcon(":/mpimage/light/normal/my_collection_normal.svg");
    } else {
        icon = QIcon(":/mpimage/light/normal/famous_ballad_normal.svg");
    }
    setIcon(icon);

    setText(playlist->displayName());
}

