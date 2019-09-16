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

#include <DListWidget>

#include "../../core/playlist.h"

DWIDGET_USE_NAMESPACE

class MusicListView : public DListWidget
{
    Q_OBJECT
public:
    explicit MusicListView(QWidget *parent = Q_NULLPTR);
    ~MusicListView();

    void updateScrollbar();
    void showContextMenu(const QPoint &pos);

signals:
    void playall(PlaylistPtr playlist);
    void customResort(const QStringList &uuids);
    void displayNameChanged();

protected:
    virtual void startDrag(Qt::DropActions supportedActions) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

};
