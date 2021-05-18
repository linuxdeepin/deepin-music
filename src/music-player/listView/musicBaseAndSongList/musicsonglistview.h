/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     ZouYa <zouya@uniontech.com>
 *
 * Maintainer: WangYu <wangyu@uniontech.com>
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
#include <DLineEdit>

DWIDGET_USE_NAMESPACE

class MusicBaseAndSonglistModel;
class QShortcut;
// 自定义歌单列表
class MusicSongListView : public DListView
{
    Q_OBJECT
public:
    // Item高度
    static constexpr int ItemHeight = 40;
    // Item与listview宽度差
    static constexpr int ItemWidthDiff = 64;
    // Item编辑状态边距
    static constexpr int ItemEditMargin = 5;
    // Icon边长
    static constexpr int ItemIconSide = 20;
public:
    explicit MusicSongListView(QWidget *parent = Q_NULLPTR);
    ~MusicSongListView() override;

    void showContextMenu(const QPoint &pos);
    void adjustHeight();
    bool getHeightChangeToMax();
public slots:
    void setThemeType(int type);

    // 新建歌单
    void addNewSongList();
    void changeCdaSongList(int stat);
    // 删除歌单
    void rmvSongList();
    void slotUpdatePlayingIcon();
    void slotMenuTriggered(QAction *action);
    void slotCurrentChanged(const QModelIndex &cur, const QModelIndex &pre);
    void slotDoubleClicked(const QModelIndex &index);
    // 重命名完成
    void slotLineEditingFinished();
    // cda状态变更弹窗
    void slotPopMessageWindow(int stat);
signals:
    void sigAddNewSongList();
    void sigRmvSongList();
    void currentChanged(const QModelIndex &current, const QModelIndex &previous) Q_DECL_OVERRIDE;

protected:
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
    // 实现delete快捷操作
    void keyReleaseEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    void dragEnterEvent(QDragEnterEvent *event) Q_DECL_OVERRIDE;
    void dragMoveEvent(QDragMoveEvent *event) Q_DECL_OVERRIDE;
    void dropEvent(QDropEvent *event) Q_DECL_OVERRIDE;
private:
    // 初始化快捷键
    void init();
    // 初始化快捷键
    void initShortcut();
    // 初始化重命名控件
    void initRenameLineEdit();

    // 重命名快捷键
    void slotRenameShortcut();
    // ESC快捷键
    void slotEscShortcut();

//    void setAttrRecur(QDomElement elem, QString strtagname, QString strattr, QString strattrval);
    QString newDisplayName();

private:
    MusicBaseAndSonglistModel *m_model = nullptr;
    DStyledItemDelegate  *m_delegate        = nullptr;
    // 重命名的Item
    DStandardItem        *m_renameItem = nullptr;
    // 重命名控件
    DLineEdit            *m_renameLineEdit = nullptr;

    // 新建歌单快捷键
//    QShortcut           *m_newItemShortcut = nullptr;
    // 重命名快捷键
    QShortcut           *m_renameShortcut = nullptr;
    // ESC快捷键
    QShortcut           *m_escShortcut = nullptr;

    bool                pixmapState         = false;
    bool                m_heightChangeToMax = false;
};

