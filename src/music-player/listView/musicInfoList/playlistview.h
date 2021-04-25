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

#include "commonservice.h"
#include "databaseservice.h"

DWIDGET_USE_NAMESPACE

class PlaylistModel;
class PlayItemDelegate;
class InfoDialog;
class QShortcut;
class PlayListView : public DListView
{
    Q_OBJECT
public:
    explicit PlayListView(const QString &hash, bool isPlayQueue, QWidget *parent = Q_NULLPTR);
    ~PlayListView() override;

    void setThemeType(int type);
    int getThemeType() const;

    QStandardItem *item(int row, int column) const;
    //void setCurrentItem(QStandardItem *item);
    void reloadAllSonglist();
    // 性能优化歌曲展示专用
    void initAllSonglist(const QString &hash);
    // 歌单&收藏页面
    void initCostomSonglist(const QString &hash);
    // 展示歌名包含str的歌曲
    void resetSonglistByStr(const QString &searchWord);
    // 展示专辑包含的歌曲
    void resetSonglistByAlbum(const QList<AlbumInfo> &albuminfos);
    // 展示歌手的歌曲
    void resetSonglistBySinger(const QList<SingerInfo> &singerInfos);
    // 获取展示中的歌曲列表
    QList<MediaMeta> getMusicListData();
    // 根据排序添加数据
    QList<MediaMeta> setDataBySortType(QList<MediaMeta> &mediaMetas, DataBaseService::ListSortType sortType);

    //QPixmap getSidebarPixmap();
    //zy---begin
    QPixmap getPlayPixmap(bool isSelect = false);
    void playListChange();
    // 读写当前hash，用来判断当前显示的是什么列表
    //void setCurrentHash(QString hash);
    QString getCurrentHash();
    // 读写不同列表显示模式
    void setViewModeFlag(QString hash, QListView::ViewMode mode);
    QListView::ViewMode getViewMode();
    // 排序
    DataBaseService::ListSortType getSortType();
    void setSortType(DataBaseService::ListSortType sortType);
    // 弹出错误提示框，并播放下一首
    void showErrorDlg();
    // 获取列表中歌曲数量
    int getMusicCount();
    // 二级页面添加歌曲，设置hash
    void setMusicListView(QMap<QString, MediaMeta> musicinfos, const QString &hash);
    // 设置当前列表属于哪个页面
    void setListPageSwitchType(ListPageSwitchType type);
    // 获取是否播放队列
    bool getIsPlayQueue();
    // 平板选中项
    QModelIndexList tabletSelectedIndexes();
public slots:
    void slotOnClicked(const QModelIndex &index);

    void slotOnDoubleClicked(const QModelIndex &index);

    void slotLoadData();
    void slotUpdatePlayingIcon();

    // 封面图片刷新
    void slotCoverUpdate(const MediaMeta &meta);
    void slotTheme(int type);

    // 删除单个歌单中的歌曲
    void slotRemoveSingleSong(const QString &listHash, const QString &musicHash);
    // 增加歌曲
    void slotMusicAddOne(const QString &listHash, MediaMeta addMeta);
    // 跳转到当前播放歌曲位置
    void slotScrollToCurrentPosition(const QString &songlistHash);
    //右键菜单响应begin
    // 添加到播放队列
    void slotAddToPlayQueue();
    // 添加收藏
    void slotAddToFavSongList(const QString &songName);
    // 添加到新歌单
    void slotAddToNewSongList(const QString &songName);
    // 播放音乐
    void slotPlayMusic();
    // 添加到其他歌单
    void slotAddToCustomSongList();
    // 在文件管理起查看
    void slotOpenInFileManager();
    // 从歌单移出
    void slotRmvFromSongList();
    // 从本地删除
    void slotDelFromLocal();
    // 查看歌曲信息
    void showDetailInfoDlg();
    // 播放队列中歌曲被删除
    void slotPlayQueueMetaRemove(const QString &metaHash);
    // 播放状态发生改变
    void slotPlaybackStatusChanged(Player::PlaybackStatus statue);
    // 右键菜单设置音乐编码
    void slotTextCodecMenuClicked(QAction *action);
    // 右键菜单添加到歌单
    void slotPlaylistMenuClicked(QAction *action);
    ///右键菜单响应end
    // 右键编码通知其他页面编码同步
    void slotUpdateCodec(const MediaMeta &meta);
    // 删除cda相关歌曲
    void slotRmvCdaSongs();
    // 设置选择模式
    void slotSetSelectModel(CommonService::TabletSelectMode model);
    // 全选
    void slotSelectAll();
    // 显示歌单列表
    void slotShowSongList();
    // 添加到其他歌单或者播放队列
    void slotAddToSongList(const QString &hash, const QString &name);
    // 横竖屏切换
    void slotHScreen(bool isHScreen);
private:
    // 播放音乐相关处理
    void playMusic(const MediaMeta &meta);
    void insertRow(int row, MediaMeta meta);
    bool isContain(const QString &hash);
    // 排序
    void sortList(QList<MediaMeta> &musicInfos, const DataBaseService::ListSortType &sortType);
    // 平板清除选中
    void tabletClearSelection();
signals:
    bool musicResultListCountChanged(QString hash);
    //zy---end
    void customSort();
    void hideEmptyHits(bool ishide);
    void getSearchData(bool isvalid); //get search data
public:
    bool getIsPlayQueue() const;
    //void reflushItemMediaMeta(const MediaMeta &meta);
    bool getMenuIsShow();
protected:
    virtual void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    virtual void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    virtual void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    virtual void dragEnterEvent(QDragEnterEvent *event) Q_DECL_OVERRIDE;
    virtual void startDrag(Qt::DropActions supportedActions) Q_DECL_OVERRIDE;
    virtual void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    //virtual void keyboardSearch(const QString &search) Q_DECL_OVERRIDE;
    void contextMenuEvent(QContextMenuEvent *event)Q_DECL_OVERRIDE;

    void dragMoveEvent(QDragMoveEvent *event) Q_DECL_OVERRIDE;
    void dropEvent(QDropEvent *event) Q_DECL_OVERRIDE;
private:
    PlaylistModel           *m_model        = nullptr;
    PlayItemDelegate        *m_delegate     = nullptr;
    int                      m_themeType    = 1;
    bool                     m_IsPlayQueue;
    // 根据hash区分所有，收藏与自定义歌单
    QString                  m_currentHash;
    QMap<QString, QListView::ViewMode> m_viewModeMap;
    // 详细信息窗口
    InfoDialog          *m_pInfoDlg = nullptr;
    // 详细信息快捷键
    QShortcut           *m_pDetailShortcut = nullptr;
    // 判断歌曲列表是哪个页面
    ListPageSwitchType   m_listPageType;
    QPixmap m_sidebarPixmap;
    QIcon     m_defaultIcon = QIcon::fromTheme("cover_max");
    bool      m_menuIsShow = false;
    static constexpr int FirstLoadCount = 15;
    QModelIndex m_pressIndex;
};
