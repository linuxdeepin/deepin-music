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
#include <QDomElement>

DWIDGET_USE_NAMESPACE

class MusicBaseAndSonglistModel;
class QShortcut;
//自定义歌单列表
class MusicSongListView : public DListView
{
    Q_OBJECT
public:
    explicit MusicSongListView(QWidget *parent = Q_NULLPTR);
    ~MusicSongListView() override;

    void init();

public slots:
    void slotTheme(int type);

    //add a new play list
    void addNewSongList();
    void slotUpdatePlayingIcon();
signals:

protected:
//    virtual void startDrag(Qt::DropActions supportedActions) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

//    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    void dragEnterEvent(QDragEnterEvent *event) Q_DECL_OVERRIDE;
    void dragMoveEvent(QDragMoveEvent *event) Q_DECL_OVERRIDE;
//    void dropEvent(QDropEvent *event) Q_DECL_OVERRIDE;

private:
    void SetAttrRecur(QDomElement elem, QString strtagname, QString strattr, QString strattrval);
    QString newDisplayName();
private:
    //    QList<PlaylistPtr >  allPlaylists;
    MusicBaseAndSonglistModel *model = nullptr;
    DStyledItemDelegate  *delegate        = nullptr;
    QStandardItem        *playingItem     = nullptr;
    //QStandardItem      *m_currentitem = nullptr;
    QPixmap              playingPixmap;
    QPixmap              albumPixmap;
    QPixmap              defaultPixmap;
    QShortcut           *m_newItemShortcut = nullptr;
    int                 m_type             = 1;
    bool                pixmapState         = false;
};

