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

#include <DListView>

#include "commonservice.h"
#include "databaseservice.h"

DWIDGET_USE_NAMESPACE

class AlbumDataModel;
class AlbumDataDelegate;
class MusicListDialog;
class AlbumListView : public DListView
{
    Q_OBJECT
public:
    explicit AlbumListView(QString hash, QWidget *parent = Q_NULLPTR);
    ~AlbumListView();
    /**
     * @brief setAlbumListData set album data from DataBaseService to delegate
     * @param listinfo all album data
     */
    void setAlbumListData(QList<AlbumInfo> &&listinfo);
    //展示专辑名包含str的专辑
    void resetAlbumListDataByStr(const QString &searchWord);
    //展示包含列表中歌曲的专辑
    void resetAlbumListDataBySongName(const QList<MediaMeta> &mediaMetas);
    //展示包含列表中歌手的专辑
    void resetAlbumListDataBySinger(const QList<SingerInfo> &singerInfos);

    QList<AlbumInfo> getAlbumListData() const;

    int getMusicCount();
    // 获取列表中专辑数量
    int getAlbumCount();
    void setViewModeFlag(QListView::ViewMode mode);
    QListView::ViewMode getViewMode();

    MediaMeta playing() const;
    //MediaMeta hoverin() const;

    void setThemeType(int type);
    int getThemeType() const;

    //void setPlayPixmap(QPixmap pixmap, QPixmap sidebarPixmap, QPixmap albumPixmap);
    QPixmap getPlayPixmap() const;
    //QPixmap getSidebarPixmap() const;
    QPixmap getPlayPixmap(bool isSelect = false);
    void updateList();

    //排序
    DataBaseService::ListSortType getSortType();
    void setSortType(DataBaseService::ListSortType sortType);
    // 根据排序添加数据
    void setDataBySortType(QList<AlbumInfo> &albumInfos, DataBaseService::ListSortType sortType);
    QString getHash() const;

signals:
    void requestCustomContextMenu(const QPoint &pos);
    void modeChanged(int);
public slots:
    // 跳转到当前播放歌曲位置
    void slotScrollToCurrentPosition(QString songlistHash);
    // 接收编码变更，更新对应项编码
    void slotUpdateCodec(const MediaMeta &meta);
private slots:
    void slotCoverUpdate(const MediaMeta &meta);
    void slotRemoveSingleSong(const QString &listHash, const QString &musicHash);
protected:
    void dragEnterEvent(QDragEnterEvent *event) Q_DECL_OVERRIDE;
    void dragMoveEvent(QDragMoveEvent *event) Q_DECL_OVERRIDE;
    void dropEvent(QDropEvent *event) Q_DECL_OVERRIDE;
private:
    // 排序
    void sortList(QList<AlbumInfo> &albumInfos, const DataBaseService::ListSortType &sortType);
private:
    int                     musicTheme     = 1; //light theme
    AlbumDataModel          *albumModel    = nullptr;
    AlbumDataDelegate       *albumDelegate = nullptr;
    MediaMeta                 playingMeta;
    MediaMeta                 hoverinMeta;
    QPixmap                 playingPix = QPixmap(":/texts/music_play1_20px.svg");
//    MusicListDialog        *musciListDialog = nullptr;
    QString                 m_hash;
    QListView::ViewMode     m_viewModel = QListView::ListMode;
    QIcon                   m_defaultIcon = QIcon::fromTheme("cover_max");
};
