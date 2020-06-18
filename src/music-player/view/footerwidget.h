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

#include <DBlurEffectWidget>
#include <QPointer>
#include <DFloatingWidget>

#include "../core/playlist.h"
#include <searchmeta.h>

class QAudioBuffer;

DWIDGET_USE_NAMESPACE

class PlayListWidget;
class FooterPrivate;
class Footer : public DFloatingWidget
{
    Q_OBJECT
    Q_PROPERTY(QString defaultCover READ defaultCover WRITE setDefaultCover)

public:
    explicit Footer(QWidget *parent = nullptr);
    ~Footer() override;

public:
    void setCurPlaylist(PlaylistPtr playlist);
    void enableControl(bool enable = true);
    void setViewname(const QString &viewname);

    QString defaultCover() const;

    void setLyricButtonChecked(bool flag);
    void setPlaylistButtonChecked(bool flag);

    PlayListWidget *getPlayListWidget();
    void showPlayListWidget(int width, int height, bool changFlag = false);
    void setSize(int width, int height, bool changFlag = false);
    bool getShowPlayListFlag();
    void refreshBackground();

signals:
    void toggleLyricView();
    void locateMusic(PlaylistPtr playlist, const MetaPtr meta);

    void play(PlaylistPtr playlist, const MetaPtr meta);
    void resume(PlaylistPtr playlist, const MetaPtr meta);
    void pause(PlaylistPtr playlist, const MetaPtr meta);
    void next(PlaylistPtr playlist, const MetaPtr meta);
    void prev(PlaylistPtr playlist, const MetaPtr meta);
    void changeProgress(qint64 value, qint64 duration);
    void volumeChanged(int volume);
    void updateVolume(int volume);

    void toggleMute();
    void togglePlaylist();
    void modeChanged(int);
    void toggleFavourite(const MetaPtr meta);

    void progressRealHeightChanged(qreal height);
    void mouseMoving(Qt::MouseButton button);

    void audioBufferProbed(const QAudioBuffer &buffer);
    void metaBuffer(const QVector<float> &buffer, const QString &hash);

public slots:
    void onMusicListAdded(PlaylistPtr playlist, const MetaPtrList metalist);
    void onMusicListRemoved(PlaylistPtr playlist, const MetaPtrList metalist);
    void onMusicPlayed(PlaylistPtr playlist, const MetaPtr meta);
    void onMusicError(PlaylistPtr playlist, const MetaPtr meta, int error);
    void onMusicPause(PlaylistPtr playlist, const MetaPtr meta);
    void onMusicStoped(PlaylistPtr playlist, const MetaPtr meta);
    void onProgressChanged(qint64 value, qint64 duration, qint64 coefficient);
    void onCoverChanged(const MetaPtr meta, const DMusic::SearchMeta &, const QByteArray &coverData);
    void onVolumeChanged(int volume);
    void onMutedChanged(bool muted);
    void onModeChange(int mode);
    void onUpdateMetaCodec(const QString &preTitle, const QString &preArtist, const QString &preAlbum, const MetaPtr meta);
    void setDefaultCover(QString defaultCover);
    void onMediaLibraryClean();
    void slotTheme(int type);
    void onTogglePlayButton();

protected:
    virtual void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
    virtual void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    virtual void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    virtual void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    virtual bool eventFilter(QObject *obj, QEvent *event) Q_DECL_OVERRIDE;

private:
    QScopedPointer<FooterPrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), Footer)
};

