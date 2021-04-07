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

