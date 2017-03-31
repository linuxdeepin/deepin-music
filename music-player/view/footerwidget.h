/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#pragma once

#include <QFrame>
#include <QPointer>
#include <QLabel>

#include "../core/playlist.h"
#include <searchmeta.h>

class FooterPrivate;
class Footer : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(QString defaultCover READ defaultCover WRITE setDefaultCover)

public:
    explicit Footer(QWidget *parent = 0);
    ~Footer();

public:
    void enableControl(bool enable = true);
    void initData(PlaylistPtr current, int mode);

    QString defaultCover() const;

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

    void toggleMute();
    void togglePlaylist();
    void modeChanged(int);
    void toggleFavourite(const MetaPtr meta);

    void mouseMoving(Qt::MouseButton botton);

public slots:
    void onMusicListAdded(PlaylistPtr playlist, const MetaPtrList metalist);
    void onMusicListRemoved(PlaylistPtr playlist, const MetaPtrList metalist);
    void onMusicPlayed(PlaylistPtr playlist, const MetaPtr meta);
    void onMusicError(PlaylistPtr playlist, const MetaPtr meta, int error);
    void onMusicPause(PlaylistPtr playlist, const MetaPtr meta);
    void onMusicStoped(PlaylistPtr playlist, const MetaPtr meta);
    void onProgressChanged(qint64 value, qint64 duration);
    void onCoverChanged(const MetaPtr meta, const DMusic::SearchMeta &, const QByteArray &coverData);
    void onVolumeChanged(int volume);
    void onMutedChanged(bool muted);
    void onModeChange(int mode);
    void onUpdateMetaCodec(const MetaPtr meta);
    void setDefaultCover(QString defaultCover);
    void onMediaLibraryClean();

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

