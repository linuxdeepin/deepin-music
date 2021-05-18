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

#include "commonservice.h"
#include "databaseservice.h"

DWIDGET_USE_NAMESPACE

class SingerDataModel;
class SingerDataDelegate;

class SingerListView : public DListView
{
    Q_OBJECT
public:
    explicit SingerListView(const QString &hash, QWidget *parent = Q_NULLPTR);
    ~SingerListView();

    /**
     * @brief setSingerListData set singer data from DataBaseService to delegate
     * @param listinfo all singer data
     */
    void setSingerListData(QList<SingerInfo> &&listinfo);
    /**
     * @brief getSingerListData get all singer data
     * @return container of all singer
     */
    QList<SingerInfo> getSingerListData() const;
    //展示歌手名包含str的歌手
    void resetSingerListDataByStr(const QString &searchWord);
    //展示包含列表中歌曲相关的歌手
    void resetSingerListDataBySongName(const QList<MediaMeta> &mediaMetas);
    //展示包含列表中专辑相关的歌手
    void resetSingerListDataByAlbum(const QList<AlbumInfo> &albumInfos);

    int getMusicCount();

    void setViewModeFlag(QListView::ViewMode mode);
    QListView::ViewMode getViewMode();

    bool  playingState()const;

    void setThemeType(int type);
    int getThemeType() const;

    // void setPlayPixmap(QPixmap pixmap, QPixmap sidebarPixmap, QPixmap albumPixmap);
    QPixmap getPlayPixmap() const;
    //QPixmap getSidebarPixmap() const;
    QPixmap getPlayPixmap(bool isSelect = false);

    void updateList();

    //排序
    DataBaseService::ListSortType getSortType();
    void setSortType(DataBaseService::ListSortType sortType);
    // 根据排序添加数据
    void setDataBySortType(QList<SingerInfo> &singerInfos, DataBaseService::ListSortType sortType);
    QString getHash() const;
    // 获取列表中歌手数量
    int getSingerCount();
public slots:
    // 歌曲删除
    void slotRemoveSingleSong(const QString &listHash, const QString &musicHash);
    // 跳转到当前播放歌曲位置
    void slotScrollToCurrentPosition(const QString &songlistHash);
    // 刷新播放动态图
    void slotUpdatePlayingIcon();
    // 横竖屏切换
    void slotHScreen(bool isHScreen);
signals:
    void requestCustomContextMenu(const QPoint &pos);
    void modeChanged(int);
private slots:
    void slotCoverUpdate(const MediaMeta &meta);
    // 接收编码变更，更新对应项编码
    void slotUpdateCodec(const MediaMeta &meta);
    // 添加歌曲
    void slotAddSingleSong(const QString &listHash, const MediaMeta &addMeta);
    // 多选歌曲删除时逻辑
    void slotRemoveSelectedSongs(const QString &deleteHash, const QStringList &musicHashs, bool removeFromLocal);
    // 播放状态发生改变
    void slotPlaybackStatusChanged(Player::PlaybackStatus statue);
protected:
    void dragEnterEvent(QDragEnterEvent *event) Q_DECL_OVERRIDE;
    void dragMoveEvent(QDragMoveEvent *event) Q_DECL_OVERRIDE;
    void dropEvent(QDropEvent *event) Q_DECL_OVERRIDE;
private:
    // 排序
    void sortList(QList<SingerInfo> &singerInfos, const DataBaseService::ListSortType &sortType);
    // 判断map中是否包含list中的某项
    bool mapContainsList(QMap<QString, MediaMeta> metasMap, QStringList musicHashs);
private:
    int                     musicTheme      = 1; //light theme
    SingerDataModel         *singerModel    = nullptr;
    SingerDataDelegate      *signerDelegate = nullptr;
//    MusicListDialog        *musicListDialog = nullptr;
    QPixmap                 playingPix = QPixmap(":/texts/music_play1_20px.svg");
    QString                  m_hash;
    QListView::ViewMode      m_viewModel = QListView::ListMode;
    QIcon                   m_defaultIcon = QIcon::fromTheme("cover_max");
};
