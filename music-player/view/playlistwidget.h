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

class PlayListView;
class PlaylistWidget : public QFrame
{
    Q_OBJECT
public:
    explicit PlaylistWidget(QWidget *parent = 0);
    void initData(QList<PlaylistPtr > playlists, PlaylistPtr last);

signals:
    void addPlaylist(bool editmode);
    void selectPlaylist(PlaylistPtr playlist);
    void hidePlaylist();
    void playall(PlaylistPtr playlist);
    void customResort(const QStringList &uuids);

public slots:
    void onMusicPlayed(PlaylistPtr playlist, const MetaPtr);
    void onPlaylistAdded(PlaylistPtr);
    void onCurrentChanged(PlaylistPtr playlist);

protected:
    virtual void focusOutEvent(QFocusEvent *event) Q_DECL_OVERRIDE;

private:
    PlayListView *m_listview;
};

