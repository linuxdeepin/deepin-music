// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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

    void refreshListview();
    void refreshListview(ListPageSwitchType switchtype, const QString &searchword);
    void setViewMode(DListView::ViewMode mode);
    void setSortType(DataBaseService::ListSortType sortType);
    //排序
    DataBaseService::ListSortType getSortType();
    //显示模式
    QListView::ViewMode getViewMode();
    int  getMusicCountByMusic();
    int  getAlbumCount();
    int  getMusicCountByAlbum();
    int  getSingerCount();
    int  getMusicCountBySinger();
    QList<MediaMeta> getMusicListData();
    QList<MediaMeta> getCurMusicListData();
    QList<SingerInfo> getSingerListData();
    QList<AlbumInfo> getAlbumListData();

    void setCurrentPage(ListPageSwitchType type);
public slots:
    // 控件大小改变
    void slotSizeChange();
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
