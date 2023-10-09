// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <DScrollArea>
#include <DLabel>
#include <DIconButton>
#include <DGuiApplicationHelper>

#include "mediameta.h"
DWIDGET_USE_NAMESPACE

class MusicBaseListView;
class MusicSongListView;
class MusicListView;
class MusicListDataWidget;
class MusicImageButton;

class MusicListScrollArea : public DScrollArea
{
    Q_OBJECT
public:
    explicit MusicListScrollArea(QWidget *parent = Q_NULLPTR);

public slots:
    void slotTheme(int type);
    void slotListViewClicked(const QModelIndex &index);
    void slotAddNewSongList();
    // 切换到搜索结果界面
    void viewChanged(ListPageSwitchType switchtype, const QString &hashOrSearchword);
    void slotUpdateDragScroll();
#ifdef DTKWIDGET_CLASS_DSizeMode
    void slotSizeModeChanged(DGuiApplicationHelper::SizeMode sizeMode);
#endif

protected:
    bool eventFilter(QObject *o, QEvent *e) Q_DECL_OVERRIDE;
    void resizeEvent(QResizeEvent *e) Q_DECL_OVERRIDE;

private:
    MusicBaseListView  *m_dataBaseListview  = nullptr;
    MusicSongListView  *m_customizeListview = nullptr;
    DIconButton        *m_addListBtn;
    DLabel             *dataBaseLabel;
    DLabel             *customizeLabel;
};

