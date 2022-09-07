// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DBOPERATE_H
#define DBOPERATE_H

#include <QObject>
#include <QDateTime>
#include <QMutex>
#include <QDebug>
#include <QSqlDatabase>

class QSqlDatabase;
class MediaMeta;
class MediaLibrary;
class DBOperate : public QObject
{
    Q_OBJECT
public:
    explicit DBOperate(QObject *parent = nullptr);
    ~DBOperate();

    struct PlaylistDataThread {
        QString uuid;
        bool    readonly    = false;
    };

    void stop();
    void setNeedSleep();
public slots:
    void     slotImportMedias(const QString &importHash, QString playHash, const QStringList &urllist);
    void     slotCreatCoverImg(const QList<MediaMeta> &metas);
    void     slotRemoveSelectedSongs(const QString &curpage, const QStringList &musichashlist, bool removeFromLocal);
private:
    bool     deleteMetaFromAllMusic(const QStringList &metaHash, bool removeFromLocal);
    bool     deleteMetaFromPlaylist(QString uuid, const QStringList &metaHash);
    // 歌曲数量
    int      allMusicInfosCount();
    // 歌单信息
    QList<PlaylistDataThread>  allPlaylistMetaUUid();
    void     addMediaMetaToDB(const MediaMeta &meta);
    // 判断歌曲是否存在
    bool     isMediaMetaExist(const QString &hash);
    // 添加歌曲到歌单
    int      addMetaToPlaylist(QString uuid, const QList<MediaMeta> &metas);
    // 添加歌曲到歌单
    bool     addMetaToAll(MediaMeta meta);
signals:
    void     fileIsNotExist(QString imagepath);

    void     sigImportMetaFromThread(MediaMeta meta);
    // 导入成功结束
    void     sigImportFinished(int failCount, int successCount, int exsitCount);
    void     sigCreatOneCoverImg(MediaMeta meta);
    // 收藏中的歌曲被删除，动态显示
    void     signalFavSongRemove(const QString &musicHash);
    // 发送删除歌曲通知消息，动态显示
    void     signalRmvSong(const QString &listHash, const QString &musicHash, bool removeFromLocal);
    // 所有歌曲被清空
    void     signalAllMusicCleared();
    // 已导入百分比
    void     signalImportedPercent(int percent);
    // 歌曲数量导入
    void     signalMusicAddOne(QString listHash, MediaMeta meta);
    // 收藏中的歌曲被删除，动态显示
    void     signalFavSongAdd(QString musicHash);
    // 删除结束
    void     signalDelFinish();
public:
private:
    MediaLibrary     *m_mediaLibrary = nullptr;
    QSqlDatabase      m_db;
    bool              m_needStop = false;
    bool              m_needSleep = false;
    QMutex            m_mutex;
    QString           m_importHash;
    QString           m_playHash;
    // 导入的歌曲计数
    int               m_successCount = 0;
    // 存在的歌曲计数
    int               m_exsitCount = 0;
    // 导入失败的歌曲计数
    int               m_importFailCount = 0;
};

#endif // DBOPERATE_H
