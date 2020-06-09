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

class MusicListView : public DListView
{
    Q_OBJECT
public:
    explicit MusicListView(QWidget *parent = Q_NULLPTR);
    ~MusicListView();

    void showContextMenu(const QPoint &pos);

    void addMusicList(PlaylistPtr playlist, bool addFlag = false);

    QStandardItem *item(int row, int column = 0) const;
    void setCurrentItem ( QStandardItem *item );

    PlaylistPtr playlistPtr(const QModelIndex &index);
    PlaylistPtr playlistPtr(QStandardItem *item);

    void setCurPlaylist(QStandardItem *item);

    void closeAllPersistentEditor();
    void clearSelected();

    void changePicture(QPixmap pixmap, QPixmap albumPixmap);

    void adjustHeight();

public slots:
    void slotTheme(int type);
    void onRename(QStandardItem *item);
    void closeEditor(QWidget *editor, QAbstractItemDelegate::EndEditHint hint) Q_DECL_OVERRIDE;

signals:
    void playall(PlaylistPtr playlist);
    void customResort(const QStringList &uuids);
    void displayNameChanged();
    void pause(PlaylistPtr playlist, const MetaPtr meta);
    void removeAllList(const MetaPtr meta);
    void importSelectFiles(PlaylistPtr playlist, QStringList urllist);
    void addToPlaylist(PlaylistPtr playlist, const MetaPtrList &metalist);

    void currentChanged(const QModelIndex &current, const QModelIndex &previous) Q_DECL_OVERRIDE;

protected:
//    virtual void startDrag(Qt::DropActions supportedActions) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    void dragEnterEvent(QDragEnterEvent *event) Q_DECL_OVERRIDE;
    void dragMoveEvent(QDragMoveEvent *event) Q_DECL_OVERRIDE;
    void dropEvent(QDropEvent *event) Q_DECL_OVERRIDE;

private:
    QList<PlaylistPtr >  allPlaylists;
    QStandardItemModel   *model           = nullptr;
    DStyledItemDelegate  *delegate        = nullptr;
    QStandardItem        *playingItem     = nullptr;
    //QStandardItem      *m_currentitem = nullptr;
    QPixmap              playingPixmap;
    QPixmap              albumPixmap;
    QPixmap              defaultPixmap;
    int                  m_type = 1;
};

