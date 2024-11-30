// Copyright (C) 2020 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PRESENTER_H
#define PRESENTER_H

#include "libdmusic_global.h"

#include <QObject>
#include <QImage>
#include <QColor>
#include <QVariant>

#include "global.h"

class PresenterPrivate;
class LIBDMUSICSHARED_EXPORT Presenter : public QObject
{
    Q_OBJECT
public:
    Presenter(const QString &unknownAlbumStr = "", const QString &unknownArtistStr = "", QObject *parent = nullptr);
    ~Presenter();

    // dbus接口
    Q_INVOKABLE void setMprisPlayer(const QString &serviceName, const QString &desktopEntry, const QString &identity);

    // 播放控制
    Q_INVOKABLE QStringList supportedSuffixList()const;
    Q_INVOKABLE void setActivateMeta(const QString &metaHash);
    Q_INVOKABLE QImage getActivateMetImage();
    Q_INVOKABLE QVariantMap getActivateMeta();
    Q_INVOKABLE QVariant getPlaybackStatus();
    Q_INVOKABLE void setMute(bool mute);
    Q_INVOKABLE bool getMute();
    Q_INVOKABLE void setVolume(int volume);
    Q_INVOKABLE int getVolume();
    Q_INVOKABLE void setPlaybackMode(QVariant mode);
    Q_INVOKABLE QVariant getPlaybackMode();
    Q_INVOKABLE void setPosition(qint64 position);
    Q_INVOKABLE qint64 getPosition();
    // 均衡器
    Q_INVOKABLE void setEQ(bool enabled, int curIndex, QVariantList indexbaud);
    Q_INVOKABLE void setEQEnable(bool enable);
    Q_INVOKABLE void setEQpre(int val);
    Q_INVOKABLE void setEQbauds(int index, int val);
    Q_INVOKABLE void setEQCurMode(int curIndex);

    //  歌曲管理
    Q_INVOKABLE void showMetaFile(const QString &hash);
    Q_INVOKABLE void playAlbum(const QString &album, const QString &metaHash = "");
    Q_INVOKABLE bool nextMetaFromPlay(const QString &metaHash);
    Q_INVOKABLE bool preMetaFromPlay(const QString &metaHash);
    Q_INVOKABLE void playArtist(const QString &artist, const QString &metaHash = "");
    Q_INVOKABLE void playPlaylist(const QString &playlistHash, const QString &metaHash = "");
    Q_INVOKABLE void setCurrentPlayList(const QString &playlistHash);
    Q_INVOKABLE QString getCurrentPlayList();
    Q_INVOKABLE void importMetas(const QStringList &urls, const QString &playlistHash = "", const bool &playFalg = false);
    Q_INVOKABLE void addMetasToPlayList(const QStringList &metaHash, const QString &playlistHash = "");
    Q_INVOKABLE void addAlbumToPlayList(const QString &album, const QString &playlistHash = "");
    Q_INVOKABLE void addArtistToPlayList(const QString &artist, const QString &playlistHash = "");
    Q_INVOKABLE void clearPlayList(const QString &PlaylistHash = "");
    Q_INVOKABLE void removeFromPlayList(const QStringList listToDel, const QString &playlistHash = "",
                                        bool delFlag = false);
    Q_INVOKABLE void moveMetasPlayList(const QStringList &metaHashs, const QString &playlistHash = "", const QString &nextHash = "");
    Q_INVOKABLE QVariantList getPlaylistMetas(const QString &hash = "", int count = -1);
    Q_INVOKABLE bool isExistMeta();
    Q_INVOKABLE QVariantMap musicInforFromHash(const QString &hash);
    Q_INVOKABLE QVariantMap addPlayList(const QString &name);
    Q_INVOKABLE QVariant playlistSortType(const QString &hash);
    Q_INVOKABLE void sortPlaylist(const int &type, const QString &hash);
    Q_INVOKABLE int playlistMetaCount(const QString &hash);
    Q_INVOKABLE bool deletePlaylist(const QString &hash);
    Q_INVOKABLE bool renamePlaylist(const QString &name, const QString &playlistHash);
    Q_INVOKABLE void movePlaylist(const QString &hash, const QString &nextHash);
    Q_INVOKABLE QVariantMap playlistInfoFromHash(const QString &hash);
    Q_INVOKABLE bool isExistMeta(const QString &metaHash, const QString &playlistHash);
    Q_INVOKABLE QVariantList allPlaylistInfos();
    Q_INVOKABLE QVariantList customPlaylistInfos();
    Q_INVOKABLE QVariantList allAlbumInfos();
    Q_INVOKABLE QVariantList allArtistInfos();

    //搜索歌曲
    Q_INVOKABLE QVariantMap quickSearchText(const QString &text);
    Q_INVOKABLE QVariantMap searchText(const QString &text, const QString &type = "");
    Q_INVOKABLE QVariantList searchedAlbumInfos();
    Q_INVOKABLE QVariantList searchedArtistInfos();

    // 配置文件
    Q_INVOKABLE void syncToSettings();
    Q_INVOKABLE void resetToSettings();
    Q_INVOKABLE QVariant valueFromSettings(const QString &key);
    Q_INVOKABLE void setValueToSettings(const QString &key, const QVariant &value);

    // 歌词解析
    Q_INVOKABLE QVariantList getLyrics();

    // 歌曲解析
    Q_INVOKABLE QStringList detectEncodings(const QString &metaHash);
    Q_INVOKABLE void updateMetaCodec(const QString &metaHash, const QString &codecStr);
    Q_INVOKABLE QColor getMainColorByKmeans();
    Q_INVOKABLE QColor getSecondColorByKmeans();
    Q_INVOKABLE QImage getEffectImage();
    Q_INVOKABLE void setEffectImage(const QImage &img);

    // 退出
    Q_INVOKABLE void forceExit();

public slots:
    // 播放器
    void play();
    void pause();
    void playPause();
    void playPre();
    void playNext();
    void resume();
    void stop();

    void saveDataToDB();

signals:
    // 播放器
    void restorePlaybackStatus();
    void currentPlaylistSChanged(QString playlistHash);
    void metaChanged();
    void positionChanged(qint64 position, qint64 length);
    void playbackStatusChanged(DmGlobal::PlaybackStatus status);
    void muteChanged(bool mute);
    void volumeChanged(int volume);

    void resetedFromSettings();
    void valueChangedFromSettings(const QString &key, const QVariant &value);
    void updatePlayingIcon(const QString &iconPath);
    void audioSpectrumData(QVector<int> data);
    void audioBuffer(const QVariantList &buffer, const QString &hash);
    void addOneMeta(QStringList playlistHashs, QVariantMap meta);
    void addMetaFinished(QStringList playlistHashs);
    void importFinished(QStringList playlistHashs, int failCount, int sucessCount, int existCount);
    void deleteOneMeta(QStringList playlistHashs, QString hash);
    void deleteFinished(QStringList playlistHashs);
    void playlistSortChanged(QString playlistHash);
    void addedPlaylist(QString playlistHash);
    void deletedPlaylist(QString playlistHash);
    void renamedPlaylist(const QString &name, const QString &playlistHash);
    void updatedMetaCodec(QVariantMap meta, QString preAlbum, QString preArtist);
    void updateCDStatus(int status);
    void quitRequested();
    void raiseRequested();

private:
    PresenterPrivate *m_data;
};
Q_DECLARE_METATYPE(Presenter *)
#endif // PRESENTER_H
