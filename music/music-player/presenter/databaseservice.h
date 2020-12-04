/*
* Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
*
* Author: Zou Ya<zouya@uniontech.com>
*
* Maintainer: Zou Ya <zouya@uniontech.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program. If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef DATABASESERVICE_H
#define DATABASESERVICE_H

#include <QObject>
#include <QMap>
#include <QSqlDatabase>
#include <QModelIndexList>

class PlaylistMeta;
class MediaMeta;
class AlbumInfo;
class SingerInfo;
class MediaLibrary;
class DataBaseService : public QObject
{
    Q_OBJECT
public:
    struct PlaylistData {
        QString uuid;
        QString displayName;
        QString icon;

        QStringList              sortMetas;

//        QMap<QString, int>       invalidMetas;
        int     sortType    = 0;
        int     orderType   = 0;

        uint     sortID;
        bool    editmode    = false;
        bool    readonly    = false;
        bool    hide        = false;
        bool    active      = false;
        bool    playStatus  = false;
    };

    enum ListSortType {
        SortByNull = -1,
        SortByAddTime,
        SortByTitle,
        SortBySinger,
        SortByAblum,
        SortByCustom,
    };
public:
    QSqlDatabase            getDatabase();
    static DataBaseService *getInstance();

    //所有歌曲信息
    QList<MediaMeta>     allMusicInfos();
    int                  allMusicInfosCount();                         //歌曲数量

    //专辑
    QList<AlbumInfo> allAlbumInfos();
    //演唱者
    QList<SingerInfo> allSingerInfos();

    //删除歌单所选歌曲
    void removeSelectedSongs(const QString &curpage, const QStringList &hashlist);
    //删除播放历史歌曲
    void removeHistorySelectedSong(const QStringList &hashlist);

    void getAllMediaMetasInThread();

    //导入
    void                 importMedias(const QStringList &urllist);
    void                 addMediaMeta(const MediaMeta &meta);
    QList<MediaMeta>     getNewLoadMusicInfos();                          //获取新导入文件

    // 音乐收藏
    void favoriteMusic(const MediaMeta meta);
    // 音乐是否收藏
    bool favoriteExist(const MediaMeta meta);
    //歌单相关
    QString              getPlaylistNameByUUID(QString uuid);
    uint                 getPlaylistMaxSortid();
    QList<PlaylistData>  customSongList(); //获取自定义歌单
    QList<MediaMeta>     customizeMusicInfos(const QString &hash);         //获取歌单歌曲&获取收藏歌曲
    void                 addPlaylist(const PlaylistData &playlistMeta);    //添加一个歌单
    QList<PlaylistData>  allPlaylistMeta();                                //歌单信息
    void                 updatePlaylistSortType(int type, QString uuid);   //更新歌单排序类型，如按歌手，时间排序
    int                  getPlaylistSortType(QString uuid);                //获取歌单排序类型
//    void                 updatePlaylistOrderType(int type, QString uuid);  //更新歌单显示类型，list，icon，未使用框架QListView::ViewMode
//    int                  getPlaylistOrderType(QString uuid);               //获取歌单QListView::ViewMode
public slots:
    void slotGetAllMediaMetaFromThread(QList<MediaMeta> allMediaMeta);
    void slotGetMetaFromThread(MediaMeta meta);
    void slotImportFinished();//收到子线程导入结束通知
    void slotCreatOneCoverImg(MediaMeta meta);//收到图片加载完信号
signals:
    void sigGetAllMediaMeta();
    void sigImportMedias(const QStringList &urllist);//发送给子线程执行
    void sigCreatCoverImg(const QList<MediaMeta> &metas);//发送给子线程执行创建图片
    void sigImportFinished();//导入结束，通知主界面
    void sigCoverUpdate(const MediaMeta &meta);//封面图片刷新
    void sigRmvSong(const QString &str);//发送删除歌曲通知消息，动态显示
private:
    bool createConnection();
    bool playlistExist(const QString &uuid);
    void initPlaylistTable();//初始化歌单统计表
public:
    QMap<QString, MediaMeta> m_MediaMetaMap;
private:
    DataBaseService();
    static DataBaseService *instance;
    QSqlDatabase m_db;

    QList<PlaylistData> m_PlaylistMeta;
    QList<MediaMeta> m_AllMediaMeta;
    QList<AlbumInfo> m_AllAlbumInfo;
    QList<SingerInfo> m_AllSingerInfo;
    QList<MediaMeta> m_AllFavourite;

    QList<MediaMeta> m_loadMediaMeta;  //新加载的歌曲文件
};

Q_DECLARE_METATYPE(DataBaseService::ListSortType)

#endif // DATABASESERVICE_H
