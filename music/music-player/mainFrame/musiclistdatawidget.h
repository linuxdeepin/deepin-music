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
#include <DListView>
#include <DToolButton>
#include <DLabel>
#include <QHBoxLayout>

#include "mediameta.h"
DWIDGET_USE_NAMESPACE

class ActionBar;
class DDropdown;
class PlayListView;
class AlbumListView;
class SingerListView;
class SearchResultTabWidget;
class QShortcut;
class InfoDialog;
class MusicListDataWidget : public DWidget
{
    Q_OBJECT
public:
    explicit MusicListDataWidget(QWidget *parent = Q_NULLPTR);
    ~MusicListDataWidget() override;
    void showEmptyHits(int count);
public slots:
    void slotTheme(int type);
    // 左侧菜单切换ListView
    void viewChanged(ListPageSwitchType switchtype, const QString &hashOrSearchword);
    void switchViewModel();
    void slotSortChange(QAction *action);
    void slotImportFinished();
    //刷新各列表数量信息
    void refreshInfoLabel(QString hash);
    void slotRemoveSingleSong(const QString &listHash, const QString &musicHash);//删除了一首音乐
    void slotPlaylistNameUpdate(const QString &listHash);
protected:
    bool eventFilter(QObject *o, QEvent *e) Q_DECL_OVERRIDE;
protected:
    virtual void dragEnterEvent(QDragEnterEvent *event) Q_DECL_OVERRIDE;
    virtual void dropEvent(QDropEvent *event) Q_DECL_OVERRIDE;

private slots:
    // 播放所有歌曲
    void onPlayAllClicked();
private:
    void initUI();
    void initTitle(QHBoxLayout *layout);
    void initAlbumAction(QHBoxLayout *layout);
    void initArtistAction(QHBoxLayout *layout);
    void initMusicAction(QHBoxLayout *layout);
    void initBtPlayAll(QHBoxLayout *layout);
    void initCountLabelAndListMode(QHBoxLayout *layout);
    void initemptyHits(QVBoxLayout *layout);
    //刷新显示模式按钮check状态
    void refreshModeBtn(DListView::ViewMode mode);
    //刷新当前排序菜单
    void refreshSortAction(QString hash = "");
private:
    DWidget             *m_contentWidget   = nullptr;
    QStackedWidget      *m_pStackedWidget  = nullptr;
    DLabel              *m_emptyHits       = nullptr;
    ActionBar           *m_actionBar       = nullptr;
    DLabel              *m_titleLabel      = nullptr;
    DDropdown           *m_albumDropdown   = nullptr;
    DDropdown           *m_artistDropdown  = nullptr;
    DDropdown           *m_musicDropdown   = nullptr;
    SearchResultTabWidget *m_SearchResultTabWidget = nullptr;
    DPushButton         *m_btPlayAll       = nullptr;
    DLabel              *m_infoLabel       = nullptr;
    DToolButton         *m_btIconMode      = nullptr;
    DToolButton         *m_btlistMode      = nullptr;

//    MusicListDataView   *albumListView  = nullptr;
//    MusicListDataView   *artistListView = nullptr;
    AlbumListView       *m_albumListView  = nullptr; //album view
    SingerListView      *m_singerListView = nullptr; //singer view
    PlayListView        *m_musicListView  = nullptr; //all music view
    QAction             *m_customAction    = nullptr;
    bool                m_updateFlag       = false;
//    int                 m_preIndex = 0;//记录前一次显示，当清除搜索时回到上一页面
    QString             m_currentHash;
    ListPageSwitchType  m_preSwitchtype = AllSongListType;
    QString             m_preHash = "all";//记录前一次显示，当清除搜索时回到上一页面
};

class ActionBar : public DWidget
{
    Q_OBJECT
public:
    explicit ActionBar(QWidget *parent = Q_NULLPTR);
    ~ActionBar() {}
private:
    bool MoveFlag = false;

protected:
    //Filter the mousemoveevent
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
};

