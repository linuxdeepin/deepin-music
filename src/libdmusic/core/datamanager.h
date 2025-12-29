// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include "global.h"

class DataManagerPrivate;
class DataManager : public QObject
{
    Q_OBJECT
public:
    DataManager(QStringList supportedSuffixs, QObject *parent = nullptr);
    ~DataManager();

    void setCurrentPlayliHash(const QString &hash);
    QString currentPlayliHash();
    DMusic::MediaMeta metaFromHash(const QString &hash);
    DMusic::PlaylistInfo playlistFromHash(const QString &hash);
    QList<DMusic::MediaMeta> getPlaylistMetas(const QString &hash = "", int count = -1);
    QList<DMusic::PlaylistInfo> allPlaylistInfos();
    QVariantList allPlaylistVariantList();
    QList<DMusic::PlaylistInfo> customPlaylistInfos();
    QVariantList customPlaylistVariantList();
    QList<DMusic::AlbumInfo> allAlbumInfos();
    QVariantList allAlbumVariantList();
    QList<DMusic::ArtistInfo> allArtistInfos();
    QVariantList allArtistVariantList();

    void importMetas(const QStringList &urls, const QString &playlistHash = "", const bool &playFalg = false);
    void addMetasToPlayList(const QList<QString> &metaHash, const QString &playlistHash = "",
                            const bool &addToPlay = true);
    void addMetasToPlayList(const QList<DMusic::MediaMeta> &metas, const QString &playlistHash = "",
                            const bool &addToPlay = true);
    void clearPlayList(const QString &playlistHash = "", const bool &addToPlay = true);
    void removeFromPlayList(const QStringList listToDel, const QString &playlistHash = "",
                            bool delFlag = false);
    bool moveMetasPlayList(const QStringList &metaHashs, const QString &playlistHash = "", const QString &nextHash = "");
    bool isExistMeta();
    DMusic::PlaylistInfo addPlayList(const QString &name);
    void sortPlaylist(const int &type, const QString &hash, bool signalFlag = true);
    bool deletePlaylist(QString playlistHash);
    bool renamePlaylist(const QString &name, const QString &playlistHash);
    void movePlaylist(const QString &hash, const QString &nextHash);
    bool isExistMeta(const QString &metaHash, const QString &playlistHash);
    void saveDataToDB();

    // 更新编码
    void updateMetaCodec(const DMusic::MediaMeta &meta);

    //搜索歌曲
    void quickSearchText(const QString &text, QStringList &metaTitles,
                         QList<QPair<QString, QString> > &albums, QList<QPair<QString, QString> > &artists);
    void searchText(const QString &text, QList<DMusic::MediaMeta> &metaTitles,
                    QList<DMusic::AlbumInfo> &albums, QList<DMusic::ArtistInfo> &artists, const QString &type = "");
    QList<DMusic::AlbumInfo> searchedAlbumInfos();
    QList<DMusic::ArtistInfo> searchedArtistInfos();
    QVariantList searchedAlbumVariantList();
    QVariantList searchedArtistVariantList();

    //配置文件
    void syncToSettings();
    void resetToSettings();
    QVariant valueFromSettings(const QString &key);
    void setValueToSettings(const QString &key, const QVariant &value, const bool &empty = false);

public slots:
    void slotAddOneMeta(QStringList playlistHashs, DMusic::MediaMeta meta);
    void slotLazyLoadDatabase();

signals:
    void signalImportMetas(const QStringList &urls, const QSet<QString> &metaHashs, bool importPlayFlag,
                           const QSet<QString> &playMetaHashs, const QSet<QString> &allMetaHashs,
                           const QString &playlistHash, const bool &playFalg);
    void signalAddOneMeta(QStringList playlistHashs, DMusic::MediaMeta meta, const bool &addToPlay);
    void signalAddMetaFinished(QStringList playlistHashs);
    void signalImportFinished(QStringList playlistHashs, int failCount, int sucessCount, int existCount, QString mediaHash);
    void signalDeleteOneMeta(QStringList playlistHashs, QString hash, const bool &addToPlay);
    void signalDeleteFinished(QStringList playlistHashs);
    void signalUpdatedMetaCodec(DMusic::MediaMeta meta, QString preAlbum, QString preArtist);
    void signalPlaylistSortChanged(QString playlistHash);
    void signalCurrentPlaylistSChanged(QString playlistHash);
    void signalClearImportingHash(const QString &hash);

private:
    void initPlaylist();
    int metaIndexFromHash(const QString &hash);
    int playlistIndexFromHash(const QString &hash);
    void deleteMetaFromAllMetas(const QStringList &hashs);
    void addMetaToAlbum(const DMusic::MediaMeta &meta);
    void deleteMetaFromAlbum(const QString &metaHash, const QString &name);
    void addMetaToArtist(const DMusic::MediaMeta &meta);
    void deleteMetaFromArtist(const QString &metaHash, const QString &name);
    int allMusicCountDB();
    bool loadCurrentMetasDB();
    bool loadMetasDB();
    bool loadPlaylistDB();
    bool loadPlaylistMetasDB();
    bool isPlaylistExistDB(const QString &uuid);
    void addPlaylistDB(const DMusic::PlaylistInfo &playlist);
    bool deletePlaylistDB(const QString &uuid);
    bool deleteAllPlaylistDB();
    int addMetasToPlaylistDB(const QString &uuid, const QList<DMusic::MediaMeta> &metas);

private:
    DataManagerPrivate               *m_data;
};

#endif //DATAMANAGER_H
