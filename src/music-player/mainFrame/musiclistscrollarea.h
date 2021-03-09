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

#include <DScrollArea>
#include <DLabel>
#include <DIconButton>

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
protected:
    bool eventFilter(QObject *o, QEvent *e) Q_DECL_OVERRIDE;
    void resizeEvent(QResizeEvent *e) Q_DECL_OVERRIDE;

private:
    MusicBaseListView *m_dataBaseListview = nullptr;
    MusicSongListView *m_customizeListview = nullptr;
    DIconButton      *m_addListBtn;
    DLabel                *dataBaseLabel;
    DLabel                *customizeLabel;
};

