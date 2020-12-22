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
    explicit PlayListView(QString hash, bool isPlayQueue, QWidget *parent = Q_NULLPTR);
    ~PlayListView() override;

    void setThemeType(int type);
    int getThemeType() const;

    QStandardItem *item(int row, int column) const;
    void setCurrentItem(QStandardItem *item);
    //性能优化歌曲展示专用
    void initAllSonglist(QString hash);
    //歌单&收藏页面
    void initCostomSonglist(const QString &hash);
    //展示歌名包含str的歌曲
    void resetSonglistByStr(const QString &searchWord);
    //展示专辑包含的歌曲
    void resetSonglistByAlbum(const QList<AlbumInfo> &albuminfos);
    //展示歌手的歌曲
    void resetSonglistBySinger(const QList<SingerInfo> &singerInfos);
    //获取展示中的歌曲列表
    QList<MediaMeta> getMusicListData();
    // 根据排序添加数据
    void setDataBySortType(QList<MediaMeta> &mediaMetas, DataBaseService::ListSortType sortType);

    QPixmap getSidebarPixmap();
    //zy---begin
    QPixmap getPlayPixmap(bool isSelect = false);
    void playListChange();
    //读写当前hash，用来判断当前显示的是什么列表
    void setCurrentHash(QString hash);
    QString getCurrentHash();
    //读写不同列表显示模式
    void setViewModeFlag(QString hash, QListView::ViewMode mode);
    QListView::ViewMode getViewMode();
    //排序
    DataBaseService::ListSortType getSortType();
    void setSortType(DataBaseService::ListSortType sortType);
    //获取收藏名字
    QString getFavName();
    //弹出错误提示框，并播放下一首
    void showErrorDlg();
public slots:
    void slotOnDoubleClicked(const QModelIndex &index);
    void slotUpdatePlayingIcon();

    void slotImportFinished(QString hash);//新导入文件
    void slotCoverUpdate(const MediaMeta &meta);//封面图片刷新
    void slotTheme(int type);

    // 删除单个歌单中的歌曲
    void slotRemoveSingleSong(const QString &listHash, const QString &musicHash);
    ///右键菜单响应begin
    // 添加到播放队列
    void slotAddToPlayQueue();
    // 添加收藏
    void slotAddToFavSongList(const QString songName);
    // 添加到新歌单
    void slotAddToNewSongList(const QString songName);
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
    void slotPlayQueueMetaRemove(QString metaHash);
    // 右键菜单设置音乐编码
    void slotTextCodecMenuClicked(QAction *action);
    // 右键菜单添加到歌单
    void slotPlaylistMenuClicked(QAction *action);
    ///右键菜单响应end

private:
    void insertRow(int row, MediaMeta meta);
    bool isContain(QString hash);
signals:
    bool musicResultListCountChanged(QString hash);
    //zy---end
    void customSort();
    void hideEmptyHits(bool ishide);
    void getSearchData(bool isvalid); //get search data
public:
    bool getIsPlayQueue() const;
    void reflushItemMediaMeta(const MediaMeta &meta);

protected:
    virtual void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    virtual void dragEnterEvent(QDragEnterEvent *event) Q_DECL_OVERRIDE;
    virtual void startDrag(Qt::DropActions supportedActions) Q_DECL_OVERRIDE;
    virtual void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    virtual void keyboardSearch(const QString &search) Q_DECL_OVERRIDE;
    void contextMenuEvent(QContextMenuEvent *event) Q_DECL_OVERRIDE;

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
//    // 移出歌单快捷键
//    QShortcut           *m_pRmvSongsShortcut = nullptr;
    // 显示菜单快捷键
    //QShortcut           *m_pShowMenuShortcut = nullptr;

    QPixmap m_sidebarPixmap;
    const QPixmap m_albumPixmap = QPixmap(":/mpimage/light/music_white_album_cover/music1.svg");
    QIcon     m_defaultIcon = QIcon(":/common/image/cover_max.svg");
};
