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

#include <DWidget>
#include <DListView>
#include <DToolButton>
#include <DLabel>
#include <QHBoxLayout>
#include <QResizeEvent>

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
class SubSonglistWidget;
class MusicListDataWidget : public DWidget
{
    Q_OBJECT
public:
    explicit MusicListDataWidget(QWidget *parent = Q_NULLPTR);
    ~MusicListDataWidget() override;
    void showEmptyHits(int count);
    void initInfoLabel(QString hash);
public slots:
    void slotTheme(int type);
    // 左侧菜单切换ListView
    void slotViewChanged(ListPageSwitchType switchtype, const QString &hashOrSearchword, QMap<QString, MediaMeta> musicinfos);
    void switchViewModel();
    void slotSortChange(QAction *action);
    void slotImportFinished(QString hash, int successCount);
    // 刷新各列表数量信息
    void refreshInfoLabel(QString hash);
    // 刷新显示模式按钮check状态
    void refreshModeBtnByHash(const QString &hash);
    // 刷新播放所有按钮enable状态
    void refreshPlayAllBtn(int count);
    // 删除了一首音乐
    void slotRemoveSingleSong(const QString &listHash, const QString &musicHash);
    void slotPlaylistNameUpdate(const QString &listHash);
protected:
    bool eventFilter(QObject *o, QEvent *e) Q_DECL_OVERRIDE;
protected:
    virtual void dragEnterEvent(QDragEnterEvent *event) Q_DECL_OVERRIDE;
    virtual void dropEvent(QDropEvent *event) Q_DECL_OVERRIDE;
    virtual void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
private slots:
    // 播放所有歌曲
    void slotPlayAllClicked();
    // 歌曲列表数量变化
    void slotMusicRowCountChanged();
private:
    void initUI();
    void initTitle(QHBoxLayout *layout);
    void initAlbumAction(QHBoxLayout *layout);
    void initArtistAction(QHBoxLayout *layout);
    void initMusicAction(QHBoxLayout *layout);
    void initBtPlayAll(QHBoxLayout *layout);
    void initCountLabel(QHBoxLayout *layout);
    void initListIconMode(QHBoxLayout *layout);
    void initemptyHits(QVBoxLayout *layout);
    // 刷新显示模式按钮check状态
    void refreshModeBtn(DListView::ViewMode mode);
    // 刷新当前排序菜单
    void refreshSortAction(const QString &hash = "");
private:
    DWidget             *m_contentWidget   = nullptr;
    QStackedWidget      *m_pStackedWidget  = nullptr;
    DLabel              *m_emptyHits       = nullptr;
    ActionBar           *m_actionBar       = nullptr;
    // 歌手专辑二级页面
    SubSonglistWidget   *m_subSonglistWidget = nullptr;
    QWidget             *m_lableWidget     = nullptr;
    DLabel              *m_titleLabel      = nullptr;
    DDropdown           *m_albumDropdown   = nullptr;
    DDropdown           *m_artistDropdown  = nullptr;
    DDropdown           *m_musicDropdown   = nullptr;
    DPushButton         *m_btPlayAll       = nullptr;
    DLabel              *m_infoLabel       = nullptr;
    DToolButton         *m_btIconMode      = nullptr;
    DToolButton         *m_btlistMode      = nullptr;

    AlbumListView       *m_albumListView  = nullptr;
    SingerListView      *m_singerListView = nullptr;
    PlayListView        *m_musicListView  = nullptr;
    SearchResultTabWidget *m_searchResultTabWidget = nullptr;

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

