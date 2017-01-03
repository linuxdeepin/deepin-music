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
    void locateMusic(PlaylistPtr playlist, const MusicMeta &info);

    void play(PlaylistPtr playlist, const MusicMeta &meta);
    void resume(PlaylistPtr playlist, const MusicMeta &meta);
    void pause(PlaylistPtr playlist, const MusicMeta &meta);
    void next(PlaylistPtr playlist, const MusicMeta &meta);
    void prev(PlaylistPtr playlist, const MusicMeta &meta);
    void changeProgress(qint64 value, qint64 duration);
    void volumeChanged(int volume);

    void toggleMute();
    void togglePlaylist();
    void modeChanged(int);
    void toggleFavourite(const MusicMeta &meta);

    void mouseMoving(Qt::MouseButton botton);

public slots:
    void onMusicAdded(PlaylistPtr playlist, const MusicMeta &meta);
    void onMusicListAdded(PlaylistPtr playlist, const MusicMetaList &metalist);
    void onMusicRemoved(PlaylistPtr playlist, const MusicMeta &meta);
    void onMusicPlayed(PlaylistPtr playlist, const MusicMeta &meta);
    void onMusicPause(PlaylistPtr playlist, const MusicMeta &meta);
    void onMusicStoped(PlaylistPtr playlist, const MusicMeta &meta);
    void onProgressChanged(qint64 value, qint64 duration);
    void onCoverChanged(const MusicMeta &meta, const QByteArray &coverData);
    void onVolumeChanged(int volume);
    void onMutedChanged(bool muted);
    void setDefaultCover(QString defaultCover);

protected:
    virtual void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    virtual bool eventFilter(QObject *obj, QEvent *event) Q_DECL_OVERRIDE;

private:
    QScopedPointer<FooterPrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), Footer)
};

