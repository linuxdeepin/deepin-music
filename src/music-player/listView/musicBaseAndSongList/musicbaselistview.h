// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <DListView>
#include <QDomElement>

#include "commonservice.h"
DWIDGET_USE_NAMESPACE

class MusicBaseAndSonglistModel;
class QStackLayout;
//音乐库列表
class MusicBaseListView : public DListView
{
    Q_OBJECT
public:
    explicit MusicBaseListView(QWidget *parent = Q_NULLPTR);
    ~MusicBaseListView() override;

    void init();
    void showContextMenu(const QPoint &pos);
public slots:
    void setThemeType(int type);
    void slotUpdatePlayingIcon();
    void slotMenuTriggered(QAction *action);
    void slotItemClicked(const QModelIndex &index);
    void viewChanged(ListPageSwitchType switchtype, QString hashOrSearchword);

protected:
//    virtual void startDrag(Qt::DropActions supportedActions) Q_DECL_OVERRIDE;
//    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

//    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    void dragEnterEvent(QDragEnterEvent *event) Q_DECL_OVERRIDE;
    void dragMoveEvent(QDragMoveEvent *event) Q_DECL_OVERRIDE;
    void dropEvent(QDropEvent *event) Q_DECL_OVERRIDE;

private:
//    void SetAttrRecur(QDomElement elem, QString strtagname, QString strattr, QString strattrval);
    void setActionDisabled(const QString &hash, QAction *act);
private:
    //    QList<PlaylistPtr >  allPlaylists;
    MusicBaseAndSonglistModel *model = nullptr;
    DStyledItemDelegate  *delegate        = nullptr;
    QStandardItem        *playingItem     = nullptr;
    //QStandardItem      *m_currentitem = nullptr;
    bool                m_sizeChangedFlag   = false;
    bool                pixmapState         = false;
};

