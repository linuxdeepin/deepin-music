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
    void showContextMenu(const QPoint &pos);
    void adjustHeight();
    bool getHeightChangeToMax();
public slots:
    void setThemeType(int type);

    // 新建歌单
    void addNewSongList();
    // 删除歌单
    void rmvSongList();
    void slotUpdatePlayingIcon();
    void slotMenuTriggered(QAction *action);
signals:
    void sigAddNewSongList();
    void sigRmvSongList();
    void currentChanged(const QModelIndex &current, const QModelIndex &previous) Q_DECL_OVERRIDE;

protected:
//    virtual void startDrag(Qt::DropActions supportedActions) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void closeEditor(QWidget *editor, QAbstractItemDelegate::EndEditHint hint) Q_DECL_OVERRIDE;
    // 实现delete快捷操作
    void keyReleaseEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    void dragEnterEvent(QDragEnterEvent *event) Q_DECL_OVERRIDE;
    void dragMoveEvent(QDragMoveEvent *event) Q_DECL_OVERRIDE;
    void dropEvent(QDropEvent *event) Q_DECL_OVERRIDE;
private:
    // 初始化快捷键
    void initShortcut();
    void SetAttrRecur(QDomElement elem, QString strtagname, QString strattr, QString strattrval);
    QString newDisplayName();
private:
    //    QList<PlaylistPtr >  allPlaylists;
    MusicBaseAndSonglistModel *model = nullptr;
    DStyledItemDelegate  *delegate        = nullptr;
    //QStandardItem      *m_currentitem = nullptr;
    QPixmap              playingPixmap;
    QPixmap              albumPixmap;
    QPixmap              defaultPixmap;
    // 新建歌单快捷键
    QShortcut           *m_newItemShortcut = nullptr;
    bool                pixmapState         = false;
    bool                m_heightChangeToMax = false;
};

