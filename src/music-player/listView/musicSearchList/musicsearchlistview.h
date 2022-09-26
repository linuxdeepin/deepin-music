// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <DListView>
#include <QPixmap>

#include "mediameta.h"
#include "searchresult.h"

//DWIDGET_USE_NAMESPACE

class MusicSearchListModel;
class MusicSearchListDelegate;
//class SearchResult;
class MusicSearchListview: public DListView
{
    Q_OBJECT
public:
    explicit MusicSearchListview(QWidget *parent = Q_NULLPTR);
    ~MusicSearchListview() override;

    void setSearchResultWidget(SearchResult *result);
    void setSearchType(SearchType type);
    SearchType getSearchType() const;
    void setSearchText(QString text);
    int rowCount();
    int getIndexInt()const;

    void setThemeType(int type);
    int getThemeType() const;
    QString getSearchText()const;

    void    setCurrentIndexInt(int row);
//    QString getCurrentIndexText(int row);

//    void setPlayPixmap(QPixmap pixmap, QPixmap sidebarPixmap, QPixmap albumPixmap);
//    QPixmap getPlayPixmap() const;
//    QPixmap getSidebarPixmap() const;
//    QPixmap getAlbumPixmap() const;
//    void updateList();
    //void keyChoice();

//    void setViewModeFlag(QListView::ViewMode mode);
//    void addItem(const QString str);
private:
    // 单击或者触屏单击时调用，跳转到搜索结果tab页
    void switchToSearchResultTab(const QModelIndex &index);
signals:
    void sigSearchClear();

public slots:
    void SearchClear();
    void slotOnClicked(const QModelIndex &index);
    void onReturnPressed();
protected:
    virtual void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    virtual bool event(QEvent *event)Q_DECL_OVERRIDE;
public:
    SearchType                  m_SearchType;
private:
    MusicSearchListModel        *m_model        = nullptr;
    MusicSearchListDelegate     *m_delegate     = nullptr;
    QString                     m_searchText     = "";
    int                         m_themeType     = 1;
    int                         m_CurrentIndex = -1;
    //QString                     m_defaultCover = ":/images/logo.svg";
    QPixmap                     m_defaultIcon = QPixmap(":/icons/deepin/builtin/actions/cover_max_200px.svg");
//    QPixmap                     m_playingPixmap = QPixmap(":/images/logo.svg");
//    QPixmap                     m_sidebarPixmap = QPixmap(":/images/logo.svg");
//    QPixmap                     m_albumPixmap   = QPixmap(":/images/logo.svg");

    QList<MediaMeta> m_MediaMetas;
    QList<AlbumInfo> m_AlbumInfos;
    QList<SingerInfo> m_SingerInfos;
    SearchResult                *m_SearchResultWidget = nullptr;
};

