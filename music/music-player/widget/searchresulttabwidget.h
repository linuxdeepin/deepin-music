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

#include <DWidget>
#include <QStackedWidget>
#include <DHorizontalLine>
#include <DListView>
#include "mediameta.h"
#include "databaseservice.h"

DWIDGET_USE_NAMESPACE
class Label;
class LineWidget;
class AlbumListView;
class SingerListView;
class PlayListView;
class SearchResultTabWidget : public DWidget
{
    Q_OBJECT
public:
    explicit SearchResultTabWidget(QWidget *parent = nullptr);
    ~SearchResultTabWidget();

    void refreshListview(ListPageSwitchType switchtype, QString searchword);
    void setViewMode(DListView::ViewMode mode);
    void setSortType(DataBaseService::ListSortType sortType);
    //排序
    DataBaseService::ListSortType getSortType();
    int  getMusicCount();
    int  getAlbumCount();
    int  getSingerCount();
public slots:

signals:
    //通知主界面搜索结果tab切换，设计排序与数量显示
    void sigSearchTypeChanged(QString hash);

protected:
    virtual void showEvent(QShowEvent *event) Q_DECL_OVERRIDE;
    virtual void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
    virtual void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
private:
    void setLabelChecked(Label *label);
private:
    Label *m_music = nullptr;
    QRect m_musicRect;
    Label *m_singer = nullptr;
    QRect m_musicSinger;
    Label *m_album = nullptr;
    QRect m_musicAlbum;
    DHorizontalLine *m_Line = nullptr;
    QStackedWidget      *m_StackedWidget   = nullptr;
    LineWidget *m_LineWidget = nullptr;

    AlbumListView       *m_albumListView  = nullptr; //album view
    SingerListView      *m_singerListView = nullptr; //singer view
    PlayListView        *m_musicListView  = nullptr; //music view
    QString              m_searchWord;
    ListPageSwitchType           m_switchtype;
};
