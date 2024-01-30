// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QDomElement>
#include <QTimer>
#include <QAbstractItemView>
#include <QMimeData>

#include <DListView>
#include <DLineEdit>
#include <DGuiApplicationHelper>

DGUI_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

class MusicBaseAndSonglistModel;

class MusicItemDelegate : public DStyledItemDelegate
{
    Q_OBJECT
public:
    explicit MusicItemDelegate(QAbstractItemView *parent = nullptr);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

class QShortcut;
// 自定义歌单列表
class MusicSongListView : public DListView
{
    Q_OBJECT
public:
    static constexpr int ItemHeight = 40;          // Item高度
    static constexpr int CompactItemHeight = 24;   // 紧凑模式Item高度
    static constexpr int ItemWidthDiff = 64;       // Item与listview宽度差
    static constexpr int ItemEditMargin = 5;       // Item编辑状态边距
    static constexpr int CompactItemEditMargin = 2;// 紧凑模式Item编辑状态边距
    static constexpr int ItemIconSide = 20;        // Icon边长

public:
    explicit MusicSongListView(QWidget *parent = Q_NULLPTR);
    ~MusicSongListView() override;

    void showContextMenu(const QPoint &pos);
    void adjustHeight();
    bool getHeightChangeToMax();
    int highlightedRow() const;
    // 拖拽表格数据
    void dropItem(int preRow);
    // 获取拖拽图片
    QPixmap dragItemPixmap();

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
    // 更新滚动条
    void slotUpdateDragScroll();
#ifdef DTKWIDGET_CLASS_DSizeMode
    void slotSizeModeChanged(DGuiApplicationHelper::SizeMode sizeMode);
#endif

signals:
    void sigAddNewSongList();
    void sigRmvSongList();
    void sigUpdateDragScroll();
    void currentChanged(const QModelIndex &current, const QModelIndex &previous) Q_DECL_OVERRIDE;
    void sigThemeTypeChanged(int type);

protected:
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
    // 实现delete快捷操作
    void keyReleaseEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    void startDrag(Qt::DropActions supportedActions) Q_DECL_OVERRIDE;
    void dragEnterEvent(QDragEnterEvent *event) Q_DECL_OVERRIDE;
    void dragMoveEvent(QDragMoveEvent *event) Q_DECL_OVERRIDE;
    void dragLeaveEvent(QDragLeaveEvent *event) Q_DECL_OVERRIDE;
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

    QString newDisplayName();

private:
    friend MusicItemDelegate;
    MusicBaseAndSonglistModel *m_model          = nullptr;
    MusicItemDelegate         *m_delegate       = nullptr;
    DStandardItem             *m_renameItem     = nullptr; // 重命名的Item
    DLineEdit                 *m_renameLineEdit = nullptr; // 重命名控件
    QShortcut                 *m_renameShortcut = nullptr; // 重命名快捷键
    QShortcut                 *m_escShortcut    = nullptr; // ESC快捷键
    QTimer                     m_dragScrollTimer;

    bool                      pixmapState         = false;
    bool                      m_heightChangeToMax = false;
    bool                      m_isDraging         = false;
    QModelIndex               m_preIndex;
};

