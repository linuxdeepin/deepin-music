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

#include "../core/playlist.h"

#include <QObject>
#include <QScopedPointer>
#include <MprisPlayer>
#include <searchmeta.h>

class QAudioBuffer;

class Playlist;

class PresenterPrivate;
class Transfer: public QObject
{
    Q_OBJECT
public:
    explicit Transfer(QObject *parent = 0);
    ~Transfer();
public slots:
    void onMusicListAdded(PlaylistPtr playlist, const MetaPtrList metalist);
signals:
    void musicListAdded(PlaylistPtr playlist, const MetaPtrList metalist);
};

class Presenter : public QObject
{
    Q_OBJECT
public:
    explicit Presenter(QObject *parent = nullptr);
    ~Presenter();

    void initMpris(MprisPlayer *mprisPlayer);
    void prepareData();
    void postAction();
    void handleQuit();
    void openUri(const QUrl &uri);
    void removeListSame(QStringList *list);

    QList<PlaylistPtr > allplaylist();
    PlaylistPtr playlist(const QString &id);

public slots:
    void volumeUp();
    void volumeDown();
    void togglePaly();
    void pause();
    void next();
    void prev();
    void onHandleQuit();
    void onSavePosition();
    void requestImportPaths(PlaylistPtr playlist, const QStringList &filelist);

signals:
    void dataLoaded();
    //控制进度条滑块
    void hidewaveformScale();

signals:
    //! player
    void audioBufferProbed(const QAudioBuffer &buffer);
    void metaBuffer(const QVector<float> &buffer, const QString &hash);

    //! ui: request import dialog
    void meidaFilesImported(PlaylistPtr playlist, MetaPtrList metalist);

    //! ui: control
    // TODO: need path
    void requestImportFiles();
    void showPlaylist(bool visible);
    void showMusicList(PlaylistPtr playlist);

    //! from playlist manager
    void playlistAdded(PlaylistPtr, bool newflag = false);
    void playlistRemove(PlaylistPtr);
    void activePlaylistChanged(PlaylistPtr);

    //! from music manager
    void playNext(PlaylistPtr playlist, const MetaPtr meta);
    void locateMusic(PlaylistPtr, const MetaPtr);
    void currentMusicListChanged(PlaylistPtr);
    void musicListResorted(PlaylistPtr);
    void musicListAdded(PlaylistPtr playlist, const MetaPtrList metalist);
    void musicListRemoved(PlaylistPtr playlist, const MetaPtrList metalist);
    void requestMusicListMenu(const QPoint &pos,
                              PlaylistPtr selectedlist,
                              PlaylistPtr favlist,
                              QList<PlaylistPtr >newlists,
                              char type);
    void musicListClear();

    //! from control
    void musicPlayed(PlaylistPtr playlist, const MetaPtr meta);
    void musicError(PlaylistPtr playlist, const MetaPtr meta, int error);
    void musicPaused(PlaylistPtr playlist, const MetaPtr meta);
    void musicStoped(PlaylistPtr playlist, const MetaPtr meta);
    void musicMetaUpdate(PlaylistPtr playlist, const MetaPtr meta);
    void progrossChanged(qint64 pos, qint64 length, qint64 coefficient);
    void volumeChanged(int volume);
    void mutedChanged(bool muted);
    void modeChanged(int);

    //! from lyricservice
    void requestContextSearch(const QString &context);
    void lyricSearchFinished(const MetaPtr,  const DMusic::SearchMeta &song, const QByteArray &lyricData);
    void coverSearchFinished(const MetaPtr, const DMusic::SearchMeta &song, const QByteArray &coverData);
    void contextSearchFinished(const QString &context, const QList<DMusic::SearchMeta> &metalist);

    //! meta info
    void metaLibraryClean();
    void scanFinished(const QString &jobid, int mediaCount);
    void notifyMusciError(PlaylistPtr playlist, const MetaPtr meta, int error);
    void notifyAddToPlaylist(PlaylistPtr playlist, const MetaPtrList metalist, int count);

    //! search
    void searchCand(QString searchText, PlaylistPtr playlist);
    void searchResult(QString searchText, QList<PlaylistPtr> resultlist, QString id);
    void musicFileMiss();
    //语音控制
    void sigSpeedResult(int action, bool result);

public slots:
    //! music control interface
    void onSyncMusicPlay(PlaylistPtr playlist, const MetaPtr meta);
    void onSyncMusicResume(PlaylistPtr playlist, const MetaPtr meta);
    void onSyncMusicPrev(PlaylistPtr playlist, const MetaPtr meta);
    void onSyncMusicNext(PlaylistPtr playlist, const MetaPtr meta);

    void onMusicPlay(PlaylistPtr playlist, const MetaPtr meta);
    void onMusicPause(PlaylistPtr playlist, const MetaPtr meta);
    void onMusicPauseNow(PlaylistPtr playlist, const MetaPtr meta);
    void onMusicResume(PlaylistPtr playlist, const MetaPtr meta);
    void onMusicStop(PlaylistPtr playlist, const MetaPtr meta);
    void onMusicPrev(PlaylistPtr playlist, const MetaPtr meta);
    void onMusicNext(PlaylistPtr playlist, const MetaPtr meta);

    void onToggleFavourite(const MetaPtr meta);
    void onAddMetasFavourite(const MetaPtrList metalist);
    void onRemoveMetasFavourite(const MetaPtrList metalist);

    void onChangeProgress(qint64 value, qint64 range);
    void onVolumeChanged(int volume);
    void onPlayModeChanged(int mode);
    void onToggleMute();
    void onFadeInOut();

    void onUpdateMetaCodec(const QString &preTitle, const QString &preArtist, const QString &preAlbum, const MetaPtr meta);

    //! music list
    void onPlayall(PlaylistPtr playlist);
    void onResort(PlaylistPtr playlist, int sortType);

    //! ui: playlist manager interface
    void onMusiclistRemove(PlaylistPtr playlist, const MetaPtrList metalist);
    void onMusiclistDelete(PlaylistPtr playlist, const MetaPtrList metalist);
    void onAddToPlaylist(PlaylistPtr playlist, const MetaPtrList metalist);
    void onAddMetaToPlaylist(PlaylistPtr playlist, const MetaPtrList metalist);
    void onPlaylistAdd(bool edit);
    void onCurrentPlaylistChanged(PlaylistPtr playlist);
    void onCustomResort(const QStringList &uuids);

    //! ui: menu interface
    void onRequestMusiclistMenu(const QPoint &pos, char type);
    void onSearchText(const QString &id, const QString &text);
    void onSearchCand(const QString text);//查询候选项
    void onExitSearch();
    void onLocateMusicAtAll(const QString &hash);
    void onChangeSearchMetaCache(const MetaPtr meta, const DMusic::SearchMeta &search);

    void onScanMusicDirectory();
    void onImportFiles(const QStringList &filelist, PlaylistPtr playlist);

    //语音控制槽函数
    void onSpeechPlayMusic(const QString music);
    void onSpeechPlayArtist(const QString artist);
    void onSpeechPlayArtistMusic(const QString artist, const QString music);
    void onSpeechPlayFaverite();
    void onSpeechPlayCustom(const QString listName);
    void onSpeechPlayRadom();

    void onSpeechPause();
    void onSpeechStop();
    void onSpeechResume();
    void onSpeechPrevious();
    void onSpeechNext();

    void onSpeechFavorite();
    void onSpeechunFaverite();
    void onSpeechsetMode(const int mode);

    //均衡器
    void setEqualizer(bool enabled, int curIndex, QList<int> indexbaud);
    void setEqualizerEnable(bool enabled);
    void setEqualizerpre(int val);
    void setEqualizerbauds(int index, int val);
    void setEqualizerCurMode(int curIndex);

private:
    bool containsStr(QString searchText, QString text);
    QScopedPointer<PresenterPrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), Presenter)
};


