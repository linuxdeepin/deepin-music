// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <DListView>
#include <DGuiApplicationHelper>

#include "commonservice.h"
#include "databaseservice.h"

DGUI_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

class AlbumDataModel;
class AlbumDataDelegate;
class AlbumListView : public DListView
{
    Q_OBJECT
public:
    explicit AlbumListView(const QString &hash, QWidget *parent = Q_NULLPTR);
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

    QPixmap getPlayPixmap() const;
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
    void slotScrollToCurrentPosition(const QString &songlistHash);
    // 接收编码变更，更新对应项编码
    void slotUpdateCodec(const MediaMeta &meta);
    // 刷新播放动态图
    void slotUpdatePlayingIcon();
    // 播放状态发生改变
    void slotPlaybackStatusChanged(Player::PlaybackStatus statue);
    // 横竖屏切换
    void slotHScreen(bool isHScreen);
#ifdef DTKWIDGET_CLASS_DSizeMode
    // 切换紧凑模式
    void slotSizeModeChanged(DGuiApplicationHelper::SizeMode sizeMode);
#endif

private slots:
    void slotCoverUpdate(const MediaMeta &meta);
    void slotRemoveSingleSong(const QString &listHash, const QString &musicHash);
    // 添加歌曲
    void slotAddSingleSong(const QString &listHash, const MediaMeta &addMeta);
    // 多选歌曲删除时逻辑
    void slotRemoveSelectedSongs(const QString &deleteHash, const QStringList &musicHashs, bool removeFromLocal);

protected:
    void dragEnterEvent(QDragEnterEvent *event) Q_DECL_OVERRIDE;
    void dragMoveEvent(QDragMoveEvent *event) Q_DECL_OVERRIDE;
    void dropEvent(QDropEvent *event) Q_DECL_OVERRIDE;

private:
    // 排序
    void sortList(QList<AlbumInfo> &albumInfos, const DataBaseService::ListSortType &sortType);
    // 判断map中是否包含list中的某项
    bool mapContainsList(QMap<QString, MediaMeta> metasMap, QStringList musicHashs);

private:
    int                     musicTheme     = 1; //light theme
    AlbumDataModel          *albumModel    = nullptr;
    AlbumDataDelegate       *albumDelegate = nullptr;
    MediaMeta                 playingMeta;
    MediaMeta                 hoverinMeta;
    QPixmap                 playingPix = QPixmap(":/texts/music_play1_20px.svg");
    QString                 m_hash; //用于区分当前列表是album列表还是搜索中的album列表
    QListView::ViewMode     m_viewModel = QListView::ListMode;
    QIcon                   m_defaultIcon = QIcon::fromTheme("cover_max");
};
