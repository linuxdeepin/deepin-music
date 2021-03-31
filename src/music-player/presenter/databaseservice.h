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

#ifndef DATABASESERVICE_H
#define DATABASESERVICE_H

#include <QObject>
#include <QMap>
#include <QSqlDatabase>
#include <QModelIndexList>
#include "dboperate.h"

#include "util/singleton.h"

class PlaylistMeta;
class MediaMeta;
class AlbumInfo;
class SingerInfo;
class MediaLibrary;
class DataBaseService : public QObject, public DMusic::DSingleton<DataBaseService>
{
    Q_OBJECT
public:
    struct PlaylistData {
        QString uuid;
        QString displayName;
        QString icon;

        QStringList              sortMetas;

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
        SortByAddTimeASC,
        SortByTitleASC,
        SortBySingerASC,
        SortByAblumASC,
        SortByCustomASC,
        SortByAddTimeDES,
        SortByTitleDES,
        SortBySingerDES,
        SortByAblumDES,
        SortByCustomDES,
        SortByAddTime,
        SortByTitle,
        SortBySinger,
        SortByAblum,
        SortByCustom,
    };
public:
    //QSqlDatabase            getDatabase();

    QList<MediaMeta>     getMusicInfosBySortAndCount(int count);
    // 所有歌曲信息
    QList<MediaMeta>     allMusicInfos(bool refresh = true);
    // 歌曲数量
    int                  allMusicInfosCount();
    // 通过hash值获取单个歌曲的信息
    MediaMeta            getMusicInfoByHash(const QString &hash);
    // 专辑
    QList<AlbumInfo> allAlbumInfos();
    // 演唱者
    QList<SingerInfo> allSingerInfos();

    // 删除歌单所选歌曲 removeFromLocal:是否从本地删除
    void removeSelectedSongs(const QString &curpage, const QStringList &musichashlist, bool removeFromLocal);

    // 导入
    void importMedias(const QString &importHash, const QStringList &urllist);
    // 获取导入状态，true正在导入，false没有导入，空闲
    //bool                 getImportStatus();
//    void                 addMediaMeta(const MediaMeta &meta);

    // 音乐收藏
    bool favoriteMusic(const MediaMeta meta);
    // 音乐是否收藏
    bool favoriteExist(const MediaMeta meta);
    // 歌单相关
    bool                 deleteMetaFromPlaylist(QString uuid, const QStringList &metaHash);
    QString              getPlaylistNameByUUID(const QString &uuid);
    uint                 getPlaylistMaxSortid();
    // 获取自定义歌单
    QList<PlaylistData>  getCustomSongList();
    // 获取歌单歌曲&获取收藏歌曲
    QList<MediaMeta>     customizeMusicInfos(const QString &hash);
    // 添加一个歌单
    void                 addPlaylist(const PlaylistData &playlistMeta);
    // 删除一个歌单,返回值确认是否删除成功
    bool                 deletePlaylist(const QString &hash);
    // 更新部分歌单
    void                 updatePlaylist(const QVector<PlaylistData> &playlistDataList);
    // 歌单信息
    QList<PlaylistData>  allPlaylistMeta();
    // 添加歌曲到歌单
    int                  addMetaToPlaylist(QString uuid, const QList<MediaMeta> &metas);
    // 更新歌单排序类型，如按歌手，时间排序
    void                 updatePlaylistSortType(int type, QString uuid);
    // 更新歌单名称
    void                 updatePlaylistDisplayName(QString displayname, QString uuid);
    // 获取歌单排序类型
    int                  getPlaylistSortType(QString uuid);
    // 获取歌单中歌曲数量
    int                  getPlaylistSongCount(QString uuid);
    // 判断歌曲是否存在于该自定义歌单,返回值确认是否存在
    bool                 isMediaMetaInSonglist(const QString &songlistHash, const QString &musicHash);
//    // 更新歌单显示类型，list，icon，未使用框架QListView::ViewMode
//    void                 updatePlaylistOrderType(int type, QString uuid);
//    // 获取歌单QListView::ViewMode
//    int                  getPlaylistOrderType(QString uuid);
    // 更新歌曲的编码方式
    void                 updateMetaCodec(const MediaMeta &meta);
    // 设置从文管导入后播放的第一首歌曲
    void                 setFirstSong(const QString &strurl);
    // 从文管导入后播放的第一首歌曲
    QString              getFirstSong();
    // 获取删除状态
    bool                 getDelStatus();
    // 获取删除中的歌曲hash
    QStringList          getDelMetaHashs();
    void                 setDelNeedSleep();
public slots:
    void slotGetMetaFromThread(MediaMeta meta);
    // 收到子线程导入结束通知
    void slotImportFinished(int failCount, int successCount, int exsitCount);
    // 收到子线程删除歌曲通知消息，动态显示
    void slotRmvSongThread(const QString &listHash, const QString &musicHash, bool removeFromLocal);
    // 收到子线程删除结束
    void slotDelFinish();
signals:
    // 所有歌曲数量变化
    void signalMusicAddOne(QString listHash, MediaMeta meta);
    // 所有歌曲被清空
    void signalAllMusicCleared();
    void signalGetAllMediaMeta();
    // 发送给子线程执行创建图片
    void signalCreatCoverImg(const QList<MediaMeta> &metas);
    // 导入结束，通知主界面 allCount:待导入的文件数量   successCount：导入成功的数量
    void signalImportFinished(QString hash, int successCount);
    // 导入失败，有不支持的文件
    void signalImportFailed();
    // 封面图片刷新
    void signalCoverUpdate(const MediaMeta &meta);
    // 发送删除歌曲通知消息，动态显示
    void signalRmvSong(const QString &listHash, const QString &musicHash);
    // 收藏中的歌曲被删除，动态显示
    void signalFavSongRemove(const QString &musicHash);
    // 收藏中的歌曲被删除，动态显示
    void signalFavSongAdd(QString musicHash);
    // 文管启动加载数据完成后直接播放歌曲
    void signalPlayFromFileMaganager();
    // 歌单名称改变
    void signalPlaylistNameUpdate(QString hash);
    // 已导入百分比
    void signalImportedPercent(int percent);
    // 删除结束
    void signalDelFinish();

    // 子线程执行
    // 导入歌曲
    void signalImportMedias(QString importHash, const QStringList &urllist);
    // 删除歌单所选歌曲 removeFromLocal:是否从本地删除
    void sigRemoveSelectedSongs(const QString &curpage, const QStringList &musichashlist, bool removeFromLocal);
private:
    bool createConnection();
    bool isPlaylistExist(const QString &uuid);
    // 初始化歌单统计表
    void initPlaylistTable();
public:
    QMap<QString, MediaMeta> m_MediaMetaMap;
private:
    explicit DataBaseService();
    ~DataBaseService();
    friend class DMusic::DSingleton<DataBaseService>;

private:
    QThread *m_workerThread = nullptr;
    QSqlDatabase m_db;
    QList<PlaylistData> m_PlaylistMeta;
    QList<MediaMeta> m_AllMediaMeta;
    QList<AlbumInfo> m_AllAlbumInfo;
    QList<SingerInfo> m_AllSingerInfo;
    QList<MediaMeta> m_AllFavourite;
    // 新加载的歌曲文件
    DBOperate m_worker;
// 不需要这个变量控制导入多次问题
//    bool             m_importing = false;
    QString          m_importHash;
    // 文管导入的第一首歌
    QString          m_firstSonsg = "";
    // 正在删除中
    bool             m_deleting = false;
    // 准备删除的歌曲hash
    QStringList      m_musichashlistToDel;
    // 第一次查询所有歌曲
    bool             m_firstInitAllMusic = true;
};

Q_DECLARE_METATYPE(DataBaseService::ListSortType)

#endif // DATABASESERVICE_H
