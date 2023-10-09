// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QDomElement>

#include <DListView>
#include <DGuiApplicationHelper>

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
#ifdef DTKWIDGET_CLASS_DSizeMode
    void slotSizeModeChanged(DGuiApplicationHelper::SizeMode sizeMode);
#endif

protected:
    void dragEnterEvent(QDragEnterEvent *event) Q_DECL_OVERRIDE;
    void dragMoveEvent(QDragMoveEvent *event) Q_DECL_OVERRIDE;
    void dropEvent(QDropEvent *event) Q_DECL_OVERRIDE;

private:
    void setActionDisabled(const QString &hash, QAction *act);

private:
    MusicBaseAndSonglistModel *model             = nullptr;
    DStyledItemDelegate       *delegate          = nullptr;
    QStandardItem             *playingItem       = nullptr;
    bool                       m_sizeChangedFlag = false;
    bool                       pixmapState       = false;
};

